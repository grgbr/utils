/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Process signal interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2017
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_SIGNAL_H
#define _UTILS_SIGNAL_H

#include <utils/cdefs.h>
#include <stdbool.h>
#include <signal.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define usig_assert_api(_expr) \
	stroll_assert("utils:usig", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define usig_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_API)

static inline __utils_nothrow
void
usig_action(int                                 signum,
            const struct sigaction * __restrict act,
            struct sigaction * __restrict       oldact)
{
	usig_assert_api(signum > 0);
	usig_assert_api(act || oldact);

	usig_assert_api(!sigaction(signum, act, oldact));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nothrow
void
usig_action(int                                 signum,
            const struct sigaction * __restrict act,
            struct sigaction * __restrict       oldact)
{
	sigaction(signum, act, oldact);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(1) __utils_nothrow
int
usig_isemptyset(const sigset_t * set)
{
	usig_assert_api(set);

	return !!sigisemptyset(set);
}

#if defined(CONFIG_UTILS_ASSERT_API)

static inline __utils_nonull(1) __utils_nothrow
void
usig_emptyset(sigset_t * set)
{
	usig_assert_api(set);

	usig_assert_api(!sigemptyset(set));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(1) __utils_nothrow
void
usig_emptyset(sigset_t * set)
{
	sigemptyset(set);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_API)

static inline __utils_nonull(1) __utils_nothrow
void
usig_fillset(sigset_t * set)
{
	usig_assert_api(set);

	usig_assert_api(!sigfillset(set));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(1) __utils_nothrow
void
usig_fillset(sigset_t * set)
{
	sigfillset(set);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_API)

#include <errno.h>

static inline __utils_nonull(1) __utils_nothrow
void
usig_addset(sigset_t * set, int signum)
{
	usig_assert_api(set);
	usig_assert_api(signum > 0);

	usig_assert_api(!sigaddset(set, signum));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(1) __utils_nothrow
void
usig_addset(sigset_t * set, int signum)
{
	sigaddset(set, signum);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_API)

static inline __utils_nonull(1) __utils_nothrow
void
usig_delset(sigset_t * set, int signum)
{
	usig_assert_api(set);
	usig_assert_api(signum > 0);

	usig_assert_api(!sigdelset(set, signum));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline void __utils_nonull(1) __utils_nothrow
usig_delset(sigset_t * set, int signum)
{
	sigdelset(set, signum);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(1) __utils_nothrow
bool
usig_ismember(const sigset_t * set, int signum)
{
	usig_assert_api(set);
	usig_assert_api(signum > 0);

	int ret;

	ret = sigismember(set, signum);
	usig_assert_api(ret >= 0);

	return !!ret;
}

#if defined(CONFIG_UTILS_ASSERT_API)

static inline __utils_nothrow
void
usig_procmask(int                         how,
              const sigset_t * __restrict set,
              sigset_t * __restrict       oldset)
{
	usig_assert_api(set || oldset);

	usig_assert_api(!sigprocmask(how, set, oldset));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nothrow
void
usig_procmask(int                         how,
              const sigset_t * __restrict set,
              sigset_t * __restrict       oldset)
{
	sigprocmask(how, set, oldset);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

extern const sigset_t * const         usig_inval_msk;
extern const sigset_t * const         usig_empty_msk;
extern const sigset_t * const         usig_full_msk;
extern const struct sigaction * const usig_dflt_act;

struct usig_new_act {
	unsigned int             no;
	const struct sigaction * act;
};

struct usig_orig_act {
	unsigned int     no;
	struct sigaction act;
};

extern void
usig_setup_actions(const struct usig_new_act nevv[__restrict_arr],
                   struct usig_orig_act      orig[__restrict_arr],
                   unsigned int              nr)
	__utils_nonull(1) __utils_nothrow;

extern void
usig_restore_actions(const struct usig_orig_act orig[__restrict_arr],
                     unsigned int               nr)
	__utils_nonull(1) __utils_nothrow;

#if defined(CONFIG_UTILS_SIGNAL_FD)

#include <utils/fd.h>
#include <sys/signalfd.h>

extern int
usig_read_fd(int fd, struct signalfd_siginfo * infos, unsigned int count)
	__utils_nonull(2) __leaf __warn_result;

static inline __utils_nonull(1) __utils_nothrow
int
usig_open_fd(const sigset_t * mask, int flags)
{
	usig_assert_api(mask);
	usig_assert_api(!usig_isemptyset(mask));
	usig_assert_api(!(flags & ~(SFD_NONBLOCK | SFD_CLOEXEC)));

	int fd;

	fd = signalfd(-1, mask, flags);
	if (fd < 0) {
		usig_assert_api(errno != EBADF);
		usig_assert_api(errno != EINVAL);

		return -errno;
	}

	return fd;
}

static inline
int
usig_close_fd(int fd)
{
	usig_assert_api(fd >= 0);

	int ret;

	ret = ufd_close(fd);

	usig_assert_api(!ret || (ret == -EINTR));

	return ret;
}

#endif /* defined(CONFIG_UTILS_SIGNAL_FD) */

#endif /* _UTILS_SIGNAL_H */
