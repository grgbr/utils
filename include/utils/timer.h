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

#ifndef _UTILS_TIMER_H
#define _UTILS_TIMER_H

#include <utils/time.h>
#include <stroll/dlist.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define utimer_assert_api(_expr) \
	stroll_assert("utils:utimer", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define utimer_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

struct utimer;

typedef void (utimer_expire_fn)(struct utimer * __restrict);

struct utimer {
	struct stroll_dlist_node node;
	struct timespec          date;
	utimer_expire_fn *       expire;
};

#define UTIMER_INIT(_timer) \
	{ .node   = STROLL_DLIST_INIT((_timer).node) }

#define utimer_assert_timer(_timer) \
	utimer_assert_api(_timer); \
	utimer_assert_api((_timer)->expire)

static inline __utils_nonull(1) __utils_pure  __utils_nothrow __returns_nonull
struct timespec *
utimer_expiry_date(const struct utimer * __restrict timer)
{
	utimer_assert_timer(timer);

	return (struct timespec *)&timer->date;
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
utimer_is_armed(const struct utimer * __restrict timer)
{
	utimer_assert_timer(timer);

	return !stroll_dlist_empty(&timer->node);
}

static inline __utils_nonull(1) __utils_nothrow
void
utimer_cancel(struct utimer * __restrict timer)
{
	utimer_assert_timer(timer);
	utime_assert_tspec(&timer->date);

	stroll_dlist_remove_init(&timer->node);
}

static inline __utils_nonull(1, 2) __utils_nothrow
void
utimer_setup(struct utimer * __restrict timer,
             utimer_expire_fn *         expire)
{
	utimer_assert_api(timer);
	utimer_assert_api(expire);

	timer->expire = expire;
}

static inline __utils_nonull(1) __utils_nothrow
void
utimer_init(struct utimer * __restrict timer)
{
	utimer_assert_api(timer);

	stroll_dlist_init(&timer->node);
}

extern void
utimer_arm(struct utimer * __restrict timer)
	__utils_nonull(1) __utils_nothrow __leaf;

extern void
utimer_arm_msec(struct utimer * __restrict timer, unsigned long msec)
	__utils_nonull(1) __utils_nothrow;

extern void
utimer_arm_sec(struct utimer * __restrict timer, unsigned long sec)
	__utils_nonull(1) __utils_nothrow;

extern const struct timespec *
utimer_issue_date(void) __utils_pure __utils_nothrow __leaf __warn_result;

extern long
utimer_issue_msec(void) __utils_nothrow __warn_result;

extern void utimer_run(void) __utils_nothrow;

#endif /* _UTILS_TIMER_H */
