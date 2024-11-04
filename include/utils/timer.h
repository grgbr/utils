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
#include <stdint.h>

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
 * Detect size of time_t type used to encode tv_sec field of struct timespec.
 *
 * As stated into the glibc manual section "Feature Test Macros", the _TIME_BITS
 * macro controls the bit size of time_t, and therefore the bit size of all
 * time_t-derived types and the prototypes of all related functions.
 * 
 * See
 * https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
 * for more infos.
 */
#if !defined(_TIME_BITS)
#define UTIMER_TIMET_BITS __TIMESIZE
#else  /* defined(_TIME_BITS) */
#define UTIMER_TIMET_BITS _TIME_BITS
#endif /* !defined(_TIME_BITS) */

/*
 * Maximum tick value that can be encoded.
 *
 * Allows to prevent overflow while converting a tick's second part to the
 * tv_sec field of a struct timespec (tv_sec is a time_t, i.e., either a signed
 * 64 or 32 bits word).
 */
#if UTIMER_TIMET_BITS == 32
#define UTIMER_TICK_MAX \
	(UINT64_MAX >> (64 - (31 + UTIMER_TICK_SUBSEC_BITS)))

#elif UTIMER_TIMET_BITS == 64
#define UTIMER_TICK_MAX \
	(UINT64_MAX >> 1)
#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

/*
 * Maximum tick value that can be encoded as seconds within a time_t used by the
 * tv_sec field of struct timespec when compiled with signed 64-bits time_t.
 */
/*
 * Maximum tick value that can be encoded as seconds within a time_t used by the
 * tv_sec field of struct timespec when compiled with signed 32-bits time_t.
 */

#define UTIMER_SEC_MAX \
	((unsigned long)(UTIMER_TICK_MAX >> UTIMER_TICK_SUBSEC_BITS))

#define UTIMER_TVSEC_MAX \
	((time_t)(UTIMER_TICK_MAX >> UTIMER_TICK_SUBSEC_BITS))

/*
 * Maximum tick value that can be encoded as milliseconds within a signed long
 * (required since utimer_issue_msec() returns a long int).
 */
#define UTIMER_TICK_MSEC_MAX \
	((((uint64_t)(LONG_MAX) / UINT64_C(1000)) << \
	  UTIMER_TICK_SUBSEC_BITS) + \
	  ((((uint64_t)(LONG_MAX) % UINT64_C(1000)) << \
	    UTIMER_TICK_SUBSEC_BITS) / UINT64_C(1000)))

#define utimer_assert_tspec_api(tspec) \
	utimer_assert_api(_tspec); \
	utimer_assert_api((_tspec)->tv_sec >= 0); \
	utimer_assert_api((_tspec)->tv_sec <= UTIMER_TVSEC_MAX); \
	utimer_assert_api((_tspec)->tv_nsec >= 0); \
	utimer_assert_api((_tspec)->tv_nsec < 1000000000L)

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
uint64_t
utimer_tick_from_tspec_lower(const struct timespec * __restrict tspec)
{
	utimer_assert_tspec_api(tspec);

	/*
	 * ticks = (number of seconds * number of ticks per second) +
	 *         (number of nanoseconds / tick period)
	 * rounded to lower multiple of tick period.
	 */
	return ((uint64_t)tspec->tv_sec << UTIMER_TICK_SUBSEC_BITS) +
	       ((uint64_t)tspec->tv_nsec / UTIMER_TICK_NSEC);
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
uint64_t
utimer_tick_from_tspec_upper(const struct timespec * __restrict tspec)
{
	utimer_assert_tspec_api(tspec);

	/*
	 * ticks = (number of seconds * number of ticks per second) +
	 *         (number of nanoseconds / tick period)
	 * rounded to upper multiple of tick period.
	 */
	return ((uint64_t)tspec->tv_sec << UTIMER_TICK_SUBSEC_BITS) +
	       (((uint64_t)tspec->tv_nsec + UTIMER_TICK_NSEC - 1) /
	        UTIMER_TICK_NSEC);
}

static inline __utils_nonull(2) __utils_nothrow
void
utimer_tspec_from_tick(uint64_t tick, struct timespec * __restrict result)
{
	utimer_assert_api(tick <= UTIMER_TICK_MAX);
	utimer_assert_api(result);

	/* seconds = number of ticks / number of ticks per second */
	result->tv_sec = (time_t)(tick >> UTIMER_TICK_SUBSEC_BITS);
	/* nanoseconds = number of sub second ticks * tick period */
	result->tv_nsec = (tick & UTIMER_TICK_SUBSEC_MASK) * UTIMER_TICK_NSEC;
}

static inline __utils_const __utils_nothrow
unsigned long
utimer_msec_from_tick_lower(uint64_t tick)
{
	utimer_assert_api(tick <= UTIMER_TICK_MSEC_MAX);

	uint64_t secs = tick >> UTIMER_TICK_SUBSEC_BITS;
	uint64_t nsecs = (tick & UTIMER_TICK_SUBSEC_MASK) * UTIMER_TICK_NSEC;

	return (unsigned long)
	       ((secs * UINT64_C(1000)) + (nsecs / UINT64_C(1000000)));
}

static inline __utils_const __utils_nothrow
unsigned long
utimer_msec_from_tick_upper(uint64_t tick)
{
	utimer_assert_api(tick <= UTIMER_TICK_MSEC_MAX);

	uint64_t secs = tick >> UTIMER_TICK_SUBSEC_BITS;
	uint64_t nsecs = (tick & UTIMER_TICK_SUBSEC_MASK) * UTIMER_TICK_NSEC;

	return (unsigned long)
	       ((secs * UINT64_C(1000)) +
	        ((nsecs + UINT64_C(1000000) - 1) / UINT64_C(1000000)));
}

static inline __utils_const __utils_nothrow
unsigned long
utimer_sec_from_tick_lower(uint64_t tick)
{
	utimer_assert_api(tick <= UTIMER_TICK_MAX);

	return (unsigned long)(tick >> UTIMER_TICK_SUBSEC_BITS);
}

static inline __utils_const __utils_nothrow
unsigned long
utimer_sec_from_tick_upper(uint64_t tick)
{
	utimer_assert_api(tick <= UTIMER_TICK_MAX);

	return (unsigned long)
	       ((tick + UTIMER_TICK_SUBSEC_MASK) >> UTIMER_TICK_SUBSEC_BITS);
}

extern uint64_t
utimer_tick(void) __utils_nothrow __warn_result;

/******************************************************************************
 * Timer handling
 ******************************************************************************/

struct utimer;

typedef void (utimer_expire_fn)(struct utimer * __restrict);

struct utimer {
	struct stroll_dlist_node node;
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

extern int
utimer_issue_tick(uint64_t * __restrict tick)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

extern struct timespec *
utimer_issue_tspec(struct timespec * __restrict tspec)
	__utils_nonull(1) __utils_nothrow __warn_result;

extern long
utimer_issue_msec(void) __utils_nothrow __warn_result;

extern void utimer_run(void) __utils_nothrow;

#endif /* _UTILS_TIMER_H */
