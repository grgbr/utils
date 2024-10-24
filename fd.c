/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/fd.h"
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>

ssize_t
ufd_nointr_read(int fd, char * __restrict data, size_t size)
{
	ssize_t ret;

	do {
		ret = ufd_read(fd, data, size);
	} while (ret == -EINTR);

	return ret;
}

ssize_t
ufd_nointr_write(int fd, const char * __restrict data, size_t size)
{
	ssize_t ret;

	do {
		ret = ufd_write(fd, data, size);
	} while (ret == -EINTR);

	return ret;
}

int
ufd_nointr_open(const char * __restrict path, int flags)
{
	int fd;

	do {
		fd = ufd_open(path, flags);
	} while (fd == -EINTR);

	return fd;
}

int
ufd_nointr_open_at(int dir, const char * __restrict path, int flags)
{
	int fd;

	do {
		fd = ufd_open_at(dir, path, flags);
	} while (fd == -EINTR);

	return fd;
}

#if !defined(__NR_close_range) || !defined(__USE_GNU)

#if defined(CONFIG_UTILS_VALGRIND)

/*
 * Valgrind opens multiple files for internal purposes (e.g., its output
 * channels, vgdb pipes...)
 * While performing a runtime analysis, these file descriptors should not be
 * closed. Any attempt to close() them will fail with a EBADF error since
 * Valgrind "overloads" close() to pretend these are not opened.
 *
 * To indicate the analyzed program that this set of opened files is reserved
 * for Valgrind's own usage, Valgrind "overloads" getrlimit() to report an
 * adjusted maximum allowed number of open file descriptors.
 *
 * What we do here consists in probing the maximum file descriptor number
 * allowed and adjust / ceil the fd given in argument with it.
 *
 * See function setup_file_descriptors() in Valgrind's core source code.
 */
static inline __nothrow
unsigned int
ufd_adjust_last_fd(unsigned int fd)
{
	return stroll_min(fd, ufd_max_nr() - 1);
}

#else  /* !defined(UTILS_VALGRIND) */

static inline __nothrow __const
unsigned int
ufd_adjust_last_fd(unsigned int fd)
{
	return fd;
}

#endif /* defined(UTILS_VALGRIND) */

int
ufd_close_fds(unsigned int first, unsigned int last)
{
	ufd_assert_api(first <= last);

	DIR * dir;
	int   ret;

	dir = opendir("/proc/self/fd");
	if (!dir) {
		ufd_assert_intern(errno != EBADF);
		return -errno;
	}

	last = ufd_adjust_last_fd(last);

	while (true) {
		struct dirent * ent;

		errno = 0;
		ent = readdir(dir);
		if (!ent) {
			ufd_assert_intern(errno != EBADF);
			break;
		}

		if (ent->d_type == DT_LNK) {
			unsigned int fd;
			char *       err;

			fd = (unsigned int)strtoul(ent->d_name, &err, 10);
			if (!*err &&
			    ((int)fd != dirfd(dir)) &&
			    (fd >= first) && (fd <= last))
				ufd_close(fd);
		}
	}

	ret = -errno;

	closedir(dir);

	return ret;
}

#endif /* !defined(__NR_close_range) || !defined(__USE_GNU) */
