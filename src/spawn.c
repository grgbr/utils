/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/spawn.h"
#include <utils/pipe.h>
#include <utils/path.h>
#include <stroll/dlist.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <spawn.h>
#include <pthread.h>
#include <fcntl.h>
#include <utils/signal.h>
#include <utils/thread.h>

struct popen_ctx {
	struct stroll_dlist_node node;
	FILE * stream;
	pid_t child;
};

static struct stroll_dlist_node head = STROLL_DLIST_INIT(head);

FILE *
etux_spawn_popen(const char *path,
		 char *const argv[],
		 char *const envp[],
		 int flags)
{
	spawn_assert_api(upath_validate_path_name(path) > 0);
	spawn_assert_api(!(flags & ~(O_WRONLY | O_RDONLY | O_CLOEXEC)));
	spawn_assert_api((flags & O_WRONLY) ^ (flags & O_RDONLY));

	struct popen_ctx *ctx;
	struct popen_ctx *entrie;
	int pipefd[2];
	int parent_end;
	int child_end;
	int child_fd;
	const char *mode;
	posix_spawn_file_actions_t fa;
	int err;

	ctx = malloc(sizeof(*ctx));
	if (!ctx) {
		errno = ENOMEM;
		return NULL;
	}

	stroll_dlist_init(&ctx->node);
	if (flags & O_WRONLY) {
		parent_end = UPIPE_WRITE_END;
		child_end = UPIPE_READ_END;
		child_fd = 0;
		mode = "w";
	} else {
		parent_end = UPIPE_READ_END;
		child_end = UPIPE_WRITE_END;
		child_fd = 1;
		mode = "r";
	}

	if (upipe_open_anon(pipefd, O_CLOEXEC) < 0)
		goto free;

	if (pipefd[child_end] == child_fd) {
		int fd = fcntl(pipefd[parent_end], F_DUPFD_CLOEXEC, 0);

		if (fd < 0)
			goto close;

		upipe_close(pipefd[child_end]);
		pipefd[child_end] = fd;
	}

	posix_spawn_file_actions_init(&fa);
	err = posix_spawn_file_actions_adddup2(&fa, pipefd[child_end], child_fd);
	if (err) {
		errno = err;
		goto destroy;
	}

	// force close all popen pipe for case CLOSEXEC not set
	stroll_dlist_foreach_entry(&head, entrie, node) {
		if (fileno(entrie->stream) != child_fd) {
			err = posix_spawn_file_actions_addclose(&fa,
				fileno(entrie->stream));
			if (err) {
				errno = err;
				goto destroy;
			}
		}
	}

	err = posix_spawn(&ctx->child, path, &fa, NULL, argv, envp);
	if (err) {
		errno = err;
		goto destroy;
	}

	posix_spawn_file_actions_destroy(&fa);
	upipe_close(pipefd[child_end]);
	if (!(flags & O_CLOEXEC))
		fcntl(pipefd[parent_end], F_SETFD, 0);

	ctx->stream = fdopen(pipefd[parent_end], mode);
	if (ctx->stream) {
		stroll_dlist_nqueue_back(&head, &ctx->node);
		return ctx->stream;
	}

	upipe_close(pipefd[parent_end]);
	kill(ctx->child, SIGTERM);
	waitpid(ctx->child, NULL, 0);
	free(ctx);
	return NULL;

destroy:
	posix_spawn_file_actions_destroy(&fa);
close:
	upipe_close(pipefd[UPIPE_READ_END]);
	upipe_close(pipefd[UPIPE_WRITE_END]);
free:
	free(ctx);
	return NULL;
}

int
etux_spawn_pclose(FILE *stream)
{
	spawn_assert_api(stream);

	pid_t pid;
	pid_t wait_pid;
	int wstatus;
	struct popen_ctx *ctx;
	int found = 0;
	int ret;

	stroll_dlist_foreach_entry(&head, ctx, node) {
		if (ctx->stream == stream) {
			found = 1;
			break;
		}
	}

	if (!found) {
		spawn_assert_api(found);
		return -EBADF;
	}

	stroll_dlist_remove(&ctx->node);
	pid = ctx->child;
	ret = fclose(ctx->stream);
	free(ctx);
	if (ret)
		return -errno;

	do {
		int state;

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
		wait_pid = waitpid(pid, &wstatus, 0);
		pthread_setcancelstate(state, NULL);
	} while(wait_pid == -1 && errno == EINTR);

	if (wait_pid == -1)
		return -errno;

	return wstatus;
}

static struct sigaction intr, quit;
static int sa_refcntr = 0;
static struct uthr_mutex lock = UTHR_INIT_MUTEX;

int
etux_spawn_system(const char *path,
		  char *const argv[],
		  char *const envp[])
{
	int ret;
	pid_t pid;
	pid_t wait_pid;
	int wstatus;
	posix_spawnattr_t spawn_attr;
	struct sigaction sa;
	sigset_t omask;
	sigset_t reset;

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	usig_emptyset(&sa.sa_mask);

	uthr_lock_mutex(&lock);
	if (sa_refcntr++ == 0) {
		usig_action(SIGINT,  &sa, &intr);
		usig_action(SIGQUIT, &sa, &quit);
	}
	uthr_unlock_mutex(&lock);

	usig_addset(&sa.sa_mask, SIGCHLD);
	usig_procmask(SIG_BLOCK, &sa.sa_mask, &omask);
	usig_emptyset(&reset);
	if (intr.sa_handler != SIG_IGN)
		usig_addset(&reset, SIGINT);

	if (quit.sa_handler != SIG_IGN)
		usig_addset(&reset, SIGQUIT);

	posix_spawnattr_init(&spawn_attr);
	posix_spawnattr_setsigmask(&spawn_attr, &omask);
	posix_spawnattr_setsigdefault(&spawn_attr, &reset);
	posix_spawnattr_setflags(&spawn_attr,
	                         POSIX_SPAWN_SETSIGDEF | POSIX_SPAWN_SETSIGMASK);
	ret = posix_spawn(&pid, path, NULL, &spawn_attr, argv, envp);
	posix_spawnattr_destroy(&spawn_attr);
	if (!ret) {
		do {
			int state;

			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
			wait_pid = waitpid(pid, &wstatus, 0);
			pthread_setcancelstate(state, NULL);
		} while(wait_pid == -1 && errno == EINTR);

		if (wait_pid == -1)
			ret = -errno;
		else
			ret = wstatus;

	} else
		ret = -ret; // posix_spawn return errno value

	uthr_lock_mutex(&lock);
	if (--sa_refcntr == 0) {
		usig_action(SIGINT,  &intr, NULL);
		usig_action(SIGQUIT, &quit, NULL);
	}
	uthr_unlock_mutex(&lock);
	return ret;
}
