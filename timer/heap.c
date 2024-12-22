/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "common.h"
#include <errno.h>

static struct stroll_pprheap_base etux_timer_the_heap =
	STROLL_PPRHEAP_BASE_INIT;

static inline __utils_nonull(1) __utils_const __utils_nothrow __returns_nonull
struct etux_timer *
etux_timer_from_heap_node(const struct stroll_pprheap_node * __restrict node)
{
	etux_timer_assert_intern(node);

	return stroll_pprheap_entry(node, struct etux_timer, heap);
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __returns_nonull
struct etux_timer *
etux_timer_heap_lead_timer(const struct stroll_pprheap_base * __restrict heap)
{
	etux_timer_assert_intern(heap);
	etux_timer_assert_intern(!stroll_pprheap_base_isempty(heap));

	struct etux_timer * tmr =
		etux_timer_from_heap_node(stroll_pprheap_base_peek(heap));

	etux_timer_assert_timer_intern(tmr);
	etux_timer_assert_intern(tmr->expire);

	return tmr;
}

static __utils_nonull(1, 2) __utils_pure __utils_nothrow __warn_result
int
etux_timer_heap_tick_cmp(
	const struct stroll_pprheap_node * __restrict first,
	const struct stroll_pprheap_node * __restrict second,
	void *                                        data __unused)
{
	etux_timer_assert_intern(first);
	etux_timer_assert_intern(second);

	const struct etux_timer * fst = etux_timer_from_heap_node(first);
	const struct etux_timer * snd = etux_timer_from_heap_node(second);

	etux_timer_assert_intern(fst->tick >= 0);
	etux_timer_assert_intern(snd->tick >= 0);

	return (fst->tick > snd->tick) - (fst->tick < snd->tick);
}

static __utils_nonull(1, 2) __utils_nothrow
void
etux_timer_heap_insert(struct stroll_pprheap_base * __restrict heap,
                       struct etux_timer * __restrict          timer,
                       int64_t                                 tick)

{
	etux_timer_assert_intern(heap);
	etux_timer_assert_timer_intern(timer);
	etux_timer_assert_intern(timer->expire);
	etux_timer_assert_intern(tick >= 0);
	etux_timer_assert_intern(tick <= ETUX_TIMER_TICK_MAX);

	timer->state = ETUX_TIMER_PEND_STAT;
	timer->tick = tick;

	stroll_pprheap_base_insert(heap,
	                           &timer->heap,
	                           etux_timer_heap_tick_cmp,
	                           NULL);
}

static __utils_nonull(1, 2) __utils_nothrow
void
etux_timer_heap_adjust(struct stroll_pprheap_base * __restrict heap,
                       struct etux_timer * __restrict          timer,
                       int64_t                                 tick)

{
	etux_timer_assert_intern(heap);
	etux_timer_assert_timer_intern(timer);
	etux_timer_assert_intern(timer->expire);
	etux_timer_assert_intern(tick >= 0);
	etux_timer_assert_intern(tick <= ETUX_TIMER_TICK_MAX);

	int64_t old = timer->tick;

	if (tick == old)
		return;

	timer->tick = tick;
	if (tick < old)
		stroll_pprheap_base_promote(heap,
		                            &timer->heap,
		                            etux_timer_heap_tick_cmp,
		                            NULL);
	else
		stroll_pprheap_base_demote(heap,
		                           &timer->heap,
		                           etux_timer_heap_tick_cmp,
		                           NULL);
}

static __utils_nonull(1, 2) __utils_nothrow
void
etux_timer_heap_arm(struct stroll_pprheap_base * __restrict heap,
                    struct etux_timer * __restrict          timer)

{
	etux_timer_assert_intern(heap);
	etux_timer_assert_timer_intern(timer);
	etux_timer_assert_intern(timer->expire);

	int64_t tick;

	tick = etux_timer_tick_from_tspec_upper_clamp(&timer->tspec);

	switch (timer->state) {
	case ETUX_TIMER_IDLE_STAT:
		etux_timer_heap_insert(heap, timer, tick);
		break;

STROLL_IGNORE_WARN("-Wimplicit-fallthrough")
	case ETUX_TIMER_RUN_STAT:
		timer->state = ETUX_TIMER_PEND_STAT;
STROLL_RESTORE_WARN

	case ETUX_TIMER_PEND_STAT:
		etux_timer_heap_adjust(heap, timer, tick);
		break;

	default:
		etux_timer_assert_intern(0);
	}
}

void
etux_timer_arm_tspec(struct etux_timer * __restrict     timer,
                     const struct timespec * __restrict tspec)
{
	etux_timer_assert_timer_api(timer);
	etux_timer_assert_api(timer->expire);
	utime_assert_tspec_api(tspec);

	etux_timer_arm_tspec_trace_enter(timer, tspec);

	timer->tspec = *tspec;
	etux_timer_heap_arm(&etux_timer_the_heap, timer);

	etux_timer_arm_tspec_trace_exit(timer);
}

void
etux_timer_arm_msec(struct etux_timer * __restrict timer, int msec)
{
	etux_timer_assert_timer_api(timer);
	etux_timer_assert_api(timer->expire);
	etux_timer_assert_api(msec >= 0);

	etux_timer_arm_msec_trace_enter(timer, msec);

	utime_monotonic_now(&timer->tspec);
	utime_tspec_add_msec_clamp(&timer->tspec, msec);

	etux_timer_heap_arm(&etux_timer_the_heap, timer);

	etux_timer_arm_msec_trace_exit(timer);
}

void
etux_timer_arm_sec(struct etux_timer * __restrict timer, int sec)
{
	etux_timer_assert_timer_api(timer);
	etux_timer_assert_api(timer->expire);
	etux_timer_assert_api(sec >= 0);

	etux_timer_arm_sec_trace_enter(timer, sec);

	utime_monotonic_now(&timer->tspec);
	utime_tspec_add_sec_clamp(&timer->tspec, sec);

	etux_timer_heap_arm(&etux_timer_the_heap, timer);

	etux_timer_arm_sec_trace_exit(timer);
}

void
etux_timer_cancel(struct etux_timer * __restrict timer)
{
	etux_timer_assert_timer_api(timer);

	etux_timer_cancel_trace_enter(timer);

	if (timer->state == ETUX_TIMER_PEND_STAT) {
		timer->state = ETUX_TIMER_IDLE_STAT;
		stroll_pprheap_base_remove(&etux_timer_the_heap,
		                           &timer->heap,
		                           etux_timer_heap_tick_cmp,
		                           NULL);
	}

	etux_timer_cancel_trace_exit(timer);
}

int64_t
etux_timer_issue_tick(void)
{
	if (!stroll_pprheap_base_isempty(&etux_timer_the_heap))
		return etux_timer_heap_lead_timer(&etux_timer_the_heap)->tick;
	else
		return (int64_t)-ENOENT;
}

void
etux_timer_run(void)
{
	int64_t         tick = -1;
	struct timespec now;

	etux_timer_run_trace_enter();

	while (!stroll_pprheap_base_isempty(&etux_timer_the_heap)) {
		struct etux_timer * tmr;

		tmr = etux_timer_heap_lead_timer(&etux_timer_the_heap);
		if (tick < tmr->tick) {
			tick = etux_timer_tick_load(&now);
			if (tick < tmr->tick)
				goto out;
		}

		tmr->state = ETUX_TIMER_RUN_STAT;

		etux_timer_expire_trace_enter(tmr, &now, tick);
		tmr->expire(tmr);
		etux_timer_expire_trace_exit(tmr);

		if (tmr->state == ETUX_TIMER_RUN_STAT) {
			tmr->state = ETUX_TIMER_IDLE_STAT;
			stroll_pprheap_base_remove(&etux_timer_the_heap,
			                           &tmr->heap,
			                           etux_timer_heap_tick_cmp,
			                           NULL);
		}
	}

out:
	etux_timer_run_trace_exit();
}
