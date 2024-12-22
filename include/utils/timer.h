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

#endif /* _ETUX_TIMER_H */
