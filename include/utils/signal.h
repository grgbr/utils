/**
 * @file      signal.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2017
 * @copyright GNU Public License v3
 *
 * Process signal interface
 *
 * @defgroup signal Signal
 *
 * This file is part of Utils
 *
 * Copyright (C) 2017 Grégor Boirie <gregor.boirie@free.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _UTILS_SIGNAL_H
#define _UTILS_SIGNAL_H

#include <utils/cdefs.h>
#include <stdbool.h>
#include <signal.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __usig_nonull(_arg_index, ...)

#define usig_assert(_expr) \
	uassert("usignal", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __usig_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define usig_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __nothrow
usig_action(int                                 signum,
            const struct sigaction * __restrict act,
            struct sigaction * __restrict       oldact)
{
	usig_assert(signum > 0);
	usig_assert(act || oldact);

	usig_assert(!sigaction(signum, act, oldact));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __nothrow
usig_action(int                                 signum,
            const struct sigaction * __restrict act,
            struct sigaction * __restrict       oldact)
{
	sigaction(signum, act, oldact);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline int __usig_nonull(1) __nothrow
usig_isemptyset(const sigset_t * set)
{
	usig_assert(set);

	return !!sigisemptyset(set);
}

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __usig_nonull(1) __nothrow
usig_emptyset(sigset_t * set)
{
	usig_assert(set);

	usig_assert(!sigemptyset(set));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __usig_nonull(1) __nothrow
usig_emptyset(sigset_t * set)
{
	sigemptyset(set);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __usig_nonull(1) __nothrow
usig_fillset(sigset_t * set)
{
	usig_assert(set);

	usig_assert(!sigfillset(set));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __usig_nonull(1) __nothrow
usig_fillset(sigset_t * set)
{
	sigfillset(set);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)
#include <errno.h>
static inline void __usig_nonull(1) __nothrow
usig_addset(sigset_t * set, int signum)
{
	usig_assert(set);
	usig_assert(signum > 0);

	usig_assert(!sigaddset(set, signum));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __usig_nonull(1) __nothrow
usig_addset(sigset_t * set, int signum)
{
	sigaddset(set, signum);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __usig_nonull(1) __nothrow
usig_delset(sigset_t * set, int signum)
{
	usig_assert(set);
	usig_assert(signum > 0);

	usig_assert(!sigdelset(set, signum));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __usig_nonull(1) __nothrow
usig_delset(sigset_t * set, int signum)
{
	sigdelset(set, signum);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline bool __usig_nonull(1) __nothrow
usig_ismember(const sigset_t * set, int signum)
{
	usig_assert(set);
	usig_assert(signum > 0);

	int ret;

	ret = sigismember(set, signum);
	usig_assert(ret >= 0);

	return !!ret;
}

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __nothrow
usig_procmask(int                         how,
              const sigset_t * __restrict set,
              sigset_t * __restrict       oldset)
{
	usig_assert(set || oldset);

	usig_assert(!sigprocmask(how, set, oldset));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __nothrow
usig_procmask(int                         how,
              const sigset_t * __restrict set,
              sigset_t * __restrict       oldset)
{
	sigprocmask(how, set, oldset);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

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
                   unsigned int              nr) __usig_nonull(1) __nothrow;

extern void
usig_restore_actions(const struct usig_orig_act orig[__restrict_arr],
                     unsigned int               nr) __usig_nonull(1) __nothrow;

#if defined(CONFIG_UTILS_SIGNAL_FD)

#include <utils/fd.h>
#include <sys/signalfd.h>

extern int
usig_read_fd(int fd, struct signalfd_siginfo * infos, unsigned int count)
	__usig_nonull(2) __leaf __warn_result;

static inline int __usig_nonull(1) __nothrow
usig_open_fd(const sigset_t * mask, int flags)
{
	usig_assert(mask);
	usig_assert(!usig_isemptyset(mask));
	usig_assert(!(flags & ~(SFD_NONBLOCK | SFD_CLOEXEC)));

	int fd;

	fd = signalfd(-1, mask, flags);
	if (fd < 0) {
		usig_assert(errno != EBADF);
		usig_assert(errno != EINVAL);

		return -errno;
	}

	return fd;
}

static inline int
usig_close_fd(int fd)
{
	usig_assert(fd >= 0);

	int ret;

	ret = ufd_close(fd);

	usig_assert(!ret || (ret == -EINTR));

	return ret;
}

#endif /* defined(CONFIG_UTILS_SIGNAL_FD) */

#endif /* _UTILS_SIGNAL_H */
