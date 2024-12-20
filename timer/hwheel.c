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

static __utils_nonull(1) __utils_nothrow __warn_result
int64_t
etux_timer_hwheel_load_tick(struct timespec * __restrict now)
{
	etux_timer_assert_intern(now);

	utime_monotonic_now(now);

	return etux_timer_tick_from_tspec_lower_clamp(now);
}

static __utils_nothrow __warn_result
int64_t
etux_timer_hwheel_tick(void)
{
	struct timespec now;

	return etux_timer_hwheel_load_tick(&now);
}

static __utils_nothrow
void
etux_timer_hwheel_refresh_tick(const struct timespec * __restrict now)
{
	if (now)
		etux_timer_the_hwheel.tick =
			etux_timer_tick_from_tspec_lower_clamp(now);
	else
		etux_timer_the_hwheel.tick = etux_timer_hwheel_tick();
}

static __utils_nonull(1, 2) __utils_nothrow
void
etux_timer_hwheel_enroll(struct etux_timer_hwheel * __restrict hwheel,
                         struct etux_timer * __restrict        timer)
{
	etux_timer_assert_intern(hwheel);
	etux_timer_assert_intern(timer);
	etux_timer_assert_intern(timer->expire);
	etux_timer_assert_intern(tmout < ETUX_TIMER_HWHEEL_TICKS_NR);

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
etux_timer_hwheel_insert(struct etux_timer * __restrict     timer,
                         const struct timespec * __restrict now)
{
	etux_timer_assert_timer_intern(timer);
	etux_timer_assert_intern(timer->expire);
	etux_timer_assert_intern(timer->state == ETUX_TIMER_IDLE_STAT);

	int64_t tick;

	tick = etux_timer_tick_from_tspec_upper_clamp(&timer->tspec);

	if (!etux_timer_the_hwheel.count++)
		etux_timer_hwheel_refresh_tick(now);
	etux_timer_the_hwheel.issue = stroll_min(tick,
	                                         etux_timer_the_hwheel.issue);

	timer->state = ETUX_TIMER_PEND_STAT;
	timer->tick = tick;
	etux_timer_hwheel_enroll(&etux_timer_the_hwheel, timer);
}

static __utils_nonull(1) __utils_nothrow
void
etux_timer_hwheel_adjust(struct etux_timer * __restrict     timer,
                         const struct timespec * __restrict now)
{
	etux_timer_assert_timer_intern(timer);
	etux_timer_assert_intern(timer->expire);
	etux_timer_assert_intern(timer->state == ETUX_TIMER_PEND_STAT);
	etux_timer_assert_intern(etux_timer_the_hwheel.count);

	int64_t tick;

	tick = etux_timer_tick_from_tspec_upper_clamp(&timer->tspec);
	if (timer->tick == tick)
		return;

	stroll_dlist_remove(&timer->node);
	if (timer->tick == etux_timer_the_hwheel.issue)
		etux_timer_the_hwheel.issue = etux_timer_the_hwheel.tick;
	etux_timer_the_hwheel.issue = stroll_min(tick,
	                                         etux_timer_the_hwheel.issue);
	if (etux_timer_the_hwheel.count == 1)
		etux_timer_hwheel_refresh_tick(now);

	timer->tick = tick;
	etux_timer_hwheel_enroll(&etux_timer_the_hwheel, timer);
}

static __utils_nonull(1) __utils_nothrow
void
etux_timer_hwheel_resched(struct etux_timer * __restrict timer)
{
	etux_timer_assert_timer_intern(timer);
	etux_timer_assert_intern(timer->expire);
	etux_timer_assert_intern(timer->state == ETUX_TIMER_RUN_STAT);
	etux_timer_assert_intern(etux_timer_the_hwheel.count);
	etux_timer_assert_intern(etux_timer_the_hwheel.issue <=
	                         etux_timer_the_hwheel.tick);

	timer->state = ETUX_TIMER_PEND_STAT;
	timer->tick = etux_timer_tick_from_tspec_upper_clamp(&timer->tspec);
	etux_timer_hwheel_enroll(&etux_timer_the_hwheel, timer);
}

static __utils_nonull(1) __utils_nothrow
void
etux_timer_hwheel_arm(struct etux_timer * __restrict     timer,
                      const struct timespec * __restrict now)
{
	etux_timer_assert_timer_intern(timer);
	etux_timer_assert_intern(timer->expire);

	switch (timer->state) {
	case ETUX_TIMER_IDLE_STAT:
		etux_timer_hwheel_insert(timer, now);
		break;

	case ETUX_TIMER_PEND_STAT:
		etux_timer_hwheel_adjust(timer, now);
		break;

	case ETUX_TIMER_RUN_STAT:
		etux_timer_hwheel_resched(timer);
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
	etux_timer_hwheel_arm(timer, NULL);

	etux_timer_arm_tspec_trace_exit(timer);
}

void
etux_timer_arm_msec(struct etux_timer * __restrict timer, int msec)
{
	etux_timer_assert_timer_api(timer);
	etux_timer_assert_api(timer->expire);
	etux_timer_assert_api(msec >= 0);

	struct timespec now;

	etux_timer_arm_msec_trace_enter(timer, msec);

	utime_monotonic_now(&now);

	timer->tspec = now;
	utime_tspec_add_msec_clamp(&timer->tspec, msec);
	etux_timer_hwheel_arm(timer, &now);

	etux_timer_arm_msec_trace_exit(timer);
}

void
etux_timer_arm_sec(struct etux_timer * __restrict timer, int sec)
{
	etux_timer_assert_timer_api(timer);
	etux_timer_assert_api(timer->expire);
	etux_timer_assert_api(sec >= 0);

	struct timespec now;

	etux_timer_arm_sec_trace_enter(timer, sec);

	utime_monotonic_now(&now);

	timer->tspec = now;
	utime_tspec_add_sec_clamp(&timer->tspec, sec);
	etux_timer_hwheel_arm(timer, &now);

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
	if (!stroll_dlist_empty(timers)) {
		struct stroll_dlist_node head = STROLL_DLIST_INIT(head);
		struct etux_timer *      tmr;
		struct etux_timer *      tmp;

		stroll_dlist_splice_after(&head,
		                          stroll_dlist_next(timers),
		                          stroll_dlist_prev(timers));
		stroll_dlist_foreach_entry_safe(&head, tmr, node, tmp)
			etux_timer_hwheel_enroll(hwheel, tmr);
	}
}

static __utils_nonull(1) __utils_nothrow
void
etux_timer_hwheel_cascade(struct etux_timer_hwheel * __restrict hwheel)
{
	unsigned int lvl;
	int64_t      idx;

	for (idx = hwheel->tick >> ETUX_TIMER_HWHEEL_SLOT_BITS, lvl = 1;
	     lvl < ETUX_TIMER_HWHEEL_LEVELS_NR;
	     idx >>= ETUX_TIMER_HWHEEL_SLOT_BITS, lvl++) {
		int slot = idx & ETUX_TIMER_HWHEEL_SLOT_MASK;

		etux_timer_hwheel_cascade_timers(hwheel,
		                                 &hwheel->slots[lvl][slot]);

		if (slot)
			return;
	}

#if 0
#warning TODO: optimise the case of eternal cascading, i.e., stop when delay to timer->tick >= ETUX_TIMER_HWHEEL_TICKS_NR ?
	etux_timer_assert_intern(lvl == ETUX_TIMER_HWHEEL_LEVELS_NR);
	etux_timer_hwheel_cascade_timers(hwheel, &hwheel->eternal);
#else
	etux_timer_assert_intern(lvl == ETUX_TIMER_HWHEEL_LEVELS_NR);
	while (!stroll_dlist_empty(&hwheel->eternal)) {
		struct etux_timer * tmr;
		int64_t             tmout;

		tmr = etux_timer_lead_timer(&hwheel->eternal);
		tmout = stroll_max(tmr->tick, hwheel->tick) - hwheel->tick;

		etux_timer_assert_intern(tmout >= 0);
		if (tmout >= ETUX_TIMER_HWHEEL_TICKS_NR)
			break;

		stroll_dlist_remove(&tmr->node);
		etux_timer_hwheel_enroll(&etux_timer_the_hwheel, tmr);
	}
#endif
}

/*
 * Find next timer expiry date within first level timing wheel.
 * Return value:
 *   -1: no timer found ;
 *    0: timer has been found but tell the caller that further searching within
 *       level 1 timing wheel is required ;
 *    1: timer has been found and search is over.
 */
static __utils_nonull(1) __utils_nothrow
int
etux_timer_hwheel_early_expiry(
	const struct stroll_dlist_node * __restrict nodes,
	int64_t                                     tick,
	int64_t * __restrict                        issue)
{
	etux_timer_assert_intern(nodes);
	etux_timer_assert_intern(tick <= ETUX_TIMER_TICK_MAX);
	etux_timer_assert_intern(issue);

	unsigned int start = tick & ETUX_TIMER_HWHEEL_SLOT_MASK;
	unsigned int slot = start;

	do {
		if (!stroll_dlist_empty(&nodes[slot])) {
			*issue = etux_timer_lead_timer(&nodes[slot])->tick;

			/* Tell caller wether cascasding is needed or not. */
			return (int)(start && (slot >= start));
		}

		slot = (slot + 1) & ETUX_TIMER_HWHEEL_SLOT_MASK;
	} while (slot != start);

	/* Tell caller that no timer has been found. */
	return -1;
}

/*
 * Find next timer expiry date within timing wheel slots given as `nodes'.
 * Return value:
 *   -1: no timer found ;
 *    0: timer has been found but tell the caller that further searching within
 *       level 1 timing wheel is required ;
 *    1: timer has been found and search is over.
 */
static __utils_nonull(1) __utils_nothrow
int
etux_timer_hwheel_cascade_expiry(
	const struct stroll_dlist_node * __restrict nodes,
	int                                         missing,
	int64_t                                     tick,
	int64_t * __restrict                        issue)
{
	etux_timer_assert_intern(nodes);
	etux_timer_assert_intern(tick <= ETUX_TIMER_TICK_MAX);
	etux_timer_assert_intern(missing <= 0);
	etux_timer_assert_intern(issue);
	etux_timer_assert_intern(*issue >= 0);
	etux_timer_assert_intern(*issue <= ETUX_TIMER_TICK_MAX);

	unsigned int start = tick & ETUX_TIMER_HWHEEL_SLOT_MASK;
	unsigned int slot = start;
	int64_t      expiry = *issue;

	do {
		const struct etux_timer * tmr;

		stroll_dlist_foreach_entry(&nodes[slot], tmr, node) {
			expiry = stroll_min(tmr->tick, expiry);
			missing = 0;
		}

		if (!missing)
			break;

		slot = (slot + 1) & ETUX_TIMER_HWHEEL_SLOT_MASK;
	} while (slot != start);

	if (!missing) {
		*issue = expiry;
		return (int)(start && (slot >= start));
	}

	return -1;
}

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
etux_timer_hwheel_find_issue(struct etux_timer_hwheel * __restrict hwheel)
{
	etux_timer_assert_intern(hwheel);

	int64_t      tick = hwheel->tick;
	int64_t      issue = ETUX_TIMER_TICK_MAX;
	int          res;
	unsigned int lvl;

	res = etux_timer_hwheel_early_expiry(hwheel->slots[0], tick, &issue);
	if (res > 0)
		/* Next expiry found and no further cascading needed. */
		return issue;

	for (lvl = 1; lvl < ETUX_TIMER_HWHEEL_LEVELS_NR; lvl++) {
		tick = (tick + ETUX_TIMER_HWHEEL_SLOT_MASK) >>
		       ETUX_TIMER_HWHEEL_SLOT_BITS;
		res = etux_timer_hwheel_cascade_expiry(hwheel->slots[lvl],
		                                       res,
		                                       tick,
		                                       &issue);
		if (res > 0)
			/* Next expiry found and no further cascading needed. */
			return issue;
	}

	if (!stroll_dlist_empty(&hwheel->eternal)) {
		tick = etux_timer_lead_timer(&hwheel->eternal)->tick;
		issue = stroll_min(tick, issue);
	}

	etux_timer_assert_intern(issue >= 0);
	etux_timer_assert_intern(issue <= ETUX_TIMER_TICK_MAX);

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

	tick = etux_timer_hwheel_load_tick(&now);
	while (tick >= etux_timer_the_hwheel.tick) {
		unsigned int               slot;
		struct stroll_dlist_node * expired;

		if (!etux_timer_the_hwheel.count) {
			etux_timer_the_hwheel.tick = tick;
			goto out;
		}

		slot = etux_timer_the_hwheel.tick & ETUX_TIMER_HWHEEL_SLOT_MASK;
		expired = &etux_timer_the_hwheel.slots[0][slot];

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

		tick = etux_timer_hwheel_load_tick(&now);
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
