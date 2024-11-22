/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _ETUX_TIMER_INTERN_H
#define _ETUX_TIMER_INTERN_H

#include "utils/timer.h"

/*
 * Timer ticks sub second precision bits.
 *
 * Configure the tick period to 1/(2^ETUX_TIMER_TICK_SUBSEC_BITS) second.
 *
 * The table below gives tick sub second precision according to allowed
 * ETUX_TIMER_TICK_SUBSEC_BITS values:
 *
 *     ETUX_TIMER_TICK_SUBSEC_BITS     Tick period  Tick frequency
 *                                  (milliseconds)         (Hertz)
 *                               0    1000.000000               1
 *                               1     500.000000               2
 *                               2     250.000000               4
 *                               3     125.000000               8
 *                               4      62.500000              16
 *                               5      31.250000              32
 *                               6      15.625000              64
 *                               7       7.812500             128
 *                               8       3.906250             256
 *                               9       1.953125             512
 *
 * Watch out!
 * The tick period MUST be a divisor of 1000000000 nanoseconds so that we can
 * perform power of 2 arithmetics (see etux_timer_tick_from_tspec_lower(),
 * etux_timer_tick_from_tspec_upper() and etux_timer_tspec_from_tick()).
 * This is the reason why ETUX_TIMER_TICK_SUBSEC_BITS MUST be < 10.
 */
#define ETUX_TIMER_TICK_SUBSEC_BITS \
	STROLL_CONCAT(CONFIG_ETUX_TIMER_SUBSEC_BITS, U)
#if (ETUX_TIMER_TICK_SUBSEC_BITS < 0) || (ETUX_TIMER_TICK_SUBSEC_BITS > 9)
#error Invalid tick sub second precision bits.
#endif

#define ETUX_TIMER_TICK_SUBSEC_MASK \
	((INT64_C(1) << ETUX_TIMER_TICK_SUBSEC_BITS) - 1)

/* Period of a tick in nanoseconds */
#define ETUX_TIMER_TICK_NSEC \
	(INT64_C(1000000000) >> ETUX_TIMER_TICK_SUBSEC_BITS)

/* Number of ticks per second. */
#define ETUX_TIMER_TICKS_PER_SEC \
	(1UL << ETUX_TIMER_TICK_SUBSEC_BITS)

/*
 * Maximum tick value that can be encoded.
 *
 * Allows to prevent overflow while converting a tick's second part to the
 * tv_sec field of a struct timespec (tv_sec is a time_t, i.e., either a signed
 * 64 or 32 bits word).
 */
#if ETUX_TIME_TIMET_BITS == 64

#define ETUX_TIMER_TICK_MAX \
	(INT64_MAX)

#elif ETUX_TIME_TIMET_BITS == 32

#define ETUX_TIMER_TICK_MAX \
	(((int64_t)(INT32_MAX) << ETUX_TIMER_TICK_SUBSEC_BITS) | \
	 ETUX_TIMER_TICK_SUBSEC_MASK)

#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

/*
 * Maximum value of a struct timespec's tv_sec field that can be converted to a
 * tick.
 */
#define ETUX_TIMER_TVSEC_MAX \
	((time_t)(ETUX_TIMER_TICK_MAX >> ETUX_TIMER_TICK_SUBSEC_BITS))

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_timer_assert_intern(_expr) \
	stroll_assert("etux:timer", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_timer_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_timer_assert_timer_intern(_timer) \
	etux_timer_assert_intern(_timer); \
	etux_timer_assert_intern(((_timer)->state != ETUX_TIMER_PEND_STAT) || \
	                         (!stroll_dlist_empty(&(_timer)->node) && \
	                          (_timer)->expire)); \
	etux_timer_assert_intern(((_timer)->state != ETUX_TIMER_RUN_STAT) || \
	                         (_timer)->expire)

static inline __utils_nonull(1) __utils_pure __utils_nothrow __returns_nonull
struct etux_timer *
etux_timer_lead_timer(const struct stroll_dlist_node * __restrict head)
{
	etux_timer_assert_intern(head);
	etux_timer_assert_intern(!stroll_dlist_empty(head));

	struct etux_timer * tmr = stroll_dlist_entry(stroll_dlist_next(head),
	                                             struct etux_timer,
	                                             node);

	etux_timer_assert_timer_intern(tmr);
	etux_timer_assert_intern(tmr->expire);

	return tmr;
}

extern int
etux_timer_tick_cmp(const struct stroll_dlist_node * __restrict first,
                    const struct stroll_dlist_node * __restrict second,
                    void *                                      data __unused)
	__utils_nonull(1, 2)
	__utils_pure
	__utils_nothrow
	__leaf
	__warn_result
	__export_intern;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
void
etux_timer_insert(struct stroll_dlist_node * __restrict list,
                  struct etux_timer * __restrict        timer)
{
	stroll_dlist_insert_inorder_back(list,
	                                 &timer->node,
	                                 etux_timer_tick_cmp,
	                                 NULL);
}

static __utils_nonull(1) __utils_nothrow
void
etux_timer_dismiss(struct etux_timer * __restrict timer)
{
	etux_timer_assert_timer_intern(timer);

	stroll_dlist_remove(&timer->node);
	timer->state = ETUX_TIMER_IDLE_STAT;
}

extern int64_t
etux_timer_tick(void) __utils_nothrow __leaf __warn_result __export_intern;

#endif /* _ETUX_TIMER_INTERN_H */
