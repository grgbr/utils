/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Timer interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2021
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_TIMER_H
#define _UTILS_TIMER_H

#include <utils/time.h>
#include <stroll/dlist.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define utimer_assert_api(_expr) \
	stroll_assert("utils:utimer", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define utimer_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

/******************************************************************************
 * Tick handling
 ******************************************************************************/

/*
 * Timer ticks sub second precision bits.
 *
 * Configure the tick period to 1/(2^UTIMER_TICK_SUBSEC_BITS) second.
 *
 * The table below gives tick sub second precision according to allowed
 * UTIMER_TICK_SUBSEC_BITS values:
 *
 *     UTIMER_TICK_SUBSEC_BITS     Tick period  Tick frequency
 *                              (milliseconds)         (Hertz)
 *                           0    1000.000000               1
 *                           1     500.000000               2
 *                           2     250.000000               4
 *                           3     125.000000               8
 *                           4      62.500000              16
 *                           5      31.250000              32
 *                           6      15.625000              64
 *                           7       7.812500             128
 *                           8       3.906250             256
 *                           9       1.953125             512
 *
 * Watch out!
 * The tick period MUST be a divisor of 1000000000 nanoseconds so that we can
 * perform power of 2 arithmetics (see utimer_tick_from_tspec_lower(),
 * utimer_tick_from_tspec_upper() and utimer_tspec_from_tick()).
 * This is the reason why UTIMER_TICK_SUBSEC_BITS MUST be < 10.
 */
#define UTIMER_TICK_SUBSEC_BITS \
	STROLL_CONCAT(CONFIG_UTILS_TIMER_SUBSEC_BITS, U)
#if (UTIMER_TICK_SUBSEC_BITS < 0) || (UTIMER_TICK_SUBSEC_BITS > 9)
#error Invalid tick sub second precision bits.
#endif

#define UTIMER_TICK_SUBSEC_MASK \
	((UINT64_C(1) << UTIMER_TICK_SUBSEC_BITS) - 1)

/* Period of a tick in nanoseconds */
#define UTIMER_TICK_NSEC \
	(UINT64_C(1000000000) >> UTIMER_TICK_SUBSEC_BITS)

/* Number of ticks per second. */
#define UTIMER_TICKS_PER_SEC \
	(1UL << UTIMER_TICK_SUBSEC_BITS)

/*
 * Maximum tick value that can be encoded.
 *
 * Allows to prevent overflow while converting a tick's second part to the
 * tv_sec field of a struct timespec (tv_sec is a time_t, i.e., either a signed
 * 64 or 32 bits word).
 */
#if UTIME_TIMET_BITS == 64

#define UTIMER_TICK_MAX \
	((uint64_t)(INT64_MAX))

#elif UTIME_TIMET_BITS == 32

#define UTIMER_TICK_MAX \
	(((uint64_t)(INT32_MAX) << UTIMER_TICK_SUBSEC_BITS) | \
	 UTIMER_TICK_SUBSEC_MASK)

#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

/*
 * Maximum value of a struct timespec's tv_sec field that can be converted to a
 * tick.
 */
#define UTIMER_TVSEC_MAX \
	((time_t)(UTIMER_TICK_MAX >> UTIMER_TICK_SUBSEC_BITS))

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

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
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
	       ((int64_t)tspec->tv_nsec / UTIMER_TICK_NSEC);
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
utimer_tick_from_tspec_upper(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	if (tspec->tv_sec <= UTIMER_TVSEC_MAX) {
		int64_t tick;

		if (!utimer_int64_add_overflow(
			(int64_t)tspec->tv_sec << UTIMER_TICK_SUBSEC_BITS,
			((int64_t)tspec->tv_nsec + UTIMER_TICK_NSEC - 1) /
			UTIMER_TICK_NSEC,
			&tick))
		return tick;
	}

	return (int64_t)-ERANGE;
}

#elif UTIME_TIMET_BITS == 32

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
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
	               (((int64_t)tspec->tv_nsec + UTIMER_TICK_NSEC - 1) /
	                UTIMER_TICK_NSEC);

	return (tick <= (int64_t)UTIMER_TICK_MAX) ? tick : (int64_t)-ERANGE;
}

#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int64_t
utimer_tick_from_tspec_upper_clamp(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	int64_t tick = utimer_tick_from_tspec_upper(tspec);

	return (tick >= 0) ? tick : UTIMER_TICK_MAX;
}

static inline __utils_const __utils_nothrow __warn_result
struct timespec
utimer_tspec_from_tick(uint64_t tick)
{
	utimer_assert_api(tick <= UTIMER_TICK_MAX);

	const struct timespec tspec = {
		/* seconds = number of ticks / number of ticks per second */
		.tv_sec = (time_t)(tick >> UTIMER_TICK_SUBSEC_BITS),
		/* nanoseconds = number of sub second ticks * tick period */
		.tv_nsec = (tick & UTIMER_TICK_SUBSEC_MASK) * UTIMER_TICK_NSEC
	};

	return tspec;
}

static inline __utils_const __utils_nothrow __warn_result
int64_t
utimer_tick_from_msec_lower(unsigned int msec)
{
	utimer_assert_api(msec <= INT_MAX);

	return ((int64_t)msec << UTIMER_TICK_SUBSEC_BITS) / INT64_C(1000);
}

static inline __utils_const __utils_nothrow __warn_result
int64_t
utimer_tick_from_msec_upper(unsigned int msec)
{
	utimer_assert_api(msec <= INT_MAX);

	return (((int64_t)msec << UTIMER_TICK_SUBSEC_BITS) + INT64_C(999)) /
	       INT64_C(1000);
}

static inline __utils_const __utils_nothrow __warn_result
int
utimer_msec_from_tick_lower(uint64_t tick)
{
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

static inline __utils_const __utils_nothrow __warn_result
int
utimer_msec_from_tick_lower_clamp(uint64_t tick)
{
	utimer_assert_api(tick <= UTIMER_TICK_MAX);

	int msec = utimer_msec_from_tick_lower(tick);

	return (msec >= 0) ? msec : INT_MAX;
}

static inline __utils_const __utils_nothrow __warn_result
int
utimer_msec_from_tick_upper(uint64_t tick)
{
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

static inline __utils_const __utils_nothrow __warn_result
int
utimer_msec_from_tick_upper_clamp(uint64_t tick)
{
	utimer_assert_api(tick <= UTIMER_TICK_MAX);

	int msec = utimer_msec_from_tick_upper(tick);

	return (msec >= 0) ? msec : INT_MAX;
}

/******************************************************************************
 * Timer handling
 ******************************************************************************/

struct utimer;

typedef void (utimer_expire_fn)(struct utimer * __restrict);

struct utimer {
	struct stroll_dlist_node node;
	struct timespec *        tspec;
	uint64_t                 tick;
	utimer_expire_fn *       expire;
};

#define UTIMER_INIT(_timer, _expire) \
	{ \
		.node   = STROLL_DLIST_INIT((_timer).node), \
		.expire = _expire \
	}

static inline __utils_nonull(1) __utils_pure  __utils_nothrow __warn_result
uint64_t
utimer_expiry_tick(const struct utimer * __restrict timer)
{
	utimer_assert_api(timer);
	utimer_assert_api(stroll_dlist_empty(&timer->node) || timer->expire);

	return timer->tick;
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
utimer_is_armed(const struct utimer * __restrict timer)
{
	utimer_assert_api(timer);
	utimer_assert_api(stroll_dlist_empty(&timer->node) || timer->expire);

	return !stroll_dlist_empty(&timer->node);
}

#if defined(CONFIG_UTILS_TIMER_LIST)

extern void
utimer_arm_tspec(struct utimer * __restrict         timer,
                 const struct timespec * __restrict tspec)
	__utils_nonull(1, 2) __utils_nothrow;

extern void
utimer_arm_msec(struct utimer * __restrict timer, unsigned long msec)
	__utils_nonull(1) __utils_nothrow;

extern void
utimer_arm_sec(struct utimer * __restrict timer, unsigned long sec)
	__utils_nonull(1) __utils_nothrow;

static inline __utils_nonull(1) __utils_nothrow
void
utimer_cancel(struct utimer * __restrict timer)
{
	utimer_assert_api(timer);
	utimer_assert_api(timer->expire);

	stroll_dlist_remove_init(&timer->node);
}

#endif /* defined(CONFIG_UTILS_TIMER_LIST) */

#if defined(CONFIG_UTILS_TIMER_HWHEEL)

extern void
utimer_arm_tspec(struct utimer * __restrict         timer,
                 const struct timespec * __restrict tspec)
	__utils_nonull(1, 2) __utils_nothrow __leaf;

extern void
utimer_arm_msec(struct utimer * __restrict timer, unsigned long msec)
	__utils_nonull(1) __utils_nothrow __leaf;

extern void
utimer_arm_sec(struct utimer * __restrict timer, unsigned long sec)
	__utils_nonull(1) __utils_nothrow __leaf;

extern void
utimer_cancel(struct utimer * __restrict timer)
	__utils_nonull(1) __utils_nothrow __leaf;

#endif /* defined(CONFIG_UTILS_TIMER_HWHEEL) */

extern void
utimer_arm_tick(struct utimer * __restrict timer, uint64_t tick)
	__utils_nonull(1) __utils_nothrow __leaf;

static inline __utils_nonull(1, 2) __utils_nothrow
void
utimer_setup(struct utimer * __restrict timer,
             utimer_expire_fn *         expire)
{
	utimer_assert_api(timer);
	utimer_assert_api(expire);

	timer->expire = expire;
}

static inline __utils_nonull(1) __utils_nothrow
void
utimer_init(struct utimer * __restrict timer, utimer_expire_fn * expire)
{
	utimer_assert_api(timer);

	stroll_dlist_init(&timer->node);
	timer->expire = expire;
}

extern struct timespec *
utimer_issue_tspec(struct timespec * __restrict tspec)
	__utils_nonull(1) __utils_nothrow __warn_result;

extern int
utimer_issue_msec(void) __utils_nothrow __warn_result;

extern void utimer_run(void) __utils_nothrow;

#endif /* _UTILS_TIMER_H */
