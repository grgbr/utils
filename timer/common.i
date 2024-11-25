/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifdef _ETUX_TIMER_COMMON_I
#error "Multiple inclusions of common timer definitions !"
#endif /* _ETUX_TIMER_COMMON_I */
#define _ETUX_TIMER_COMMON_I

#include <errno.h>

/*
 * Timer ticks sub second precision bits.
 *
 * Configure the tick period to 1/(2^ETUX_TIMER_TICK_SUBSEC_BITS) second.
 *
 * The table below gives tick sub second precision according to allowed
 * ETUX_TIMER_TICK_SUBSEC_BITS values:
 *
 *     ETUX_TIMER_TICK_SUBSEC_BITS     Tick period  Tick frequency
 *                                  (milliseconds)         (Hertz)
 *                               0     1000.000000               1
 *                               1      500.000000               2
 *                               2      250.000000               4
 *                               3      125.000000               8
 *                               4       62.500000              16
 *                               5       31.250000              32
 *                               6       15.625000              64
 *                               7        7.812500             128
 *                               8        3.906250             256
 *                               9        1.953125             512
 *
 * Watch out!
 * The tick period MUST be a divisor of 1000000000 nanoseconds so that we can
 * perform power of 2 arithmetics (see etux_timer_tick_from_tspec_lower(),
 * etux_timer_tick_from_tspec_upper() and etux_timer_tspec_from_tick()).
 * This is the reason why ETUX_TIMER_TICK_SUBSEC_BITS MUST be < 10.
 */
#define ETUX_TIMER_TICK_SUBSEC_BITS \
	STROLL_CONCAT(CONFIG_ETUX_TIMER_SUBSEC_BITS, U)
#if (ETUX_TIMER_TICK_SUBSEC_BITS < 0) || (ETUX_TIMER_TICK_SUBSEC_BITS > 9)
#error Invalid tick sub second precision bits.
#endif

#define ETUX_TIMER_TICK_SUBSEC_MASK \
	((INT64_C(1) << ETUX_TIMER_TICK_SUBSEC_BITS) - 1)

/* Period of a tick in nanoseconds */
#define ETUX_TIMER_TICK_NSEC \
	(INT64_C(1000000000) >> ETUX_TIMER_TICK_SUBSEC_BITS)

/* Number of ticks per second. */
#define ETUX_TIMER_TICKS_PER_SEC \
	(1UL << ETUX_TIMER_TICK_SUBSEC_BITS)

/*
 * Maximum tick value that can be encoded.
 *
 * Allows to prevent overflow while converting a tick's second part to the
 * tv_sec field of a struct timespec (tv_sec is a time_t, i.e., either a signed
 * 64 or 32 bits word).
 */
#if UTIME_TIMET_BITS == 64

#define ETUX_TIMER_TICK_MAX \
	(INT64_MAX)

#elif UTIME_TIMET_BITS == 32

#define ETUX_TIMER_TICK_MAX \
	(((int64_t)(INT32_MAX) << ETUX_TIMER_TICK_SUBSEC_BITS) | \
	 ETUX_TIMER_TICK_SUBSEC_MASK)

#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

/*
 * Maximum value of a struct timespec's tv_sec field that can be converted to a
 * tick.
 */
#define ETUX_TIMER_TVSEC_MAX \
	((time_t)(ETUX_TIMER_TICK_MAX >> ETUX_TIMER_TICK_SUBSEC_BITS))

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_timer_assert_intern(_expr) \
	stroll_assert("etux:timer", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_timer_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_timer_assert_timer_intern(_timer) \
	etux_timer_assert_intern(_timer); \
	etux_timer_assert_intern(((_timer)->state != ETUX_TIMER_PEND_STAT) || \
	                         (!stroll_dlist_empty(&(_timer)->node) && \
	                          (_timer)->expire)); \
	etux_timer_assert_intern(((_timer)->state != ETUX_TIMER_RUN_STAT) || \
	                         (_timer)->expire)

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

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
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

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
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

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
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

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
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

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
etux_timer_tick_from_tspec_lower_clamp(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	int64_t tick = etux_timer_tick_from_tspec_lower(tspec);

	return (tick >= 0) ? tick : ETUX_TIMER_TICK_MAX;
}

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
etux_timer_tick_from_tspec_upper_clamp(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	int64_t tick = etux_timer_tick_from_tspec_upper(tspec);

	return (tick >= 0) ? tick : ETUX_TIMER_TICK_MAX;
}

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

/******************************************************************************
 * Generic timer handling
 ******************************************************************************/

static inline __utils_nonull(1) __utils_const __utils_nothrow __returns_nonull
struct etux_timer *
etux_timer_timer_from_node(const struct stroll_dlist_node * __restrict node)
{
	etux_timer_assert_intern(node);

	return stroll_dlist_entry(node, struct etux_timer, node);
}

static __utils_nonull(1) __utils_pure __utils_nothrow __returns_nonull
struct etux_timer *
etux_timer_lead_timer(const struct stroll_dlist_node * __restrict head)
{
	etux_timer_assert_intern(head);
	etux_timer_assert_intern(!stroll_dlist_empty(head));

	struct etux_timer * tmr = stroll_dlist_entry(stroll_dlist_next(head),
	                                             struct etux_timer,
	                                             node);

	etux_timer_assert_timer_intern(tmr);
	etux_timer_assert_intern(tmr->expire);

	return tmr;
}

static __utils_nonull(1, 2) __utils_pure __utils_nothrow __warn_result
int
etux_timer_tick_cmp(const struct stroll_dlist_node * __restrict first,
                    const struct stroll_dlist_node * __restrict second,
                    void *                                      data __unused)
{
	etux_timer_assert_intern(first);
	etux_timer_assert_intern(second);

	const struct etux_timer * fst = etux_timer_timer_from_node(first);
	const struct etux_timer * snd = etux_timer_timer_from_node(second);

	etux_timer_assert_intern(fst->tick >= 0);
	etux_timer_assert_intern(snd->tick >= 0);

	return (fst->tick > snd->tick) - (fst->tick < snd->tick);
}

static __utils_nonull(1, 2) __utils_nothrow
void
etux_timer_insert(struct stroll_dlist_node * __restrict list,
                  struct etux_timer * __restrict        timer)
{
	stroll_dlist_insert_inorder_back(list,
	                                 &timer->node,
	                                 etux_timer_tick_cmp,
	                                 NULL);
}

static __utils_nonull(1) __utils_nothrow
void
etux_timer_dismiss(struct etux_timer * __restrict timer)
{
	etux_timer_assert_timer_intern(timer);

	stroll_dlist_remove(&timer->node);
	timer->state = ETUX_TIMER_IDLE_STAT;
}

static __utils_nothrow __warn_result
int64_t
etux_timer_tick(void)
{
	struct timespec now;

	utime_monotonic_now(&now);
	
	return etux_timer_tick_from_tspec_lower_clamp(&now);
}

struct timespec *
etux_timer_issue_tspec(struct timespec * __restrict tspec)
{
	etux_timer_assert_api(tspec);

	int64_t issue;

	issue = etux_timer_issue_tick();
	if (issue >= 0) {
		*tspec = etux_timer_tspec_from_tick(issue);

		return tspec;
	}
	else
		return NULL;
}

int
etux_timer_issue_msec(void)
{
	struct timespec diff;

	if (etux_timer_issue_tspec(&diff)) {
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

/* ex: set filetype=c : */
