/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _ETUX_TIMER_COMMON_H
#define _ETUX_TIMER_COMMON_H

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

#if defined(CONFIG_ETUX_TRACE)

#include "trace.i"

static inline
void
etux_timer_arm_tspec_trace_enter(const struct etux_timer * __restrict timer,
                                 const struct timespec * __restrict   tspec)
{
	lttng_ust_tracepoint(etux,
	                     etux_timer_arm_tspec_enter_trcevt,
	                     timer,
	                     tspec);
}

static inline
void
etux_timer_arm_tspec_trace_exit(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux, etux_timer_arm_tspec_exit_trcevt, timer);
}

static inline
void
etux_timer_arm_msec_trace_enter(const struct etux_timer * __restrict timer,
                                int                                  msec)
{
	lttng_ust_tracepoint(etux,
	                     etux_timer_arm_msec_enter_trcevt,
	                     timer,
	                     msec);
}

static inline
void
etux_timer_arm_msec_trace_exit(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux, etux_timer_arm_msec_exit_trcevt, timer);
}

static inline
void
etux_timer_arm_sec_trace_enter(const struct etux_timer * __restrict timer,
                               int                                  sec)
{
	lttng_ust_tracepoint(etux,
	                     etux_timer_arm_sec_enter_trcevt,
	                     timer,
	                     sec);
}

static inline
void
etux_timer_arm_sec_trace_exit(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux, etux_timer_arm_sec_exit_trcevt, timer);
}

static inline
void
etux_timer_cancel_trace_enter(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux, etux_timer_cancel_enter_trcevt, timer);
}

static inline
void
etux_timer_cancel_trace_exit(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux, etux_timer_cancel_exit_trcevt, timer);
}

static inline
void
etux_timer_issue_tspec_trace_enter(void)
{
	lttng_ust_tracepoint(etux, etux_timer_issue_tspec_enter_trcevt);
}

static inline
void
etux_timer_issue_tspec_trace_exit(const struct timespec * __restrict tspec)
{
	time_t sec = tspec ? tspec->tv_sec : -1;
	long   nsec = tspec ? tspec->tv_nsec : -1;

	lttng_ust_tracepoint(etux,
                             etux_timer_issue_tspec_exit_trcevt,
                             sec,
                             nsec);
}

static inline
void
etux_timer_issue_msec_trace_enter(void)
{
	lttng_ust_tracepoint(etux, etux_timer_issue_msec_enter_trcevt);
}

static inline
void
etux_timer_issue_msec_trace_exit(int msec)
{
	lttng_ust_tracepoint(etux, etux_timer_issue_msec_exit_trcevt, msec);
}

static inline
void
etux_timer_run_trace_enter(void)
{
	lttng_ust_tracepoint(etux, etux_timer_run_enter_trcevt);
}

static inline
void
etux_timer_run_trace_exit(void)
{
	lttng_ust_tracepoint(etux, etux_timer_run_exit_trcevt);
}

static inline
void
etux_timer_expire_trace_enter(const struct etux_timer * __restrict timer,
                              const struct timespec * __restrict   now,
                              int64_t                              tick)
{
	lttng_ust_tracepoint(etux,
	                     etux_timer_expire_enter_trcevt,
	                     timer,
	                     now,
	                     tick);
}

static inline
void
etux_timer_expire_trace_exit(const struct etux_timer * __restrict timer)
{
	lttng_ust_tracepoint(etux, etux_timer_expire_exit_trcevt, timer);
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
