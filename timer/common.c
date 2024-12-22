/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "common.h"
#include <errno.h>

/*
 * Maximum value of a struct timespec's tv_sec field that can be converted to a
 * tick.
 */
#define ETUX_TIMER_TVSEC_MAX \
	((time_t)(ETUX_TIMER_TICK_MAX >> ETUX_TIMER_TICK_SUBSEC_BITS))

/******************************************************************************
 * Tick handling
 ******************************************************************************/

#if __WORDSIZE == 64

static inline __nonull(3) __nothrow __warn_result
bool
etux_timer_int64_add_overflow(int64_t a, int64_t b, int64_t * __restrict res)
{
	return __builtin_saddl_overflow((long)a, (long)b, (long *)res);
}

#elif __WORDSIZE == 32

static inline __nonull(3) __nothrow __warn_result
bool
etux_timer_int64_add_overflow(int64_t a, int64_t b, int64_t * __restrict res)
{
	return __builtin_saddll_overflow((long long)a,
	                                 (long long)b,
	                                 (long long *)res);
}

#else
#error "Unsupported machine word size !"
#endif

#if UTIME_TIMET_BITS == 64

int64_t
etux_timer_tick_from_tspec_lower(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	if (tspec->tv_sec > ETUX_TIMER_TVSEC_MAX)
		return (int64_t)-ERANGE;

	/*
	 * ticks = (number of seconds * number of ticks per second) +
	 *         (number of nanoseconds / tick period)
	 * rounded to upper multiple of tick period.
	 */
	return ((int64_t)tspec->tv_sec << ETUX_TIMER_TICK_SUBSEC_BITS) |
	       ((int64_t)tspec->tv_nsec / (int64_t)ETUX_TIMER_TICK_NSEC);
}

int64_t
etux_timer_tick_from_tspec_upper(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	if (tspec->tv_sec <= ETUX_TIMER_TVSEC_MAX) {
		int64_t tick;

		if (!etux_timer_int64_add_overflow(
			(int64_t)tspec->tv_sec << ETUX_TIMER_TICK_SUBSEC_BITS,
			((int64_t)tspec->tv_nsec +
			 (int64_t)ETUX_TIMER_TICK_NSEC - 1) /
			(int64_t)ETUX_TIMER_TICK_NSEC,
			&tick))
		return tick;
	}

	return (int64_t)-ERANGE;
}

#elif UTIME_TIMET_BITS == 32

int64_t
etux_timer_tick_from_tspec_lower(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	/*
	 * ticks = (number of seconds * number of ticks per second) +
	 *         (number of nanoseconds / tick period)
	 * rounded to upper multiple of tick period.
	 */
	return ((int64_t)tspec->tv_sec << ETUX_TIMER_TICK_SUBSEC_BITS) |
	       ((int64_t)tspec->tv_nsec / (int64_t)ETUX_TIMER_TICK_NSEC);
}

int64_t
etux_timer_tick_from_tspec_upper(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	/*
	 * ticks = (number of seconds * number of ticks per second) +
	 *         (number of nanoseconds / tick period)
	 * rounded to upper multiple of tick period.
	 */
	int64_t tick = ((int64_t)tspec->tv_sec << ETUX_TIMER_TICK_SUBSEC_BITS) +
	               (((int64_t)tspec->tv_nsec +
	                 (int64_t)ETUX_TIMER_TICK_NSEC - 1) /
	                (int64_t)ETUX_TIMER_TICK_NSEC);

	return (tick <= ETUX_TIMER_TICK_MAX) ? tick : (int64_t)-ERANGE;
}

#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

#warning REMOVE ME if not needed...
#if 0
static __utils_const __utils_nothrow __warn_result
int
etux_timer_msec_from_tick_lower(int64_t tick)
{
	etux_timer_assert_api(tick >= 0);
	etux_timer_assert_api(tick <= ETUX_TIMER_TICK_MAX);

	int msec;
	int nsec = (int)((tick & ETUX_TIMER_TICK_SUBSEC_MASK) * ETUX_TIMER_TICK_NSEC);

	if (!__builtin_mul_overflow((int64_t)tick >> ETUX_TIMER_TICK_SUBSEC_BITS,
	                            1000,
	                            &msec) &&
	    !__builtin_sadd_overflow(msec, nsec / 1000000, &msec))
		return msec;

	return -ERANGE;
}

static __utils_const __utils_nothrow __warn_result
int
etux_timer_msec_from_tick_lower_clamp(int64_t tick)
{
	etux_timer_assert_api(tick >= 0);
	etux_timer_assert_api(tick <= ETUX_TIMER_TICK_MAX);

	int msec = etux_timer_msec_from_tick_lower(tick);

	return (msec >= 0) ? msec : INT_MAX;
}

static __utils_const __utils_nothrow __warn_result
int
etux_timer_msec_from_tick_upper(int64_t tick)
{
	etux_timer_assert_api(tick >= 0);
	etux_timer_assert_api(tick <= ETUX_TIMER_TICK_MAX);

	int msec;
	int nsec = (int)((tick & ETUX_TIMER_TICK_SUBSEC_MASK) *
	                 ETUX_TIMER_TICK_NSEC);

	if (!__builtin_mul_overflow((int64_t)
	                            tick >> ETUX_TIMER_TICK_SUBSEC_BITS,
	                            1000,
	                            &msec) &&
	    !__builtin_sadd_overflow(msec, (nsec + 999999) / 1000000, &msec))
		return msec;

	return -ERANGE;
}

static __utils_const __utils_nothrow __warn_result
int
etux_timer_msec_from_tick_upper_clamp(int64_t tick)
{
	etux_timer_assert_api(tick >= 0);
	etux_timer_assert_api(tick <= ETUX_TIMER_TICK_MAX);

	int msec = etux_timer_msec_from_tick_upper(tick);

	return (msec >= 0) ? msec : INT_MAX;
}
#endif

static __utils_const __utils_nothrow __warn_result
struct timespec
etux_timer_tspec_from_tick(int64_t tick)
{
	etux_timer_assert_api(tick >= 0);
	etux_timer_assert_api(tick <= ETUX_TIMER_TICK_MAX);

	const struct timespec tspec = {
		/* seconds = number of ticks / number of ticks per second */
		.tv_sec = (time_t)(tick >> ETUX_TIMER_TICK_SUBSEC_BITS),
		/* nanoseconds = number of sub second ticks * tick period */
		.tv_nsec = (tick & ETUX_TIMER_TICK_SUBSEC_MASK) *
		           ETUX_TIMER_TICK_NSEC
	};

	return tspec;
}

#warning REMOVE ME if not needed...
#if 0

static inline __utils_const __utils_nothrow __warn_result
int64_t
etux_timer_tick_from_msec_lower(int msec)
{
	etux_timer_assert_api(msec >= 0);

	return ((int64_t)msec << ETUX_TIMER_TICK_SUBSEC_BITS) / INT64_C(1000);
}

static inline __utils_const __utils_nothrow __warn_result
int64_t
etux_timer_tick_from_msec_upper(int msec)
{
	etux_timer_assert_api(msec >= 0);

	return (((int64_t)msec << ETUX_TIMER_TICK_SUBSEC_BITS) + INT64_C(999)) /
	       INT64_C(1000);
}

#endif

int64_t
etux_timer_tick_load(struct timespec * __restrict now)
{
	etux_timer_assert_intern(now);

	utime_monotonic_now(now);

	return etux_timer_tick_from_tspec_lower_clamp(now);
}

/******************************************************************************
 * Timer generic logic
 ******************************************************************************/

static inline __utils_nonull(1) __utils_const __utils_nothrow __returns_nonull
struct etux_timer *
etux_timer_from_list_node(const struct stroll_dlist_node * __restrict node)
{
	etux_timer_assert_intern(node);

	return stroll_dlist_entry(node, struct etux_timer, list);
}

static __utils_nonull(1, 2) __utils_pure __utils_nothrow __warn_result
int
etux_timer_tick_cmp(const struct stroll_dlist_node * __restrict first,
                    const struct stroll_dlist_node * __restrict second,
                    void *                                      data __unused)
{
	etux_timer_assert_intern(first);
	etux_timer_assert_intern(second);

	const struct etux_timer * fst = etux_timer_from_list_node(first);
	const struct etux_timer * snd = etux_timer_from_list_node(second);

	etux_timer_assert_intern(fst->tick >= 0);
	etux_timer_assert_intern(snd->tick >= 0);

	return (fst->tick > snd->tick) - (fst->tick < snd->tick);
}

__utils_nonull(1, 2) __utils_nothrow
void
etux_timer_insert_inorder(struct stroll_dlist_node * __restrict list,
                          struct etux_timer * __restrict        timer)
{
	stroll_dlist_insert_inorder_back(list,
	                                 &timer->list,
	                                 etux_timer_tick_cmp,
	                                 NULL);
}

static __utils_nonull(1) __utils_nothrow __warn_result
struct timespec *
_etux_timer_issue_tspec(struct timespec * __restrict tspec)
{
	etux_timer_assert_intern(tspec);

	int64_t issue;

	issue = etux_timer_issue_tick();
	if (issue >= 0) {
		*tspec = etux_timer_tspec_from_tick(issue);

		return tspec;
	}

	return NULL;
}

struct timespec *
etux_timer_issue_tspec(struct timespec * __restrict tspec)
{
	etux_timer_assert_api(tspec);

	etux_timer_issue_tspec_trace_enter();

	tspec = _etux_timer_issue_tspec(tspec);

	etux_timer_issue_tspec_trace_exit(tspec);

	return tspec;
}

int
etux_timer_issue_msec(void)
{
	struct timespec diff;
	int             msec;

	etux_timer_issue_msec_trace_enter();

	if (_etux_timer_issue_tspec(&diff)) {
		struct timespec now;

		utime_monotonic_now(&now);
		if (utime_tspec_sub(&diff, &now) > 0)
			msec = utime_msec_from_tspec_upper_clamp(&diff);
		else
			msec = 0;
	}
	else
		msec = -1;

	etux_timer_issue_msec_trace_exit(msec);

	return msec;
}

/* ex: set filetype=c : */
