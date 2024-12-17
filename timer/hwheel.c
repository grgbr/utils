/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "common.h"
#include <errno.h>

#define ETUX_TIMER_HWHEEL_SLOT_BITS \
	(6U)

#define ETUX_TIMER_HWHEEL_SLOT_MASK \
	((INT64_C(1) << ETUX_TIMER_HWHEEL_SLOT_BITS) - 1)

#define ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL \
	(1U << ETUX_TIMER_HWHEEL_SLOT_BITS)

/*
 * Setup maximum depth of hierarchical timing wheel.
 *
 * The table below gives time ranges that the hwheel may handle according to
 * to allowed ETUX_TIMER_TICK_SUBSEC_BITS values:
 *                                                      Tick                Time
 *                                    Time period  frequency  Hierarchy    range
 *    ETUX_TIMER_TICK_SUBSEC_BITS  (milliseconds)    (Hertz)     levels   (days)
 *                              0     1000.000000          1          4     >194
 *                              1      500.000000          2          4      >97
 *                              2      250.000000          4          4      >48
 *                              3      125.000000          8          4      >24
 *                              4       62.500000         16          4      >12
 *                              5       31.250000         32          5     >388
 *                              6       15.625000         64          5     >194
 *                              7        7.812500        128          5      >97
 *                              8        3.906250        256          5      >48
 *                              9        1.953125        512          5      >24
 */
#if ETUX_TIMER_TICK_SUBSEC_BITS < 5
#define ETUX_TIMER_HWHEEL_LEVELS_NR (4U)
#else  /* !(ETUX_TIMER_TICK_SUBSEC_BITS < 5) */
#define ETUX_TIMER_HWHEEL_LEVELS_NR (5U)
#endif /* ETUX_TIMER_TICK_SUBSEC_BITS < 5 */

/*
 * Maximum number of ticks that the hierarchical timer wheel may handle (without
 * accounting for sorted eternal timer list).
 */
#define ETUX_TIMER_HWHEEL_TICKS_NR \
	(INT64_C(1) << (ETUX_TIMER_HWHEEL_SLOT_BITS * \
	                ETUX_TIMER_HWHEEL_LEVELS_NR))

struct etux_timer_hwheel {
	unsigned int             count;
	int64_t                  tick;
	int64_t                  issue;
	struct stroll_dlist_node slots[ETUX_TIMER_HWHEEL_LEVELS_NR][ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL];
	struct stroll_dlist_node eternal;
};

static struct etux_timer_hwheel etux_timer_the_hwheel;

static __utils_nothrow __warn_result
int64_t
etux_timer_hwheel_tick(void)
{
	struct timespec now;

	utime_monotonic_now(&now);

	return etux_timer_tick_from_tspec_lower_clamp(&now);
}

static __utils_nonull(1, 2) __utils_nothrow
void
etux_timer_hwheel_enroll(struct etux_timer_hwheel * __restrict hwheel,
                         struct etux_timer * __restrict        timer)
{
	etux_timer_assert_intern(hwheel);
	etux_timer_assert_intern(timer);
	etux_timer_assert_intern(timer->expire);

	int64_t      tmout = stroll_max(timer->tick, hwheel->tick) -
	                     hwheel->tick;

	etux_timer_assert_intern(tmout >= 0);
	if (tmout < ETUX_TIMER_HWHEEL_TICKS_NR) {
		unsigned int lvl;
		unsigned int slot;

#define ETUX_TIMER_HWHEEL_TMOUT_MIN(_level) \
	(INT64_C(1) << ((_level) * ETUX_TIMER_HWHEEL_SLOT_BITS))

		for (lvl = 0; lvl < ETUX_TIMER_HWHEEL_LEVELS_NR; lvl++) {
			if (tmout < ETUX_TIMER_HWHEEL_TMOUT_MIN(lvl + 1))
				break;
		}

		etux_timer_assert_intern(lvl < ETUX_TIMER_HWHEEL_LEVELS_NR);
		slot = (timer->tick >> (lvl * ETUX_TIMER_HWHEEL_SLOT_BITS)) &
		       ETUX_TIMER_HWHEEL_SLOT_MASK;
		stroll_dlist_insert(&hwheel->slots[lvl][slot], &timer->node);
	}
	else
		etux_timer_insert_inorder(&hwheel->eternal, timer);
}

static __utils_nonull(1) __utils_nothrow
void
etux_timer_arm(struct etux_timer * __restrict timer)
{
	etux_timer_assert_timer_intern(timer);
	etux_timer_assert_intern(timer->expire);

	switch (timer->state) {
	case ETUX_TIMER_IDLE_STAT:
		etux_timer_the_hwheel.count++;
		break;

	case ETUX_TIMER_PEND_STAT:
		stroll_dlist_remove(&timer->node);
		break;

	case ETUX_TIMER_RUN_STAT:
		break;

	default:
		etux_timer_assert_intern(0);
	}

	timer->tick = etux_timer_tick_from_tspec_upper_clamp(&timer->tspec);

#warning TODO: set etux_timer_the_hwheel.issue if etux_timer_the_hwheel.count == 0 ?
	etux_timer_the_hwheel.issue = stroll_min(timer->tick,
	                                         etux_timer_the_hwheel.issue);

	etux_timer_hwheel_enroll(&etux_timer_the_hwheel, timer);

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

	if (!etux_timer_the_hwheel.count) {
		etux_timer_assert_api(timer->state != ETUX_TIMER_PEND_STAT);

		struct timespec now;

		utime_monotonic_now(&now);
		etux_timer_the_hwheel.tick =
			etux_timer_tick_from_tspec_lower_clamp(&now);
	}

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
	if (!etux_timer_the_hwheel.count) {
		etux_timer_assert_api(timer->state != ETUX_TIMER_PEND_STAT);

		etux_timer_the_hwheel.tick =
			etux_timer_tick_from_tspec_lower_clamp(&timer->tspec);
	}

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
	if (!etux_timer_the_hwheel.count) {
		etux_timer_assert_api(timer->state != ETUX_TIMER_PEND_STAT);

		etux_timer_the_hwheel.tick =
			etux_timer_tick_from_tspec_lower_clamp(&timer->tspec);
	}

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
		etux_timer_dismiss(timer);

		if (timer->tick == etux_timer_the_hwheel.issue)
			etux_timer_the_hwheel.issue =
				etux_timer_the_hwheel.tick;

		if (!--etux_timer_the_hwheel.count)
			etux_timer_the_hwheel.tick = etux_timer_hwheel_tick();
	}

	etux_timer_cancel_trace_exit(timer);
}

static __utils_nonull(1, 2) __utils_nothrow
void
etux_timer_hwheel_cascade_timers(struct etux_timer_hwheel * __restrict hwheel,
                                 struct stroll_dlist_node * __restrict timers)
{
#warning TODO: move list to local head and use foreach safe to enroll without individual removal ?
	while (!stroll_dlist_empty(timers)) {
		struct etux_timer * tmr = etux_timer_lead_timer(timers);

		stroll_dlist_remove(&tmr->node);
		etux_timer_hwheel_enroll(hwheel, tmr);
	}
}

static __utils_nonull(1) __utils_nothrow
void
etux_timer_hwheel_cascade(struct etux_timer_hwheel * __restrict hwheel)
{
	int64_t      idx;
	unsigned int lvl;

	for (idx = hwheel->tick >> ETUX_TIMER_HWHEEL_SLOT_BITS, lvl = 1;
	     lvl < ETUX_TIMER_HWHEEL_LEVELS_NR;
	     idx >>= ETUX_TIMER_HWHEEL_SLOT_BITS, lvl++) {
		int slot = idx & ETUX_TIMER_HWHEEL_SLOT_MASK;

		etux_timer_hwheel_cascade_timers(hwheel,
		                                 &hwheel->slots[lvl][slot]);

		/* TODO: unlikely ? */
		if (slot)
			return;
	}

	/*
	 * TODO: optimise the case of eternal cascading, i.e., stop when delay
	 * to timer->tick >= ETUX_TIMER_HWHEEL_TICKS_NR ?
	 */
	etux_timer_assert_intern(lvl == ETUX_TIMER_HWHEEL_LEVELS_NR);
	etux_timer_hwheel_cascade_timers(hwheel, &hwheel->eternal);
}

static __utils_nonull(1) __utils_nothrow
bool
etux_timer_hwheel_early_expiry(
	const struct etux_timer_hwheel * __restrict hwheel,
	unsigned int                                start,
	unsigned int                                stop,
	int64_t * __restrict                        issue)
{
	etux_timer_assert_intern(hwheel);
	etux_timer_assert_intern(start < ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL);
	etux_timer_assert_intern(stop <= ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL);
	etux_timer_assert_intern(start < stop);
	etux_timer_assert_intern(issue);

	const struct stroll_dlist_node * nodes = hwheel->slots[0];
	unsigned int                     slot;

	for (slot = start; slot < stop; slot++) {
		if (!stroll_dlist_empty(&nodes[slot])) {
			*issue = etux_timer_lead_timer(&nodes[slot])->tick;
			return true;
		}
	}

	return false;
}

static __utils_nonull(1) __utils_nothrow
bool
etux_timer_hwheel_late_expiry(
	const struct etux_timer_hwheel * __restrict hwheel,
	unsigned int                                level,
	unsigned int                                start,
	unsigned int                                stop,
	int64_t * __restrict                        issue)
{
	etux_timer_assert_intern(hwheel);
	etux_timer_assert_intern(level > 0);
	etux_timer_assert_intern(level < ETUX_TIMER_HWHEEL_LEVELS_NR);
	etux_timer_assert_intern(start < ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL);
	etux_timer_assert_intern(stop <= ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL);
	etux_timer_assert_intern(start < stop);
	etux_timer_assert_intern(issue);
	etux_timer_assert_intern(*issue >= 0);
	etux_timer_assert_intern(*issue <= ETUX_TIMER_TICK_MAX);

	const struct stroll_dlist_node * nodes = hwheel->slots[level];
	unsigned int                     slot;
	int64_t                          expiry = *issue;

	for (slot = start; slot < stop; slot++) {
		const struct etux_timer * tmr;

		stroll_dlist_foreach_entry(&nodes[slot], tmr, node)
			expiry = stroll_min(tmr->tick, expiry);

		if (expiry != *issue) {
			*issue = expiry;
			return true;
		}
	}

	return false;
}

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
etux_timer_hwheel_find_issue(struct etux_timer_hwheel * __restrict hwheel)
{
	unsigned int              slot = hwheel->tick &
	                                 ETUX_TIMER_HWHEEL_SLOT_MASK;
	int64_t                   issue = ETUX_TIMER_TICK_MAX;
	int64_t                   tick = hwheel->tick;
	bool                      found = false;
	unsigned int              lvl;

	found = etux_timer_hwheel_early_expiry(
		hwheel, slot, ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL, &issue);
	if (slot) {
		if (found)
			return issue;
		etux_timer_hwheel_early_expiry(hwheel, 0, slot, &issue);
		tick += ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL - slot;
	}

	for (tick >>= ETUX_TIMER_HWHEEL_SLOT_BITS, lvl = 1;
	     lvl < ETUX_TIMER_HWHEEL_LEVELS_NR;
	     tick >>= ETUX_TIMER_HWHEEL_SLOT_BITS, lvl++) {
		slot = tick & ETUX_TIMER_HWHEEL_SLOT_MASK;
		found = etux_timer_hwheel_late_expiry(
			hwheel,
			lvl,
			slot,
			ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL,
			&issue);
		if (slot) {
			if (found)
				return issue;
			etux_timer_hwheel_late_expiry(hwheel,
			                              lvl,
			                              0,
			                              slot,
			                              &issue);
			tick += ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL - slot;
		}
	}

	if (!stroll_dlist_empty(&hwheel->eternal)) {
		tick = etux_timer_lead_timer(&hwheel->eternal)->tick;
		issue = stroll_min(tick, issue);
	}

	return issue;
}

int64_t
etux_timer_issue_tick(void)
{
	if (!etux_timer_the_hwheel.count)
		return (int64_t)-ENOENT;

	if (etux_timer_the_hwheel.issue <= etux_timer_the_hwheel.tick)
		etux_timer_the_hwheel.issue =
			etux_timer_hwheel_find_issue(&etux_timer_the_hwheel);

	return etux_timer_the_hwheel.issue;
}

void
etux_timer_run(void)
{
	struct timespec now;
	int64_t         tick;

	etux_timer_run_trace_enter();

	utime_monotonic_now(&now);
	tick = etux_timer_tick_from_tspec_lower_clamp(&now);
	while (tick >= etux_timer_the_hwheel.tick) {
		unsigned int               slot;
		struct stroll_dlist_node * expired;

		if (!etux_timer_the_hwheel.count) {
			etux_timer_the_hwheel.tick = tick;
			goto out;
		}

		slot = etux_timer_the_hwheel.tick & ETUX_TIMER_HWHEEL_SLOT_MASK;
		expired = &etux_timer_the_hwheel.slots[0][slot];

		/* TODO: unlikely ? */
		if (!slot)
			etux_timer_hwheel_cascade(&etux_timer_the_hwheel);

		etux_timer_the_hwheel.tick++;

		while (!stroll_dlist_empty(expired)) {
			struct etux_timer * tmr =
				etux_timer_lead_timer(expired);

			etux_timer_assert_intern(tick >= tmr->tick);

			tmr->state = ETUX_TIMER_RUN_STAT;
			stroll_dlist_remove(&tmr->node);

			etux_timer_expire_trace_enter(tmr, &now, tick);
			tmr->expire(tmr);
			etux_timer_expire_trace_exit(tmr);

			if (tmr->state == ETUX_TIMER_RUN_STAT) {
				tmr->state = ETUX_TIMER_IDLE_STAT;
				etux_timer_the_hwheel.count--;
			}
		}

		utime_monotonic_now(&now);
		tick = etux_timer_tick_from_tspec_lower_clamp(&now);
	}

out:
	etux_timer_run_trace_exit();
}

/* TODO: expose timer lib init API ?? */
static __ctor() __utils_nothrow
void
etux_timer_ctor(void)
{
	unsigned int lvl;

	for (lvl = 0; lvl < ETUX_TIMER_HWHEEL_LEVELS_NR; lvl++) {
		unsigned int slot;

		for (slot = 0; slot < ETUX_TIMER_HWHEEL_SLOTS_PER_WHEEL; slot++)
			stroll_dlist_init(
				&etux_timer_the_hwheel.slots[lvl][slot]);
	}

	stroll_dlist_init(&etux_timer_the_hwheel.eternal);

	etux_timer_the_hwheel.count = 0;
	etux_timer_the_hwheel.tick = etux_timer_hwheel_tick();
	etux_timer_the_hwheel.issue = etux_timer_the_hwheel.tick;
}
