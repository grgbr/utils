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

/******************************************************************************
 * Timer handling
 ******************************************************************************/

struct utimer;

typedef void (utimer_expire_fn)(struct utimer * __restrict);

enum utimer_state {
	UTIMER_IDLE_STAT,
	UTIMER_PEND_STAT
};

struct utimer {
	enum utimer_state        state;
	struct stroll_dlist_node node;
	int64_t                  tick;
	struct timespec          tspec;
	utimer_expire_fn *       expire;
};

#define UTIMER_INIT(_timer, _expire) \
	{ \
		.state  = UTIMER_IDLE_STAT, \
		.node   = STROLL_DLIST_INIT((_timer).node), \
		.expire = _expire \
	}

#define utimer_assert_timer_api(_timer) \
	utimer_assert_api(_timer); \
	utimer_assert_api(((_timer)->state != UTIMER_PEND_STAT) || \
	                  (!stroll_dlist_empty(&(_timer)->node) && \
	                   (_timer)->expire))

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
utimer_is_armed(const struct utimer * __restrict timer)
{
	utimer_assert_timer_api(timer);

	return timer->state == UTIMER_PEND_STAT;
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
const struct timespec *
utimer_expiry_tspec(const struct utimer * __restrict timer)
{
	utimer_assert_timer_api(timer);

	return &timer->tspec;
}

extern void
utimer_arm_tspec(struct utimer * __restrict         timer,
                 const struct timespec * __restrict tspec)
	__utils_nonull(1, 2) __utils_nothrow __leaf;

extern void
utimer_arm_msec(struct utimer * __restrict timer, int msec)
	__utils_nonull(1) __utils_nothrow __leaf;

extern void
utimer_arm_sec(struct utimer * __restrict timer, int sec)
	__utils_nonull(1) __utils_nothrow __leaf;

#if defined(CONFIG_UTILS_TIMER_LIST)

static inline __utils_nonull(1) __utils_nothrow
void
utimer_cancel(struct utimer * __restrict timer)
{
	utimer_assert_timer_api(timer);

	if (timer->state == UTIMER_PEND_STAT) {
		stroll_dlist_remove(&timer->node);
		timer->state = UTIMER_IDLE_STAT;
	}
}

#endif /* defined(CONFIG_UTILS_TIMER_LIST) */

#if defined(CONFIG_UTILS_TIMER_HWHEEL)

extern void
utimer_cancel(struct utimer * __restrict timer)
	__utils_nonull(1) __utils_nothrow __leaf;

#endif /* defined(CONFIG_UTILS_TIMER_HWHEEL) */

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
utimer_init(struct utimer * __restrict timer, utimer_expire_fn * expire)
{
	utimer_assert_api(timer);

	timer->state = UTIMER_IDLE_STAT;
	stroll_dlist_init(&timer->node);
	timer->expire = expire;
}

extern struct timespec *
utimer_issue_tspec(struct timespec * __restrict tspec)
	__utils_nonull(1) __utils_nothrow __leaf __warn_result;

extern int
utimer_issue_msec(void) __utils_nothrow __warn_result;

extern void utimer_run(void) __utils_nothrow;

#endif /* _UTILS_TIMER_H */
