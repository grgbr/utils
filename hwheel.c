/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "timer.h"
#include <inttypes.h>
#include <values.h>

#define UTIMER_HWHEEL_SLOT_BITS \
	(6U)

#define UTIMER_HWHEEL_SLOT_MASK \
	((UINT64_C(1) << UTIMER_HWHEEL_SLOT_BITS) - 1)

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

/* TODO: cache align */
struct utimer_hwheel {
	unsigned int             count;
	uint64_t                 tick;
	uint64_t                 issue;
	struct stroll_dlist_node slots[UTIMER_HWHEEL_LEVELS_NR][UTIMER_HWHEEL_SLOTS_PER_WHEEL];
	struct stroll_dlist_node eternal;
};

static struct utimer_hwheel utimer_the_hwheel;

#if 0
/*
 * Warning: Keep these definitions just in case we need them in the futur...
 */

#define UTIMER_HWHEEL_TICK_MAX \
	(1UL << (UTIMER_HWHEEL_SLOT_BITS * UTIMER_HWHEEL_LEVELS_NR))

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

static
void
utimer_hwheel_enroll(struct utimer_hwheel * __restrict hwheel,
                     struct utimer * __restrict        timer)
{
	utimer_assert_intern(hwheel);
	utimer_assert_intern(timer);
	utimer_assert_intern(timer->tick);
	utimer_assert_intern(timer->expire);

	/* TODO: assert tmout <= maximum value ?! */
	uint64_t     tmout;
	unsigned int lvl;
	unsigned int bits;

	tmout = stroll_max(timer->tick, hwheel->tick) - hwheel->tick;

	for (lvl = 0, bits = 0;
	     lvl < UTIMER_HWHEEL_LEVELS_NR;
	     lvl++, bits += UTIMER_HWHEEL_SLOT_BITS) {
		if (!(tmout >> (bits + UTIMER_HWHEEL_SLOT_BITS)))
			break;
	}

	/* TODO: stroll_likely() ?? */
	if (lvl != UTIMER_HWHEEL_LEVELS_NR) {
		unsigned int slot = (tmout >> bits) & UTIMER_HWHEEL_SLOT_MASK;

		stroll_dlist_insert(&hwheel->slots[lvl][slot], &timer->node);
	}
	else
		stroll_dlist_insert_inorder_back(&hwheel->eternal,
		                                 &timer->node,
		                                 utimer_tick_cmp,
		                                 NULL);
}

static
void
utimer_hwheel_arm_tspec(struct utimer_hwheel * __restrict  hwheel,
                        struct utimer * __restrict         timer,
                        const struct timespec * __restrict tspec)
{
	utimer_assert_intern(timer);
	utimer_assert_intern(timer->expire);
	utimer_assert_intern(tspec);

	if (!hwheel->count++)
		hwheel->tick = utimer_tick();

	timer->tick = utimer_tick_from_tspec_upper(tspec);
	hwheel->issue = stroll_min(timer->tick, hwheel->issue);
	utimer_hwheel_enroll(hwheel, timer);
}

void
utimer_arm_tspec(struct utimer * __restrict         timer,
                 const struct timespec * __restrict tspec)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(tspec);

	utimer_hwheel_arm_tspec(&utimer_the_hwheel, timer, tspec);
}

static
void
utimer_hwheel_arm_msec(struct utimer_hwheel * __restrict  hwheel,
                       struct utimer * __restrict         timer,
                       unsigned long                      msec)
{
	utimer_assert_intern(hwheel);
	utimer_assert_intern(timer);
	utimer_assert_intern(timer->expire);
	utimer_assert_intern(msec);

	struct timespec date;

	utime_monotonic_now(&date);

	if (!hwheel->count++)
		hwheel->tick = utimer_tick_from_tspec_lower(&date);

	utime_tspec_add_msec(&date, msec);
	timer->tick = utimer_tick_from_tspec_upper(&date);
	hwheel->issue = stroll_min(timer->tick, hwheel->issue);
	utimer_hwheel_enroll(hwheel, timer);
}

void
utimer_arm_msec(struct utimer * __restrict timer, unsigned long msec)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(msec);

	utimer_hwheel_arm_msec(&utimer_the_hwheel, timer, msec);
}

static
void
utimer_hwheel_arm_sec(struct utimer_hwheel * __restrict  hwheel,
                      struct utimer * __restrict         timer,
                      unsigned long                      sec)
{
	utimer_assert_intern(hwheel);
	utimer_assert_intern(timer);
	utimer_assert_intern(timer->expire);
	utimer_assert_intern(sec);

	struct timespec date;

	utime_monotonic_now(&date);

	if (!hwheel->count++)
		hwheel->tick = utimer_tick_from_tspec_lower(&date);

	utime_tspec_add_sec(&date, sec);
	timer->tick = utimer_tick_from_tspec_upper(&date);
	hwheel->issue = stroll_min(timer->tick, hwheel->issue);
	utimer_hwheel_enroll(hwheel, timer);
}

void
utimer_arm_sec(struct utimer * __restrict timer, unsigned long sec)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(sec);

	utimer_hwheel_arm_sec(&utimer_the_hwheel, timer, sec);
}

static
void
utimer_hwheel_cancel(struct utimer_hwheel * __restrict  hwheel,
                     struct utimer * __restrict         timer)
{
	utimer_assert_intern(hwheel);
	utimer_assert_intern(timer);
	utimer_assert_intern(timer->expire);

	if (!stroll_dlist_empty(&timer->node)) {
		stroll_dlist_remove_init(&timer->node);

		if (timer->tick == hwheel->issue)
			hwheel->issue = hwheel->tick;

		if (!--hwheel->count)
			hwheel->tick = utimer_tick();
	}
}

void
utimer_cancel(struct utimer * __restrict timer)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);

	utimer_hwheel_cancel(&utimer_the_hwheel, timer);
}

static
void
utimer_hwheel_cascade_timers(struct utimer_hwheel *     hwheel,
                             struct stroll_dlist_node * timers)
{
	while (!stroll_dlist_empty(timers)) {
		struct utimer * timer;

		timer = stroll_dlist_entry(stroll_dlist_next(timers),
		                           struct utimer,
		                           node);
		utimer_assert_intern(timer);
		utimer_assert_intern(timer->tick);
		utimer_assert_intern(timer->expire);

		stroll_dlist_remove(&timer->node);
		utimer_hwheel_enroll(hwheel, timer);
	}
}

static
void
utimer_hwheel_cascade(struct utimer_hwheel * hwheel)
{
	uint64_t     idx;
	unsigned int lvl;

	for (idx = hwheel->tick >> UTIMER_HWHEEL_SLOT_BITS, lvl = 1;
	     lvl < UTIMER_HWHEEL_LEVELS_NR;
	     idx >>= UTIMER_HWHEEL_SLOT_BITS, lvl++) {
		unsigned int slot = idx & UTIMER_HWHEEL_SLOT_MASK;

		utimer_hwheel_cascade_timers(hwheel, &hwheel->slots[lvl][slot]);

		/* TODO: unlikely ? */
		if (slot)
			return;
	}

	utimer_assert_intern(lvl == UTIMER_HWHEEL_LEVELS_NR);
	utimer_hwheel_cascade_timers(hwheel, &hwheel->eternal);
}

static
uint64_t
utimer_hwheel_issue_tick(struct utimer_hwheel * hwheel)
{
	if (!hwheel->count)
		return 0;

	if (hwheel->issue <= hwheel->tick) {
		IMPLEMENT ME !: see __next_timer_interrupt() 
	}

	return hwheel->issue;
}

uint64_t
utimer_issue_tick(void)
{
	return utimer_hwheel_issue_tick(&utimer_the_hwheel);
}

static
void
utimer_hwheel_run(struct utimer_hwheel * hwheel)
{
	uint64_t tick = utimer_tick();

	while (tick > hwheel->tick) {
		unsigned int               slot;
		struct stroll_dlist_node * expired;

		if (!hwheel->count) {
			hwheel->tick = tick;
			return;
		}

		slot = hwheel->tick & UTIMER_HWHEEL_SLOT_MASK;
		expired = &hwheel->slots[0][slot];

		/* TODO: unlikely ? */
		if (!slot)
			utimer_hwheel_cascade(hwheel);

		hwheel->tick++;

		while (!stroll_dlist_empty(expired)) {
			struct utimer * timer;

			timer = stroll_dlist_entry(stroll_dlist_next(expired),
			                           struct utimer,
			                           node);
			utimer_assert_intern(timer);
			utimer_assert_intern(timer->tick);
			utimer_assert_intern(timer->expire);
			utimer_assert_intern(tick >= timer->tick);

			stroll_dlist_remove_init(&timer->node);
			timer->expire(timer);

			hwheel->count--;
		}

		tick = utimer_tick();
	}
}

void
utimer_run(void)
{
	utimer_hwheel_run(&utimer_the_hwheel);
}

static __utils_nonull(1) __utils_nothrow
void
utimer_hwheel_init(struct utimer_hwheel * __restrict hwheel)
{
	unsigned int lvl;

	for (lvl = 0; lvl < UTIMER_HWHEEL_LEVELS_NR; lvl++) {
		unsigned int slot;

		for (slot = 0; slot < UTIMER_HWHEEL_SLOTS_PER_WHEEL; slot++)
			stroll_dlist_init(&hwheel->slots[lvl][slot]);
	}

	stroll_dlist_init(&hwheel->eternal);

	hwheel->count = 0;
	hwheel->tick = utimer_tick();
	hwheel->issue = hwheel->tick;
}
