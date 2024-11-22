/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "timer.h"
#include <inttypes.h>
#include <values.h>
#include <errno.h>

#define UTIMER_HWHEEL_SLOT_BITS \
	(6U)

#define UTIMER_HWHEEL_SLOT_MASK \
	((INT64_C(1) << UTIMER_HWHEEL_SLOT_BITS) - 1)

#define UTIMER_HWHEEL_SLOTS_PER_WHEEL \
	(1U << UTIMER_HWHEEL_SLOT_BITS)

/*
 * Setup maximum depth of hierarchical timing wheel.
 *
 * The table below gives time ranges that the hwheel may handle according to
 * to allowed UTIMER_TICK_SUBSEC_BITS values:
 *                                                  Tick                Time
 *                                Time period  frequency  Hierarchy    range
 *    UTIMER_TICK_SUBSEC_BITS  (milliseconds)    (Hertz)     levels   (days)
 *                         0     1000.000000          1          4     >194
 *                         1      500.000000          2          4      >97
 *                         2      250.000000          4          4      >48
 *                         3      125.000000          8          4      >24
 *                         4       62.500000         16          4      >12
 *                         5       31.250000         32          5     >388
 *                         6       15.625000         64          5     >194
 *                         7        7.812500        128          5      >97
 *                         8        3.906250        256          5      >48
 *                         9        1.953125        512          5      >24
 */
#if UTIMER_TICK_SUBSEC_BITS < 5
#define UTIMER_HWHEEL_LEVELS_NR (4U)
#else  /* !(UTIMER_TICK_SUBSEC_BITS < 5) */
#define UTIMER_HWHEEL_LEVELS_NR (5U)
#endif /* UTIMER_TICK_SUBSEC_BITS < 5 */

/*
 * Maximum number of ticks that the hierarchical timer wheel may handle (without
 * accounting for sorted eternal timer list).
 */
#define UTIMER_HWHEEL_TICKS_NR \
	(INT64_C(1) << (UTIMER_HWHEEL_SLOT_BITS * UTIMER_HWHEEL_LEVELS_NR))

struct utimer_hwheel {
	unsigned int             count;
	int64_t                  tick;
	int64_t                  issue;
	struct stroll_dlist_node slots[UTIMER_HWHEEL_LEVELS_NR][UTIMER_HWHEEL_SLOTS_PER_WHEEL];
	struct stroll_dlist_node eternal;
};

static struct utimer_hwheel utimer_the_hwheel;

#warning REMOVE ME if not needed...
#if 0
/*
 * Warning: Keep these definitions just in case we need them in the futur...
 */

#define UTIMER_HWHEEL_TICKS_PER_MIN \
	(60UL * UTIMER_TICKS_PER_SEC)

#define UTIMER_HWHEEL_TICKS_PER_HOUR \
	(60UL * UTIMER_HWHEEL_TICKS_PER_MIN)

#define UTIMER_HWHEEL_TICKS_PER_DAY \
	(24UL * UTIMER_HWHEEL_TICKS_PER_HOUR)

#define UTIMER_HWHEEL_DAY_MAX \
	(UTIMER_HWHEEL_TICK_MAX / UTIMER_HWHEEL_TICKS_PER_DAY)

#define UTIMER_HWHEEL_HOUR_MAX \
	((UTIMER_HWHEEL_TICK_MAX - \
	  (UTIMER_HWHEEL_DAY_MAX * UTIMER_HWHEEL_TICKS_PER_DAY)) / \
	  UTIMER_HWHEEL_TICKS_PER_HOUR)

#define UTIMER_HWHEEL_MIN_MAX \
	((UTIMER_HWHEEL_TICK_MAX - \
	  (UTIMER_HWHEEL_DAY_MAX * UTIMER_HWHEEL_TICKS_PER_DAY) - \
	  (UTIMER_HWHEEL_HOUR_MAX * UTIMER_HWHEEL_TICKS_PER_HOUR)) / \
	  UTIMER_HWHEEL_TICKS_PER_MIN)

#define UTIMER_HWHEEL_SEC_MAX \
	((UTIMER_HWHEEL_TICK_MAX - \
	  (UTIMER_HWHEEL_DAY_MAX * UTIMER_HWHEEL_TICKS_PER_DAY) - \
	  (UTIMER_HWHEEL_HOUR_MAX * UTIMER_HWHEEL_TICKS_PER_HOUR) - \
	  (UTIMER_HWHEEL_MIN_MAX * UTIMER_HWHEEL_TICKS_PER_MIN)) / \
	  UTIMER_TICKS_PER_SEC)
#endif

#define UTIMER_HWHEEL_TMOUT_MAX(_level) \
	(INT64_C(1) << ((_level) * UTIMER_HWHEEL_SLOT_BITS))

static __utils_nonull(1, 2) __utils_nothrow
void
utimer_hwheel_enroll(struct utimer_hwheel * __restrict hwheel,
                     struct utimer * __restrict        timer)
{
	utimer_assert_intern(hwheel);
	utimer_assert_intern(timer);
	utimer_assert_intern(timer->expire);

	int64_t      tmout = stroll_max(timer->tick, hwheel->tick) -
	                     hwheel->tick;
#warning TODO: select whatever implementation is the most performant.
#if 0
	unsigned int lvl;
	unsigned int slot;

	utimer_assert_intern(tmout >= 0);

#if (UTIMER_HWHEEL_LEVELS_NR < 4) || (UTIMER_HWHEEL_LEVELS_NR > 5)
#error "Hierarchical timing wheel maximum depth not supported !"
#endif
	switch (tmout) {
	case 0 ... UTIMER_HWHEEL_TMOUT_MAX(1) - 1:
		lvl = 0;
		break;

	case UTIMER_HWHEEL_TMOUT_MAX(1) ... UTIMER_HWHEEL_TMOUT_MAX(2) - 1:
		lvl = 1;
		tmout = timer->tick >> UTIMER_HWHEEL_SLOT_BITS;
		break;

	case UTIMER_HWHEEL_TMOUT_MAX(2) ... UTIMER_HWHEEL_TMOUT_MAX(3) - 1:
		lvl = 2;
		tmout = timer->tick >> (2 * UTIMER_HWHEEL_SLOT_BITS);
		break;

	case UTIMER_HWHEEL_TMOUT_MAX(3) ... UTIMER_HWHEEL_TMOUT_MAX(4) - 1:
		lvl = 3;
		tmout = timer->tick >> (3 * UTIMER_HWHEEL_SLOT_BITS);
		break;

#if UTIMER_HWHEEL_LEVELS_NR == 5
	case UTIMER_HWHEEL_TMOUT_MAX(4) ... UTIMER_HWHEEL_TMOUT_MAX(5) - 1:
		lvl = 4;
		tmout = timer->tick >> (4 * UTIMER_HWHEEL_SLOT_BITS);
		break;
#endif /* UTIMER_HWHEEL_LEVELS_NR == 5 */

	default:
		etux_timer_insert(&hwheel->eternal, timer);
		return;
	}

	slot = tmout & UTIMER_HWHEEL_SLOT_MASK;
	stroll_dlist_insert(&hwheel->slots[lvl][slot], &timer->node);
#else
	utimer_assert_intern(tmout >= 0);
	if (tmout < UTIMER_HWHEEL_TICKS_NR) {
		unsigned int lvl;
		unsigned int bits;
		unsigned int slot;

		for (lvl = 0, bits = 0;
		     lvl < UTIMER_HWHEEL_LEVELS_NR;
		     lvl++, bits += UTIMER_HWHEEL_SLOT_BITS) {
			if (!(tmout >> (bits + UTIMER_HWHEEL_SLOT_BITS)))
				break;
		}

		slot = (timer->tick >> bits) & UTIMER_HWHEEL_SLOT_MASK;
		stroll_dlist_insert(&hwheel->slots[lvl][slot], &timer->node);
	}
	else
		etux_timer_insert(&hwheel->eternal, timer);
#endif
}

static __utils_nonull(1, 2) __utils_nothrow
void
utimer_arm(struct utimer * __restrict timer)
{
	utimer_assert_timer_intern(timer);
	utimer_assert_intern(timer->expire);

	switch (timer->state) {
	case UTIMER_IDLE_STAT:
		utimer_the_hwheel.count++;
		break;

	case UTIMER_PEND_STAT:
		stroll_dlist_remove(&timer->node);
		break;

	case UTIMER_RUN_STAT:
		break;

	default:
		utimer_assert_intern(0);
	}

	timer->tick = utimer_tick_from_tspec_upper_clamp(&timer->tspec);

#warning TODO: set utimer_the_hwheel.issue if utimer_the_hwheel.count == 0 ?
	utimer_the_hwheel.issue = stroll_min(timer->tick,
	                                     utimer_the_hwheel.issue);

	utimer_hwheel_enroll(&utimer_the_hwheel, timer);

	timer->state = UTIMER_PEND_STAT;
}

void
utimer_arm_tspec(struct utimer * __restrict         timer,
                 const struct timespec * __restrict tspec)
{
	utimer_assert_timer_api(timer);
	utimer_assert_api(timer->expire);
	utime_assert_tspec_api(tspec);

	if (!utimer_the_hwheel.count) {
		utimer_assert_api(timer->state != UTIMER_PEND_STAT);

		struct timespec now;

		utime_monotonic_now(&now);
		utimer_the_hwheel.tick =
			utimer_tick_from_tspec_lower_clamp(&now);
	}

	timer->tspec = *tspec;

	utimer_arm(timer);
}

void
utimer_arm_msec(struct utimer * __restrict timer, int msec)
{
	utimer_assert_timer_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(msec >= 0);

	utime_monotonic_now(&timer->tspec);
	if (!utimer_the_hwheel.count) {
		utimer_assert_api(timer->state != UTIMER_PEND_STAT);

		utimer_the_hwheel.tick =
			utimer_tick_from_tspec_lower_clamp(&timer->tspec);
	}

	utime_tspec_add_msec_clamp(&timer->tspec, msec);

	utimer_arm(timer);
}

void
utimer_arm_sec(struct utimer * __restrict timer, int sec)
{
	utimer_assert_timer_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(sec >= 0);

	utime_monotonic_now(&timer->tspec);
	if (!utimer_the_hwheel.count) {
		utimer_assert_api(timer->state != UTIMER_PEND_STAT);

		utimer_the_hwheel.tick =
			utimer_tick_from_tspec_lower_clamp(&timer->tspec);
	}

	utime_tspec_add_sec_clamp(&timer->tspec, sec);

	utimer_arm(timer);
}

void
utimer_cancel(struct utimer * __restrict timer)
{
	utimer_assert_timer_api(timer);

	if (timer->state == UTIMER_PEND_STAT) {
		utimer_dismiss(timer);

		if (timer->tick == utimer_the_hwheel.issue)
			utimer_the_hwheel.issue = utimer_the_hwheel.tick;

		if (!--utimer_the_hwheel.count)
			utimer_the_hwheel.tick = utimer_tick();
	}
}

static __utils_nonull(1, 2) __utils_nothrow
void
utimer_hwheel_cascade_timers(struct utimer_hwheel * __restrict     hwheel,
                             struct stroll_dlist_node * __restrict timers)
{
#warning TODO: move list to local head and use foreach safe to enroll without individual removal ?
	while (!stroll_dlist_empty(timers)) {
		struct utimer * tmr = utimer_lead_timer(timers);

		stroll_dlist_remove(&tmr->node);
		utimer_hwheel_enroll(hwheel, tmr);
	}
}

static __utils_nonull(1) __utils_nothrow
void
utimer_hwheel_cascade(struct utimer_hwheel * __restrict hwheel)
{
	int64_t      idx;
	unsigned int lvl;

	for (idx = hwheel->tick >> UTIMER_HWHEEL_SLOT_BITS, lvl = 1;
	     lvl < UTIMER_HWHEEL_LEVELS_NR;
	     idx >>= UTIMER_HWHEEL_SLOT_BITS, lvl++) {
		int slot = idx & UTIMER_HWHEEL_SLOT_MASK;

		utimer_hwheel_cascade_timers(hwheel, &hwheel->slots[lvl][slot]);

		/* TODO: unlikely ? */
		if (slot)
			return;
	}

	/*
	 * TODO: optimise the case of eternal cascading, i.e., stop when delay
	 * to timer->tick >= UTIMER_HWHEEL_TICKS_NR ?
	 */
	utimer_assert_intern(lvl == UTIMER_HWHEEL_LEVELS_NR);
	utimer_hwheel_cascade_timers(hwheel, &hwheel->eternal);
}

static __utils_nonull(1) __utils_pure __utils_nothrow
struct utimer *
utimer_slot_lead_timer(
	const struct stroll_dlist_node nodes[__restrict_arr
	                                     UTIMER_HWHEEL_SLOTS_PER_WHEEL],
	unsigned int                   slot)
{
	utimer_assert_intern(nodes);
	utimer_assert_intern(slot < UTIMER_HWHEEL_SLOTS_PER_WHEEL);

	if (!stroll_dlist_empty(&nodes[slot]))
		return utimer_lead_timer(&nodes[slot]);
	else
		return NULL;
}

/* TODO: refactor me !! */
static __utils_nonull(1) __utils_nothrow
int64_t
utimer_hwheel_find_issue(struct utimer_hwheel * __restrict hwheel)
{
	unsigned int          slot = hwheel->tick & UTIMER_HWHEEL_SLOT_MASK;
	unsigned int          curr;
	const struct utimer * tmr;
	int64_t               tick = hwheel->tick;
	int64_t               issue = tick + UTIMER_HWHEEL_TICKS_NR - 1;
	bool                  found = false;
	unsigned int          lvl;

	for (curr = slot; curr < UTIMER_HWHEEL_SLOTS_PER_WHEEL; curr++) {
		tmr = utimer_slot_lead_timer(hwheel->slots[0], curr);
		if (tmr) {
			issue = tmr->tick;
			found = true;
			break;
		}
	}

	if (slot) {
		if (found)
			return issue;

		for (curr = 0; curr < slot; curr++) {
			tmr = utimer_slot_lead_timer(hwheel->slots[0], curr);
			if (tmr) {
				issue = tmr->tick;
				found = true;
				break;
			}
		}

		tick += UTIMER_HWHEEL_SLOTS_PER_WHEEL - slot;
	}

	for (tick >>= UTIMER_HWHEEL_SLOT_BITS, lvl = 1;
	     lvl < UTIMER_HWHEEL_LEVELS_NR;
	     tick >>= UTIMER_HWHEEL_SLOT_BITS, lvl++) {
		slot = tick & UTIMER_HWHEEL_SLOT_MASK;

		for (curr = slot; curr < UTIMER_HWHEEL_SLOTS_PER_WHEEL; curr++) {
			stroll_dlist_foreach_entry(&hwheel->slots[lvl][curr],
			                           tmr,
			                           node) {
				if (tmr->tick < issue) {
					issue = tmr->tick;
					found = true;
				}
			}

			if (found)
				break;
		}

		if (slot) {
			if (found)
				return issue;

			for (curr = 0; curr < slot; curr++) {
				stroll_dlist_foreach_entry(&hwheel->slots[lvl][curr],
				                           tmr,
				                           node) {
					if (tmr->tick < issue) {
						issue = tmr->tick;
						found = true;
					}
				}

				if (found)
					break;
			}

			tick += UTIMER_HWHEEL_SLOTS_PER_WHEEL - slot;
		}
	}

	tmr = utimer_lead_timer(&hwheel->eternal);
	if (tmr)
		issue = tmr->tick;

	return issue;
}

static __utils_pure __utils_nothrow __warn_result
int64_t
utimer_issue_tick(void)
{
	if (!utimer_the_hwheel.count)
		return (int64_t)-ENOENT;

	if (utimer_the_hwheel.issue <= utimer_the_hwheel.tick)
		utimer_the_hwheel.issue =
			utimer_hwheel_find_issue(&utimer_the_hwheel);

	return utimer_the_hwheel.issue;
}

void
utimer_run(void)
{
	int64_t tick = utimer_tick();

	while (tick >= utimer_the_hwheel.tick) {
		unsigned int               slot;
		struct stroll_dlist_node * expired;

		if (!utimer_the_hwheel.count) {
			utimer_the_hwheel.tick = tick;
			return;
		}

		slot = utimer_the_hwheel.tick & UTIMER_HWHEEL_SLOT_MASK;
		expired = &utimer_the_hwheel.slots[0][slot];

		/* TODO: unlikely ? */
		if (!slot)
			utimer_hwheel_cascade(&utimer_the_hwheel);

		utimer_the_hwheel.tick++;

		while (!stroll_dlist_empty(expired)) {
			struct utimer * tmr = utimer_lead_timer(expired);

			utimer_assert_intern(tick >= tmr->tick);

			tmr->state = UTIMER_RUN_STAT;
			stroll_dlist_remove(&tmr->node);

			tmr->expire(tmr);

			if (tmr->state == UTIMER_RUN_STAT) {
				tmr->state = UTIMER_IDLE_STAT;
				utimer_the_hwheel.count--;
			}
		}

		tick = utimer_tick();
	}
}

/* TODO: expose timer lib init API ?? */
static __ctor() __utils_nothrow
void
utimer_ctor(void)
{
	unsigned int lvl;

	for (lvl = 0; lvl < UTIMER_HWHEEL_LEVELS_NR; lvl++) {
		unsigned int slot;

		for (slot = 0; slot < UTIMER_HWHEEL_SLOTS_PER_WHEEL; slot++)
			stroll_dlist_init(&utimer_the_hwheel.slots[lvl][slot]);
	}

	stroll_dlist_init(&utimer_the_hwheel.eternal);

	utimer_the_hwheel.count = 0;
	utimer_the_hwheel.tick = utimer_tick();
	utimer_the_hwheel.issue = utimer_the_hwheel.tick;
}
