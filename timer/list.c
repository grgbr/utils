/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "common.h"
#include <errno.h>

static struct stroll_dlist_node etux_timer_the_list =
	STROLL_DLIST_INIT(etux_timer_the_list);

static __utils_nonull(1) __utils_nothrow
void
etux_timer_arm(struct etux_timer * __restrict timer)
{
	etux_timer_assert_timer_intern(timer);
	etux_timer_assert_intern(timer->expire);

	if (timer->state == ETUX_TIMER_PEND_STAT)
		stroll_dlist_remove(&timer->node);

	timer->tick = etux_timer_tick_from_tspec_upper_clamp(&timer->tspec);
	etux_timer_insert_inorder(&etux_timer_the_list, timer);
	timer->state = ETUX_TIMER_PEND_STAT;
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
	etux_timer_arm(timer);

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

	etux_timer_arm(timer);

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

	etux_timer_arm(timer);

	etux_timer_arm_sec_trace_exit(timer);
}

void
etux_timer_cancel(struct etux_timer * __restrict timer)
{
	etux_timer_assert_timer_api(timer);

	etux_timer_cancel_trace_enter(timer);

	if (timer->state == ETUX_TIMER_PEND_STAT) {
		timer->state = ETUX_TIMER_IDLE_STAT;
		stroll_dlist_remove(&timer->node);
	}

	etux_timer_cancel_trace_exit(timer);
}

int64_t
etux_timer_issue_tick(void)
{
	if (!stroll_dlist_empty(&etux_timer_the_list))
		return etux_timer_lead_timer(&etux_timer_the_list)->tick;
	else
		return (int64_t)-ENOENT;
}

void
etux_timer_run(void)
{
	int64_t         tick = -1;
	struct timespec now;

	etux_timer_run_trace_enter();

	while (!stroll_dlist_empty(&etux_timer_the_list)) {
		struct etux_timer * tmr;

		tmr = etux_timer_lead_timer(&etux_timer_the_list);
		if (tick < tmr->tick) {
			utime_monotonic_now(&now);
			tick = etux_timer_tick_from_tspec_lower_clamp(&now);
			if (tick < tmr->tick)
				goto out;
		}

		tmr->state = ETUX_TIMER_RUN_STAT;
		stroll_dlist_remove(&tmr->node);

		etux_timer_expire_trace_enter(tmr, &now, tick);
		tmr->expire(tmr);
		etux_timer_expire_trace_exit(tmr);

		if (tmr->state == ETUX_TIMER_RUN_STAT)
			tmr->state = ETUX_TIMER_IDLE_STAT;
	}

out:
	etux_timer_run_trace_exit();
}
