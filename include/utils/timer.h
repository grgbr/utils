/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Timer interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2021
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _ETUX_TIMER_H
#define _ETUX_TIMER_H

#include <utils/time.h>
#include <stroll/dlist.h>
#if defined(CONFIG_ETUX_TIMER_HEAP)
#include <stroll/pprheap.h>
#endif /* defined(CONFIG_ETUX_TIMER_HEAP) */

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_timer_assert_api(_expr) \
	stroll_assert("etux:timer", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_timer_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

/******************************************************************************
 * Timer handling
 ******************************************************************************/

struct etux_timer;

typedef void (etux_timer_expire_fn)(struct etux_timer * __restrict);

enum etux_timer_state {
	ETUX_TIMER_IDLE_STAT,
	ETUX_TIMER_PEND_STAT,
	ETUX_TIMER_RUN_STAT
};

struct etux_timer {
	enum etux_timer_state              state;
	union {
		struct stroll_dlist_node   list;
#if defined(CONFIG_ETUX_TIMER_HEAP)
		struct stroll_pprheap_node heap;
#endif /* defined(CONFIG_ETUX_TIMER_HEAP) */
	};
	int64_t                            tick;
	struct timespec                    tspec;
	etux_timer_expire_fn *             expire;
};

#define ETUX_TIMER_INIT(_timer, _expire) \
	{ \
		.state  = ETUX_TIMER_IDLE_STAT, \
		.expire = _expire \
	}

#define etux_timer_assert_timer_api(_timer) \
	etux_timer_assert_api(_timer); \
	etux_timer_assert_api(((_timer)->state != ETUX_TIMER_PEND_STAT) || \
	                      (_timer)->expire); \
	etux_timer_assert_api(((_timer)->state != ETUX_TIMER_RUN_STAT) || \
	                      (_timer)->expire)

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
etux_timer_is_armed(const struct etux_timer * __restrict timer)
{
	etux_timer_assert_timer_api(timer);

	return timer->state == ETUX_TIMER_PEND_STAT;
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
const struct timespec *
etux_timer_expiry_tspec(const struct etux_timer * __restrict timer)
{
	etux_timer_assert_timer_api(timer);

	return &timer->tspec;
}

extern void
etux_timer_arm_tspec(struct etux_timer * __restrict     timer,
                     const struct timespec * __restrict tspec)
	__utils_nonull(1, 2) __utils_nothrow __leaf __export_public;

extern void
etux_timer_arm_msec(struct etux_timer * __restrict timer, int msec)
	__utils_nonull(1) __utils_nothrow __leaf __export_public;

extern void
etux_timer_arm_sec(struct etux_timer * __restrict timer, int sec)
	__utils_nonull(1) __utils_nothrow __leaf __export_public;

extern void
etux_timer_cancel(struct etux_timer * __restrict timer)
	__utils_nonull(1) __utils_nothrow __leaf __export_public;

static inline __utils_nonull(1, 2) __utils_nothrow
void
etux_timer_setup(struct etux_timer * __restrict timer,
                 etux_timer_expire_fn *         expire)
{
	etux_timer_assert_api(timer);
	etux_timer_assert_api(expire);

	timer->expire = expire;
}

static inline __utils_nonull(1) __utils_nothrow
void
etux_timer_init(struct etux_timer * __restrict timer,
                etux_timer_expire_fn *         expire)
{
	etux_timer_assert_api(timer);

	timer->state = ETUX_TIMER_IDLE_STAT;
	timer->expire = expire;
}

extern struct timespec *
etux_timer_issue_tspec(struct timespec * __restrict tspec)
	__utils_nonull(1) __utils_nothrow __leaf __warn_result __export_public;

extern int
etux_timer_issue_msec(void) __utils_nothrow __warn_result __export_public;

extern void etux_timer_run(void) __utils_nothrow __export_public;

#if defined(CONFIG_UTILS_TIMER_FD)

#include <utils/fd.h>
#include <sys/timerfd.h>

static inline __utils_nonull(2) __warn_result;
ssize_t
etux_timerfd_state(int fd, uint64_t * count)
{
	ssize_t ret;

	ret = ufd_read(fd, (char *)count, sizeof(*count));
	if (ret >= 0) {
		etux_timer_assert_intern(!ret || (ret == sizeof(*count)));
		return ret;
	}

	etux_timer_assert_api(errno != EIO);

	return -errno;
}

static inline __utils_nonull(2) __utils_nothrow
void
etux_timerfd_settings(int fd, struct itimerspec * current)
{
	etux_timer_assert_api(fd >= 0);
	etux_timer_assert_api(current);

	int err;

	err = timerfd_gettime(fd, current);
	if (!err)
		return 0;

	etux_timer_assert_api(errno != EBADF);
	etux_timer_assert_api(errno != EINVAL);
	etux_timer_assert_api(errno != EFAULT);

	return -errno;
}

static inline __utils_nonull(3) __utils_nothrow
int
etux_timerfd_arm(int                       fd,
                 int                       flags,
                 const struct itimerspec * expire,
                 struct itimerspec *       old)
{
	etux_timer_assert_api(fd >= 0);
	etux_timer_assert_api(!(flags & ~(TFD_TIMER_ABSTIME |
	                                  TFD_TIMER_CANCEL_ON_SET)));
	etux_timer_assert_api(expire);
	utime_assert_tspec_api(&expire->it_value);
	etux_timer_assert_api(!!expire->it_value.tv_sec ||
	                      !!expire->it_value.tv_nsec);

	int err;

	err = timerfd_settime(fd, flags, expire, old);
	if (!err)
		return 0;

	etux_timer_assert_api(errno != EBADF);
	etux_timer_assert_api(errno != EFAULT);
	etux_timer_assert_api(errno != EINVAL);

	return -errno;
}

static inline __utils_nothrow
void
etux_timerfd_cancel(int fd, struct itimerspec * old)
{
	etux_timer_assert_api(fd >= 0);

	const struct itimerspec xpr = {
		.it_interval = { 0, },
		.it_value    = { 0, }
	};
	int                     err;

	err = timerfd_settime(fd, 0, xpr, old);
	etux_timer_assert_api(!err);
}

static inline __utils_nothrow __warn_result
int
etux_timerfd_open(clockid_t clockid, int flags)
{
	etux_timer_assert_api((clockid == CLOCK_REALTIME) ||
	                      (clockid == CLOCK_MONOTONIC) ||
	                      (clockid == CLOCK_BOOTTIME) ||
	                      (clockid == CLOCK_REALTIME_ALARM) ||
	                      (clockid == CLOCK_BOOTTIME_ALARM));
	etux_timer_assert_api(!(flags & ~(TFD_NONBLOCK | TFD_CLOEXEC)));

	int fd;

	fd = timerfd_create(clockid, flags);
	if (fd >= 0)
		return fd;

	etux_timer_assert_api(errno != EINVAL);

	return -errno;
}

static inline
int
etux_timerfd_close(int fd)
{
	api(fd >= 0);

	int ret;

	ret = ufd_close(fd);

	etux_timer_assert_api(!ret || (ret == -EINTR));

	return ret;
}

#endif /* defined(CONFIG_UTILS_TIMER_FD) */

#endif /* _ETUX_TIMER_H */
