/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#undef LTTNG_UST_TRACEPOINT_PROVIDER
#define LTTNG_UST_TRACEPOINT_PROVIDER etux_timer

#undef LTTNG_UST_TRACEPOINT_INCLUDE
#define LTTNG_UST_TRACEPOINT_INCLUDE "trace.i"

#if !defined(_ETUX_TIMER_TRACE_I) || \
    defined(LTTNG_UST_TRACEPOINT_HEADER_MULTI_READ)
#define _ETUX_TIMER_TRACE_I

#include "../trace/trace.h"
#include <lttng/tracepoint.h>

ETUX_TRACE_VOID_CLASS(etux_timer)

LTTNG_UST_TRACEPOINT_ENUM(
	etux_timer,
	state_enum,
	LTTNG_UST_TP_ENUM_VALUES(
		ETUX_TRACE_ENUM_VALUE(ETUX_TIMER_IDLE_STAT)
		ETUX_TRACE_ENUM_VALUE(ETUX_TIMER_PEND_STAT)
		ETUX_TRACE_ENUM_VALUE(ETUX_TIMER_RUN_STAT)
	)
)

LTTNG_UST_TRACEPOINT_EVENT_CLASS(
	etux_timer,
	exit_trccls,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer
	),
	LTTNG_UST_TP_FIELDS(
		lttng_ust_field_integer_hex(unsigned long, timer_addr, timer)
		lttng_ust_field_integer(int64_t, timer_tick, timer->tick)
		lttng_ust_field_integer(time_t, timer_sec, timer->tspec.tv_sec)
		lttng_ust_field_integer(long, timer_nsec, timer->tspec.tv_nsec)
	)
)

LTTNG_UST_TRACEPOINT_EVENT(
	etux_timer,
	arm_tspec_enter_trcevt,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer,
		const struct timespec *,   tspec
	),
	LTTNG_UST_TP_FIELDS(
		lttng_ust_field_integer_hex(unsigned long, timer_addr, timer)
		lttng_ust_field_enum(etux_timer,
		                     state_enum,
		                     enum etux_timer_state,
		                     timer_state,
		                     timer->state)
		lttng_ust_field_integer(int64_t, timer_tick, timer->tick)
		lttng_ust_field_integer(time_t, timer_sec, timer->tspec.tv_sec)
		lttng_ust_field_integer(long, timer_nsec, timer->tspec.tv_nsec)
		lttng_ust_field_integer(time_t, tspec_sec, tspec->tv_sec)
		lttng_ust_field_integer(long, tspec_nsec, tspec->tv_nsec)
	)
)

LTTNG_UST_TRACEPOINT_EVENT_INSTANCE(
	etux_timer,
	exit_trccls,
	etux_timer,
	arm_tspec_exit_trcevt,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer
	)
)

LTTNG_UST_TRACEPOINT_EVENT(
	etux_timer,
	arm_msec_enter_trcevt,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer,
		int,                       msec
	),
	LTTNG_UST_TP_FIELDS(
		lttng_ust_field_integer_hex(unsigned long, timer_addr, timer)
		lttng_ust_field_enum(etux_timer,
		                     state_enum,
		                     enum etux_timer_state,
		                     timer_state,
		                     timer->state)
		lttng_ust_field_integer(int64_t, timer_tick, timer->tick)
		lttng_ust_field_integer(time_t, timer_sec, timer->tspec.tv_sec)
		lttng_ust_field_integer(long, timer_nsec, timer->tspec.tv_nsec)
		lttng_ust_field_integer(int, msec, msec)
	)
)

LTTNG_UST_TRACEPOINT_EVENT_INSTANCE(
	etux_timer,
	exit_trccls,
	etux_timer,
	arm_msec_exit_trcevt,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer
	)
)

LTTNG_UST_TRACEPOINT_EVENT(
	etux_timer,
	arm_sec_enter_trcevt,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer,
		int,                       sec
	),
	LTTNG_UST_TP_FIELDS(
		lttng_ust_field_integer_hex(unsigned long, timer_addr, timer)
		lttng_ust_field_enum(etux_timer,
		                     state_enum,
		                     enum etux_timer_state,
		                     timer_state,
		                     timer->state)
		lttng_ust_field_integer(int64_t, timer_tick, timer->tick)
		lttng_ust_field_integer(time_t, timer_sec, timer->tspec.tv_sec)
		lttng_ust_field_integer(long, timer_nsec, timer->tspec.tv_nsec)
		lttng_ust_field_integer(int, sec, sec)
	)
)

LTTNG_UST_TRACEPOINT_EVENT_INSTANCE(
	etux_timer,
	exit_trccls,
	etux_timer,
	arm_sec_exit_trcevt,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer
	)
)

LTTNG_UST_TRACEPOINT_EVENT(
	etux_timer,
	cancel_enter_trcevt,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer
	),
	LTTNG_UST_TP_FIELDS(
		lttng_ust_field_integer_hex(unsigned long, timer_addr, timer)
		lttng_ust_field_enum(etux_timer,
		                     state_enum,
		                     enum etux_timer_state,
		                     timer_state,
		                     timer->state)
		lttng_ust_field_integer(int64_t, timer_tick, timer->tick)
		lttng_ust_field_integer(time_t, timer_sec, timer->tspec.tv_sec)
		lttng_ust_field_integer(long, timer_nsec, timer->tspec.tv_nsec)
	)
)

LTTNG_UST_TRACEPOINT_EVENT_INSTANCE(
	etux_timer,
	exit_trccls,
	etux_timer,
	cancel_exit_trcevt,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer
	)
)

ETUX_TRACE_VOID_EVENT(etux_timer, issue_tspec_enter_trcevt)

LTTNG_UST_TRACEPOINT_EVENT(
	etux_timer,
	issue_tspec_exit_trcevt,
	LTTNG_UST_TP_ARGS(
		time_t, sec,
		long,   nsec
	),
	LTTNG_UST_TP_FIELDS(
		lttng_ust_field_integer(time_t, issue_sec, sec)
		lttng_ust_field_integer(long, issue_nsec, nsec)
	)
)

ETUX_TRACE_VOID_EVENT(etux_timer, issue_msec_enter_trcevt)

LTTNG_UST_TRACEPOINT_EVENT(
	etux_timer,
	issue_msec_exit_trcevt,
	LTTNG_UST_TP_ARGS(
		int, msec
	),
	LTTNG_UST_TP_FIELDS(
		lttng_ust_field_integer(int, issue_msec, msec)
	)
)

ETUX_TRACE_VOID_EVENT(etux_timer, run_enter_trcevt)

ETUX_TRACE_VOID_EVENT(etux_timer, run_exit_trcevt)

LTTNG_UST_TRACEPOINT_EVENT(
	etux_timer,
	expire_enter_trcevt,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer,
		const struct timespec *,   now,
                int64_t,                   tick
	),
	LTTNG_UST_TP_FIELDS(
		lttng_ust_field_integer_hex(unsigned long, timer_addr, timer)
		lttng_ust_field_integer(int64_t, timer_tick, timer->tick)
		lttng_ust_field_integer(time_t, timer_sec, timer->tspec.tv_sec)
		lttng_ust_field_integer(long, timer_nsec, timer->tspec.tv_nsec)
		lttng_ust_field_integer(time_t, now_sec, now->tv_sec)
		lttng_ust_field_integer(long, now_nsec, now->tv_nsec)
		lttng_ust_field_integer(long, now_tick, tick)
	)
)

LTTNG_UST_TRACEPOINT_EVENT(
	etux_timer,
	expire_exit_trcevt,
	LTTNG_UST_TP_ARGS(
		const struct etux_timer *, timer
	),
	LTTNG_UST_TP_FIELDS(
		lttng_ust_field_integer_hex(unsigned long, timer_addr, timer)
		lttng_ust_field_enum(etux_timer,
		                     state_enum,
		                     enum etux_timer_state,
		                     timer_state,
		                     timer->state)
		lttng_ust_field_integer(int64_t, timer_tick, timer->tick)
		lttng_ust_field_integer(time_t, timer_sec, timer->tspec.tv_sec)
		lttng_ust_field_integer(long, timer_nsec, timer->tspec.tv_nsec)
	)
)

#endif /* _ETUX_TIMER_TRACE_I */

#include <lttng/tracepoint-event.h>

/* ex: set filetype=c : */
