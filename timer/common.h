/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _ETUX_TIMER_COMMON_H
#define _ETUX_TIMER_COMMON_H

#include "utils/timer.h"

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

/******************************************************************************
 * Tick handling
 ******************************************************************************/

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
 *                               0     1000.000000               1
 *                               1      500.000000               2
 *                               2      250.000000               4
 *                               3      125.000000               8
 *                               4       62.500000              16
 *                               5       31.250000              32
 *                               6       15.625000              64
 *                               7        7.812500             128
 *                               8        3.906250             256
 *                               9        1.953125             512
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

/* Tick frequency, i.e., number of ticks per second. */
#define ETUX_TIMER_TICKS_PER_SEC \
	(1UL << ETUX_TIMER_TICK_SUBSEC_BITS)

/*
 * Maximum tick value that can be encoded.
 *
 * Allows to prevent overflow while converting a tick's second part to the
 * tv_sec field of a struct timespec (tv_sec is a time_t, i.e., either a signed
 * 64 or 32 bits word).
 */
#if UTIME_TIMET_BITS == 64

#define ETUX_TIMER_TICK_MAX \
	(INT64_MAX)

#elif UTIME_TIMET_BITS == 32

#define ETUX_TIMER_TICK_MAX \
	(((int64_t)(INT32_MAX) << ETUX_TIMER_TICK_SUBSEC_BITS) | \
	 ETUX_TIMER_TICK_SUBSEC_MASK)

#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

extern int64_t
etux_timer_tick_from_tspec_lower(const struct timespec * __restrict tspec)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__warn_result
	__export_intern;

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
etux_timer_tick_from_tspec_lower_clamp(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	int64_t tick = etux_timer_tick_from_tspec_lower(tspec);

	return (tick >= 0) ? tick : ETUX_TIMER_TICK_MAX;
}

extern int64_t
etux_timer_tick_from_tspec_upper(const struct timespec * __restrict tspec)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__warn_result
	__export_intern;

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
etux_timer_tick_from_tspec_upper_clamp(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	int64_t tick = etux_timer_tick_from_tspec_upper(tspec);

	return (tick >= 0) ? tick : ETUX_TIMER_TICK_MAX;
}

/******************************************************************************
 * Timer generic logic
 ******************************************************************************/

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

extern void
etux_timer_insert_inorder(struct stroll_dlist_node * __restrict list,
                          struct etux_timer * __restrict        timer)
	__utils_nonull(1, 2) __utils_nothrow __leaf __export_intern;

extern int64_t
etux_timer_issue_tick(void)
	__utils_nothrow __warn_result __leaf __export_intern;

/******************************************************************************
 * Tracing handling
 ******************************************************************************/

#if defined(CONFIG_ETUX_TRACE)

#include "trace.i"

static inline
void
etux_timer_arm_tspec_trace_enter(const struct etux_timer * __restrict timer,
                                 const struct timespec * __restrict   tspec)
{
	lttng_ust_tracepoint(etux_timer, arm_tspec_enter_trcevt, timer, tspec);
}

static inline
void
etux_timer_arm_tspec_trace_exit(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux_timer, arm_tspec_exit_trcevt, timer);
}

static inline
void
etux_timer_arm_msec_trace_enter(const struct etux_timer * __restrict timer,
                                int                                  msec)
{
	lttng_ust_tracepoint(etux_timer, arm_msec_enter_trcevt, timer, msec);
}

static inline
void
etux_timer_arm_msec_trace_exit(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux_timer, arm_msec_exit_trcevt, timer);
}

static inline
void
etux_timer_arm_sec_trace_enter(const struct etux_timer * __restrict timer,
                               int                                  sec)
{
	lttng_ust_tracepoint(etux_timer, arm_sec_enter_trcevt, timer, sec);
}

static inline
void
etux_timer_arm_sec_trace_exit(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux_timer, arm_sec_exit_trcevt, timer);
}

static inline
void
etux_timer_cancel_trace_enter(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux_timer, cancel_enter_trcevt, timer);
}

static inline
void
etux_timer_cancel_trace_exit(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux_timer, cancel_exit_trcevt, timer);
}

static inline
void
etux_timer_issue_tspec_trace_enter(void)
{
	lttng_ust_tracepoint(etux_timer, issue_tspec_enter_trcevt);
}

static inline
void
etux_timer_issue_tspec_trace_exit(const struct timespec * __restrict tspec)
{
	time_t sec = tspec ? tspec->tv_sec : -1;
	long   nsec = tspec ? tspec->tv_nsec : -1;

	lttng_ust_tracepoint(etux_timer, issue_tspec_exit_trcevt, sec, nsec);
}

static inline
void
etux_timer_issue_msec_trace_enter(void)
{
	lttng_ust_tracepoint(etux_timer, issue_msec_enter_trcevt);
}

static inline
void
etux_timer_issue_msec_trace_exit(int msec)
{
	lttng_ust_tracepoint(etux_timer, issue_msec_exit_trcevt, msec);
}

static inline
void
etux_timer_run_trace_enter(void)
{
	lttng_ust_tracepoint(etux_timer, run_enter_trcevt);
}

static inline
void
etux_timer_run_trace_exit(void)
{
	lttng_ust_tracepoint(etux_timer, run_exit_trcevt);
}

static inline
void
etux_timer_expire_trace_enter(const struct etux_timer * __restrict timer,
                              const struct timespec * __restrict   now,
                              int64_t                              tick)
{
	lttng_ust_tracepoint(etux_timer, expire_enter_trcevt, timer, now, tick);
}

static inline
void
etux_timer_expire_trace_exit(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux_timer, expire_exit_trcevt, timer);
}

#else  /* !defined(CONFIG_ETUX_TRACE) */

static inline
void
etux_timer_arm_tspec_trace_enter(
	const struct etux_timer * __restrict timer __unused,
	const struct timespec * __restrict   tspec __unused)
{
}

static inline
void
etux_timer_arm_tspec_trace_exit(
	const struct etux_timer * __restrict timer __unused)
{
}

static inline
void
etux_timer_arm_msec_trace_enter(
	const struct etux_timer * __restrict timer __unused,
	int                                  msec __unused)
{
}

static inline
void
etux_timer_arm_msec_trace_exit(
	const struct etux_timer * __restrict timer __unused)
{
}

static inline
void
etux_timer_arm_sec_trace_enter(
	const struct etux_timer * __restrict timer __unused,
	int                                  sec __unused)
{
}

static inline
void
etux_timer_arm_sec_trace_exit(
	const struct etux_timer * __restrict timer __unused)
{
}

static inline
void
etux_timer_cancel_trace_enter(
        const struct etux_timer * __restrict timer __unused)
{
}

static inline
void
etux_timer_cancel_trace_exit(
        const struct etux_timer * __restrict timer __unused)
{
}

static inline
void
etux_timer_issue_tspec_trace_enter(void)
{
}

static inline
void
etux_timer_issue_tspec_trace_exit(
	const struct timespec * __restrict tspec __unused)
{
}

static inline
void
etux_timer_issue_msec_trace_enter(void)
{
}

static inline
void
etux_timer_issue_msec_trace_exit(int msec __unused)
{
}

static inline
void
etux_timer_run_trace_enter(void)
{
}

static inline
void
etux_timer_run_trace_exit(void)
{
}

static inline
void
etux_timer_expire_trace_enter(
	const struct etux_timer * __restrict timer __unused,
	const struct timespec * __restrict   now __unused,
	int64_t                              tick __unused)
{
}

static inline
void
etux_timer_expire_trace_exit(
	const struct etux_timer * __restrict timer __unused)
{
}

#endif /* defined(CONFIG_ETUX_TRACE) */

#endif /* _ETUX_TIMER_COMMON_H */
