/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/timer.h"

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define utimer_assert_intern(_expr) \
	stroll_assert("utils:utimer", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define utimer_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

static inline __utils_nonull(1) __utils_pure __utils_nothrow __returns_nonull
struct utimer *
utimer_timer_from_node(const struct stroll_dlist_node * __restrict node)
{
	utimer_assert_intern(node);

	return stroll_dlist_entry(node, struct utimer, node);
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __returns_nonull
struct utimer *
utimer_lead_timer(const struct stroll_dlist_node * __restrict head)
{
	utimer_assert_intern(head);
	utimer_assert_intern(!stroll_dlist_empty(head));

	return stroll_dlist_entry(stroll_dlist_next(head), struct utimer, node);
}

/******************************************************************************
 * Tick handling
 ******************************************************************************/

#if 0
/*
 * Warning: Keep these definitions just in case we need them in the futur...
 *
 * Bits used to encode seconds portion of timer ticks.
 *
 * Number of bits used to encode the seconds portion of a timer tick. It is
 * derived as the number of available bits (minus 1 because of time_t sign bit)
 * used to encode the @p tv_sec field of a @p struct timespec minus
 * UTIMER_TICK_SUBSEC_BITS.
 *
 * On 32-bits architectures, ticks may encode time ranges larger than 68 years
 * and even more onto 64-bits platforms (multiple centuries)...
 */
#define UTIMER_TICK_SUPSEC_BITS \
	STROLL_CONST_MIN((sizeof(uint64_t) * CHAR_BIT) - \
	                 UTIMER_TICK_SUBSEC_BITS, \
	                 (sizeof_member(struct timespec, tv_sec) * CHAR_BIT) - \
	                 1)

#define UTIMER_TICK_SUPSEC_MASK \
	((1UL << UTIMER_TICK_SUPSEC_BITS) - 1)
#endif

static int
utimer_tick_cmp(const struct stroll_dlist_node * __restrict first,
                const struct stroll_dlist_node * __restrict second,
                void *                                      data __unused)
{
	utimer_assert_intern(first);
	utimer_assert_intern(second);

	const struct utimer * fst = utimer_timer_from_node(first);
	const struct utimer * snd = utimer_timer_from_node(second);

	return (fst->tick > snd->tick) - (fst->tick < snd->tick);
}

uint64_t
utimer_tick(void)
{
	struct timespec now;

	utime_monotonic_now(&now);
	
	return utimer_tick_from_tspec_lower(&now);
}

/******************************************************************************
 * Timer generic handling
 ******************************************************************************/

struct timespec *
utimer_issue_tspec(struct timespec * __restrict tspec)
{
	uint64_t tick;

	tick = utimer_issue_tick();
	if (tick) {
		utimer_tspec_from_tick(tick, tspec);

		return tspec;
	}
	else
		return NULL;
}

long
utimer_issue_msec(void)
{
	uint64_t issue;

	issue = utimer_issue_tick();
	if (issue) {
		uint64_t tick = utimer_tick();

		tick = stroll_max(issue, tick) - tick;
		tick = stroll_min(tick, UTIMER_TICK_MSEC_MAX);

		return utimer_msec_from_tick_lower(tick);
	}
	else
		return -1;
}

/******************************************************************************
 * Timer list handling
 ******************************************************************************/

#if defined(CONFIG_UTILS_TIMER_LIST)

static struct stroll_dlist_node utimer_the_list =
	STROLL_DLIST_INIT(utimer_the_list);

static __utils_nonull(1) __utils_nothrow
void
utimer_arm(struct utimer * __restrict timer)
{
	utimer_assert_intern(timer);
	utimer_assert_intern(timer->tick);
	utimer_assert_intern(timer->expire);

	stroll_dlist_remove_init(&timer->node);
	stroll_dlist_insert_inorder_back(&utimer_the_list,
	                                 &timer->node,
	                                 utimer_tick_cmp,
	                                 NULL);
}

void
utimer_arm_tspec(struct utimer * __restrict         timer,
                 const struct timespec * __restrict tspec)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(tspec);

	timer->tick = utimer_tick_from_tspec_upper(tspec);

	utimer_arm(timer);
}

void
utimer_arm_msec(struct utimer * __restrict timer, unsigned long msec)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(msec);

	struct timespec date;

	utime_monotonic_now(&date);
	utime_tspec_add_msec(&date, msec);

	utimer_arm_tspec(timer, &date);
}

void
utimer_arm_sec(struct utimer * __restrict timer, unsigned long sec)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(sec);

	struct timespec date;

	utime_monotonic_now(&date);
	utime_tspec_add_sec(&date, sec);

	utimer_arm_tspec(timer, &date);
}

uint64_t
utimer_issue_tick(void)
{
	if (!stroll_dlist_empty(&utimer_the_list)) {
		const struct utimer * timer =
			utimer_lead_timer(&utimer_the_list);

		utimer_assert_intern(timer);
		utimer_assert_intern(timer->tick);
		utimer_assert_intern(timer->expire);

		return timer->tick;
	}
	else
		return 0;
}

void
utimer_run(void)
{
	uint64_t tick = 0;

	while (!stroll_dlist_empty(&utimer_the_list)) {
		struct utimer * timer = utimer_lead_timer(&utimer_the_list);

		utimer_assert_intern(timer);
		utimer_assert_intern(timer->tick);
		utimer_assert_intern(timer->expire);

		if (tick < timer->tick) {
			tick = utimer_tick();
			if (tick < timer->tick)
				return;
		}

		stroll_dlist_remove_init(&timer->node);
		timer->expire(timer);
	}
}

#endif /* defined(CONFIG_UTILS_TIMER_LIST) */

/******************************************************************************
 * Hierarchical timer wheel handling
 ******************************************************************************/

#if defined(CONFIG_UTILS_TIMER_HWHEEL)

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

/*
 * Maximum number of ticks that the hierarchical timer wheel may handle (without
 * accounting for sorted eternal timer list).
 */
#define UTIMER_HWHEEL_TICKS_NR \
	(1UL << (UTIMER_HWHEEL_SLOT_BITS * UTIMER_HWHEEL_LEVELS_NR))

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

static __utils_nonull(1, 2) __utils_nothrow
void
utimer_hwheel_enroll(struct utimer_hwheel * __restrict hwheel,
                     struct utimer * __restrict        timer)
{
	utimer_assert_intern(hwheel);
	utimer_assert_intern(timer);
	utimer_assert_intern(timer->tick);
	utimer_assert_intern(timer->expire);

	uint64_t tmout = stroll_max(timer->tick, hwheel->tick) - hwheel->tick;

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

		slot = (tmout >> bits) & UTIMER_HWHEEL_SLOT_MASK;
		stroll_dlist_insert(&hwheel->slots[lvl][slot], &timer->node);
	}
	else
		stroll_dlist_insert_inorder_back(&hwheel->eternal,
		                                 &timer->node,
		                                 utimer_tick_cmp,
		                                 NULL);
}

static __utils_nonull(1, 2) __utils_nothrow
void
utimer_hwheel_account(struct utimer * __restrict   timer,
                      struct timespec * __restrict now)
{
	utimer_assert_intern(timer);
	utimer_assert_intern(now);

	utime_monotonic_now(now);

	if (!utimer_the_hwheel.count) {
		utimer_assert_api(stroll_dlist_empty(&timer->node));

		utimer_the_hwheel.count++;
		utimer_the_hwheel.tick = utimer_tick_from_tspec_lower(now);
	}
}

void
utimer_arm_tspec(struct utimer * __restrict         timer,
                 const struct timespec * __restrict tspec)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(tspec);

	struct timespec date;

	utimer_hwheel_account(timer, &date);

	timer->tick = utimer_tick_from_tspec_upper(tspec);
	utimer_the_hwheel.issue = stroll_min(timer->tick,
	                                     utimer_the_hwheel.issue);

	utimer_hwheel_enroll(&utimer_the_hwheel, timer);
}

void
utimer_arm_msec(struct utimer * __restrict timer, unsigned long msec)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(msec);

	struct timespec date;

	utimer_hwheel_account(timer, &date);
	utime_tspec_add_msec(&date, msec);

	timer->tick = utimer_tick_from_tspec_upper(&date);
	utimer_the_hwheel.issue = stroll_min(timer->tick,
	                                     utimer_the_hwheel.issue);

	utimer_hwheel_enroll(&utimer_the_hwheel, timer);
}

void
utimer_arm_sec(struct utimer * __restrict timer, unsigned long sec)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);
	utimer_assert_api(sec);

	struct timespec date;

	utimer_hwheel_account(timer, &date);
	utime_tspec_add_sec(&date, sec);

	timer->tick = utimer_tick_from_tspec_upper(&date);
	utimer_the_hwheel.issue = stroll_min(timer->tick,
	                                     utimer_the_hwheel.issue);

	utimer_hwheel_enroll(&utimer_the_hwheel, timer);
}

void
utimer_cancel(struct utimer * __restrict timer)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);

	if (!stroll_dlist_empty(&timer->node)) {
		stroll_dlist_remove_init(&timer->node);

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
	while (!stroll_dlist_empty(timers)) {
		struct utimer * timer = utimer_lead_timer(timers);

		utimer_assert_intern(timer);
		utimer_assert_intern(timer->tick);
		utimer_assert_intern(timer->expire);

		stroll_dlist_remove(&timer->node);
		utimer_hwheel_enroll(hwheel, timer);
	}
}

static __utils_nonull(1) __utils_nothrow
void
utimer_hwheel_cascade(struct utimer_hwheel * __restrict hwheel)
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

	struct utimer * timer = NULL;

	if (!stroll_dlist_empty(&nodes[slot])) {
		timer = utimer_lead_timer(&nodes[slot]);
		utimer_assert_intern(timer);
		utimer_assert_intern(timer->tick);
		utimer_assert_intern(timer->expire);
	}

	return timer;
}

/* TODO: refactor me !! */
static __utils_nonull(1) __utils_nothrow
uint64_t
utimer_hwheel_find_issue(struct utimer_hwheel * __restrict hwheel)
{
	unsigned int          slot = hwheel->tick & UTIMER_HWHEEL_SLOT_MASK;
	unsigned int          curr;
	const struct utimer * timer;
	uint64_t              tick = hwheel->tick;
	uint64_t              issue = tick + UTIMER_HWHEEL_TICKS_NR - 1;
	bool                  found = false;
	unsigned int          lvl;

	for (curr = slot; curr < UTIMER_HWHEEL_SLOTS_PER_WHEEL; curr++) {
		timer = utimer_slot_lead_timer(hwheel->slots[0], curr);
		if (timer) {
			issue = timer->tick;
			found = true;
			break;
		}
	}

	if (slot) {
		if (found)
			return issue;

		for (curr = 0; curr < slot; curr++) {
			timer = utimer_slot_lead_timer(hwheel->slots[0], curr);
			if (timer) {
				issue = timer->tick;
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
			                           timer,
			                           node) {
				if (timer->tick < issue) {
					issue = timer->tick;
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
				                           timer,
				                           node) {
					if (timer->tick < issue) {
						issue = timer->tick;
						found = true;
					}
				}

				if (found)
					break;
			}

			tick += UTIMER_HWHEEL_SLOTS_PER_WHEEL - slot;
		}
	}

	timer = utimer_lead_timer(&hwheel->eternal);
	if (timer)
		issue = timer->tick;

	return issue;
}

uint64_t
utimer_issue_tick(void)
{
	if (!utimer_the_hwheel.count)
		return 0;

	if (utimer_the_hwheel.issue <= utimer_the_hwheel.tick)
		utimer_the_hwheel.issue =
			utimer_hwheel_find_issue(&utimer_the_hwheel);

	return utimer_the_hwheel.issue;
}

void
utimer_run(void)
{
	uint64_t tick = utimer_tick();

	while (tick > utimer_the_hwheel.tick) {
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
			struct utimer * timer = utimer_lead_timer(expired);

			utimer_assert_intern(timer);
			utimer_assert_intern(timer->tick);
			utimer_assert_intern(timer->expire);
			utimer_assert_intern(tick >= timer->tick);

			stroll_dlist_remove_init(&timer->node);
			timer->expire(timer);

			utimer_the_hwheel.count--;
		}

		tick = utimer_tick();
	}
}

/* TODO: expose timer lib init API ?? */
static __utils_nothrow __ctor()
void
utimer_hwheel_init(void)
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

#endif /* defined(CONFIG_UTILS_TIMER_HWHEEL) */
