/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "timer.h"
#include <errno.h>

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define utimer_assert_intern(_expr) \
	stroll_assert("utils:utimer", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define utimer_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

#define utimer_assert_timer_intern(_timer) \
	utimer_assert_intern(_timer); \
	utimer_assert_intern(((_timer)->state != UTIMER_PEND_STAT) || \
	                     (!stroll_dlist_empty(&(_timer)->node) && \
	                      (_timer)->expire)); \
	utimer_assert_intern(((_timer)->state != UTIMER_RUN_STAT) || \
	                     (_timer)->expire)

static inline __utils_nonull(1) __utils_const __utils_nothrow __returns_nonull
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

	struct utimer * tmr = stroll_dlist_entry(stroll_dlist_next(head),
	                                         struct utimer,
	                                         node);

	utimer_assert_timer_intern(tmr);
	utimer_assert_intern(tmr->expire);

	return tmr;
}

static __utils_nonull(1) __utils_nothrow
void
utimer_dismiss(struct utimer * __restrict timer)
{
	utimer_assert_timer_intern(timer);

	stroll_dlist_remove(&timer->node);
	timer->state = UTIMER_IDLE_STAT;
}

/******************************************************************************
 * Tick handling
 ******************************************************************************/

#if __WORDSIZE == 64

static inline __nonull(3) __nothrow __warn_result
bool
utimer_int64_add_overflow(int64_t a, int64_t b, int64_t * __restrict res)
{
	return __builtin_saddl_overflow((long)a, (long)b, (long *)res);
}

#elif __WORDSIZE == 32

static inline __nonull(3) __nothrow __warn_result
bool
utimer_int64_add_overflow(int64_t a, int64_t b, int64_t * __restrict res)
{
	return __builtin_saddll_overflow((long long)a,
	                                 (long long)b,
	                                 (long long *)res);
}

#else
#error "Unsupported machine word size !"
#endif

#if UTIME_TIMET_BITS == 64

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
utimer_tick_from_tspec_lower(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	if (tspec->tv_sec > UTIMER_TVSEC_MAX)
		return (int64_t)-ERANGE;

	/*
	 * ticks = (number of seconds * number of ticks per second) +
	 *         (number of nanoseconds / tick period)
	 * rounded to upper multiple of tick period.
	 */
	return ((int64_t)tspec->tv_sec << UTIMER_TICK_SUBSEC_BITS) |
	       ((int64_t)tspec->tv_nsec / (int64_t)UTIMER_TICK_NSEC);
}

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
utimer_tick_from_tspec_upper(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	if (tspec->tv_sec <= UTIMER_TVSEC_MAX) {
		int64_t tick;

		if (!utimer_int64_add_overflow(
			(int64_t)tspec->tv_sec << UTIMER_TICK_SUBSEC_BITS,
			((int64_t)tspec->tv_nsec +
			 (int64_t)UTIMER_TICK_NSEC - 1) /
			(int64_t)UTIMER_TICK_NSEC,
			&tick))
		return tick;
	}

	return (int64_t)-ERANGE;
}

#elif UTIME_TIMET_BITS == 32

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
utimer_tick_from_tspec_lower(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	/*
	 * ticks = (number of seconds * number of ticks per second) +
	 *         (number of nanoseconds / tick period)
	 * rounded to upper multiple of tick period.
	 */
	return ((int64_t)tspec->tv_sec << UTIMER_TICK_SUBSEC_BITS) |
	       ((int64_t)tspec->tv_nsec / (int64_t)UTIMER_TICK_NSEC);
}

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
utimer_tick_from_tspec_upper(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	/*
	 * ticks = (number of seconds * number of ticks per second) +
	 *         (number of nanoseconds / tick period)
	 * rounded to upper multiple of tick period.
	 */
	int64_t tick = ((int64_t)tspec->tv_sec << UTIMER_TICK_SUBSEC_BITS) +
	               (((int64_t)tspec->tv_nsec +
	                 (int64_t)UTIMER_TICK_NSEC - 1) /
	                (int64_t)UTIMER_TICK_NSEC);

	return (tick <= UTIMER_TICK_MAX) ? tick : (int64_t)-ERANGE;
}

#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
utimer_tick_from_tspec_lower_clamp(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	int64_t tick = utimer_tick_from_tspec_lower(tspec);

	return (tick >= 0) ? tick : UTIMER_TICK_MAX;
}

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
utimer_tick_from_tspec_upper_clamp(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	int64_t tick = utimer_tick_from_tspec_upper(tspec);

	return (tick >= 0) ? tick : UTIMER_TICK_MAX;
}

#warning REMOVE ME if not needed...
#if 0
static __utils_const __utils_nothrow __warn_result
int
utimer_msec_from_tick_lower(int64_t tick)
{
	utimer_assert_api(tick >= 0);
	utimer_assert_api(tick <= UTIMER_TICK_MAX);

	int msec;
	int nsec = (int)((tick & UTIMER_TICK_SUBSEC_MASK) * UTIMER_TICK_NSEC);

	if (!__builtin_mul_overflow((int64_t)tick >> UTIMER_TICK_SUBSEC_BITS,
	                            1000,
	                            &msec) &&
	    !__builtin_sadd_overflow(msec, nsec / 1000000, &msec))
		return msec;

	return -ERANGE;
}

static __utils_const __utils_nothrow __warn_result
int
utimer_msec_from_tick_lower_clamp(int64_t tick)
{
	utimer_assert_api(tick >= 0);
	utimer_assert_api(tick <= UTIMER_TICK_MAX);

	int msec = utimer_msec_from_tick_lower(tick);

	return (msec >= 0) ? msec : INT_MAX;
}

static __utils_const __utils_nothrow __warn_result
int
utimer_msec_from_tick_upper(int64_t tick)
{
	utimer_assert_api(tick >= 0);
	utimer_assert_api(tick <= UTIMER_TICK_MAX);

	int msec;
	int nsec = (int)((tick & UTIMER_TICK_SUBSEC_MASK) * UTIMER_TICK_NSEC);

	if (!__builtin_mul_overflow((int64_t)tick >> UTIMER_TICK_SUBSEC_BITS,
	                            1000,
	                            &msec) &&
	    !__builtin_sadd_overflow(msec, (nsec + 999999) / 1000000, &msec))
		return msec;

	return -ERANGE;
}

static __utils_const __utils_nothrow __warn_result
int
utimer_msec_from_tick_upper_clamp(int64_t tick)
{
	utimer_assert_api(tick >= 0);
	utimer_assert_api(tick <= UTIMER_TICK_MAX);

	int msec = utimer_msec_from_tick_upper(tick);

	return (msec >= 0) ? msec : INT_MAX;
}
#endif

static __utils_const __utils_nothrow __warn_result
struct timespec
utimer_tspec_from_tick(int64_t tick)
{
	utimer_assert_api(tick >= 0);
	utimer_assert_api(tick <= UTIMER_TICK_MAX);

	const struct timespec tspec = {
		/* seconds = number of ticks / number of ticks per second */
		.tv_sec = (time_t)(tick >> UTIMER_TICK_SUBSEC_BITS),
		/* nanoseconds = number of sub second ticks * tick period */
		.tv_nsec = (tick & UTIMER_TICK_SUBSEC_MASK) * UTIMER_TICK_NSEC
	};

	return tspec;
}

#warning REMOVE ME if not needed...
#if 0

static inline __utils_const __utils_nothrow __warn_result
int64_t
utimer_tick_from_msec_lower(int msec)
{
	utimer_assert_api(msec >= 0);

	return ((int64_t)msec << UTIMER_TICK_SUBSEC_BITS) / INT64_C(1000);
}

static inline __utils_const __utils_nothrow __warn_result
int64_t
utimer_tick_from_msec_upper(int msec)
{
	utimer_assert_api(msec >= 0);

	return (((int64_t)msec << UTIMER_TICK_SUBSEC_BITS) + INT64_C(999)) /
	       INT64_C(1000);
}

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

	utimer_assert_intern(fst->tick >= 0);
	utimer_assert_intern(snd->tick >= 0);

	return (fst->tick > snd->tick) - (fst->tick < snd->tick);
}

static __utils_nothrow __warn_result
int64_t
utimer_tick(void)
{
	struct timespec now;

	utime_monotonic_now(&now);
	
	return utimer_tick_from_tspec_lower_clamp(&now);
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
	utimer_assert_timer_intern(timer);
	utimer_assert_intern(timer->expire);

	if (timer->state == UTIMER_PEND_STAT)
		stroll_dlist_remove(&timer->node);

	timer->tick = utimer_tick_from_tspec_upper_clamp(&timer->tspec);

	stroll_dlist_insert_inorder_back(&utimer_the_list,
	                                 &timer->node,
	                                 utimer_tick_cmp,
	                                 NULL);

	timer->state = UTIMER_PEND_STAT;
}

void
utimer_arm_tspec(struct utimer * __restrict         timer,
                 const struct timespec * __restrict tspec)
{
	utimer_assert_timer_api(timer);
	utimer_assert_api(timer->expire);
	utime_assert_tspec_api(tspec);

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
	utime_tspec_add_sec_clamp(&timer->tspec, sec);

	utimer_arm(timer);
}

void
utimer_cancel(struct utimer * __restrict timer)
{
	utimer_assert_timer_api(timer);

	if (timer->state == UTIMER_PEND_STAT)
		utimer_dismiss(timer);
}

static __utils_pure __utils_nothrow __warn_result
int64_t
utimer_issue_tick(void)
{
	if (!stroll_dlist_empty(&utimer_the_list))
		return utimer_lead_timer(&utimer_the_list)->tick;
	else
		return (int64_t)-ENOENT;
}

void
utimer_run(void)
{
	int64_t tick = -1;

	while (!stroll_dlist_empty(&utimer_the_list)) {
		struct utimer * tmr = utimer_lead_timer(&utimer_the_list);

		if (tick < tmr->tick) {
			tick = utimer_tick();
			if (tick < tmr->tick)
				return;
		}

		tmr->state = UTIMER_RUN_STAT;
		stroll_dlist_remove(&tmr->node);

		tmr->expire(tmr);

		if (tmr->state == UTIMER_RUN_STAT)
			tmr->state = UTIMER_IDLE_STAT;
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

static __utils_nonull(1, 2) __utils_nothrow
void
utimer_hwheel_enroll(struct utimer_hwheel * __restrict hwheel,
                     struct utimer * __restrict        timer)
{
	utimer_assert_intern(hwheel);
	utimer_assert_intern(timer);
	utimer_assert_intern(timer->expire);

	int64_t tmout = stroll_max(timer->tick, hwheel->tick) - hwheel->tick;

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
utimer_arm(struct utimer * __restrict timer)
{
	utimer_assert_timer_intern(timer);
	utimer_assert_intern(timer->expire);

	if (timer->state != UTIMER_PEND_STAT)
		utimer_the_hwheel.count++;
	else
		stroll_dlist_remove(&timer->node);

	timer->tick = utimer_tick_from_tspec_upper_clamp(&timer->tspec);

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

			if (tmr->state == UTIMER_RUN_STAT)
				tmr->state = UTIMER_IDLE_STAT;

			utimer_the_hwheel.count--;
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

#endif /* defined(CONFIG_UTILS_TIMER_HWHEEL) */

/******************************************************************************
 * Timer generic handling
 ******************************************************************************/

struct timespec *
utimer_issue_tspec(struct timespec * __restrict tspec)
{
	utimer_assert_api(tspec);

	int64_t issue;

	issue = utimer_issue_tick();
	if (issue >= 0) {
		*tspec = utimer_tspec_from_tick(issue);

		return tspec;
	}
	else
		return NULL;
}

int
utimer_issue_msec(void)
{
	struct timespec diff;

	if (utimer_issue_tspec(&diff)) {
		struct timespec now;

		utime_monotonic_now(&now);
		if (utime_tspec_sub(&diff, &now) > 0)
			return utime_msec_from_tspec_upper_clamp(&diff);
		else
			return 0;
	}
	else
		return -1;
}
