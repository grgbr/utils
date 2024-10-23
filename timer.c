/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "timer.h"

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

uint64_t
utimer_tick(void)
{
	struct timespec now;

	utime_monotonic_now(&now);
	
	return utimer_tick_from_tspec_lower(&now);
}

int
utimer_tick_cmp(const struct stroll_dlist_node * __restrict first,
                const struct stroll_dlist_node * __restrict second,
                void *                                      data __unused)
{
	utimer_assert_intern(first);
	utimer_assert_intern(second);

	const struct utimer * fst = stroll_dlist_entry(first,
	                                               const struct utimer,
	                                               node);
	const struct utimer * snd = stroll_dlist_entry(second,
	                                               const struct utimer,
	                                               node);

	return (fst->tick > snd->tick) - (fst->tick < snd->tick);
}

/******************************************************************************
 * Timer handling
 ******************************************************************************/

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
		const struct utimer * timer;

		timer = stroll_dlist_entry(stroll_dlist_next(&utimer_the_list),
		                           struct utimer,
		                           node);
		utimer_assert_intern(timer);
		utimer_assert_intern(timer->tick);
		utimer_assert_intern(timer->expire);

		return timer->tick;
	}
	else
		return 0;
}

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
	uint64_t tick;

	tick = utimer_issue_tick();
	if (tick) {
		struct timespec delay;
		struct timespec now;

		utimer_tspec_from_tick(tick, &delay);
		utime_monotonic_now(&now);

		/* delay = delay - now */
		utime_tspec_sub(&delay, &now);

		/* Convert to milliseconds and clamp down to 0. */
		return stroll_max(utime_msec_from_tspec(&delay), 0L);
	}
	else
		return -1;
}

void
utimer_run(void)
{
	uint64_t tick = 0;

	while (!stroll_dlist_empty(&utimer_the_list)) {
		struct utimer * timer;

		timer = stroll_dlist_entry(stroll_dlist_next(&utimer_the_list),
		                           struct utimer,
		                           node);
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
