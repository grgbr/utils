/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Time keeping interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2017
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_TIME_H
#define _UTILS_TIME_H

#include <utils/cdefs.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

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
#define UTIME_TIMET_BITS __TIMESIZE
#else  /* defined(_TIME_BITS) */
#define UTIME_TIMET_BITS _TIME_BITS
#endif /* !defined(_TIME_BITS) */

#if UTIME_TIMET_BITS == 64
#define UTIME_TIMET_MAX (INT64_MAX)
#elif UTIME_TIMET_BITS == 32
#define UTIME_TIMET_MAX (INT32_MAX)
#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

#define UTIME_TSPEC_MAX \
	((struct timespec){ .tv_sec = UTIME_TIMET_MAX, .tv_nsec = 999999999L })

/*
 * Ensure that fields of struct timespec are consistent, i.e. :
 * - tv_sec >= 0
 * - tv_nsec >= 0 and < 1,000,000,000.
 *
 * No support for negative timespec values since there is no consistent way to
 * represent them:
 * - storing -5 nanoseconds would end up with tv_sec = 0 and tv_nsec = -5 ;
 * - storing -1 second would end up with tv_sec = -1 and tv_nsec = 0.
 * This requires to use 2 different ways to store the negative sign...
 *
 * See https://www.gnu.org/software/libc/manual/html_node/Time-Types.html for
 * more informations.
 */
#define utime_assert_tspec_api(_tspec) \
	utime_assert_api(_tspec); \
	utime_assert_api((_tspec)->tv_sec >= 0); \
	utime_assert_api((_tspec)->tv_sec <= UTIME_TIMET_MAX); \
	utime_assert_api((_tspec)->tv_nsec >= 0); \
	utime_assert_api((_tspec)->tv_nsec < 1000000000L)

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define utime_assert_api(_expr) \
	stroll_assert("utils:utime", _expr)

static inline __utils_nonull(1) __utils_nothrow
void
utime_realtime_now(struct timespec * now)
{
	utime_assert_api(now);

	utime_assert_api(!clock_gettime(CLOCK_REALTIME, now));
}

static inline __utils_nonull(1) __utils_nothrow
void
utime_monotonic_now(struct timespec * now)
{
	utime_assert_api(now);

	utime_assert_api(!clock_gettime(CLOCK_MONOTONIC, now));
}

static inline __utils_nonull(1) __utils_nothrow
void
utime_boot_now(struct timespec * now)
{
	utime_assert_api(now);

	utime_assert_api(!clock_gettime(CLOCK_BOOTTIME, now));
}

static inline __utils_nonull(1) __utils_nothrow
void
utime_coarse_now(struct timespec * now)
{
	utime_assert_api(now);

	utime_assert_api(!clock_gettime(CLOCK_MONOTONIC_COARSE, now));
}

static inline __utils_nonull(1) __utils_nothrow
void
utime_proc_now(struct timespec * now)
{
	utime_assert_api(now);

	utime_assert_api(!clock_gettime(CLOCK_PROCESS_CPUTIME_ID, now));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define utime_assert_api(_expr)

static inline __nonull(1) __nothrow
void
utime_realtime_now(struct timespec * now)
{
	clock_gettime(CLOCK_REALTIME, now);
}

static inline __nonull(1) __nothrow
void
utime_monotonic_now(struct timespec * now)
{
	clock_gettime(CLOCK_MONOTONIC, now);
}

static inline __nonull(1) __nothrow
void
utime_coarse_now(struct timespec * now)
{
	clock_gettime(CLOCK_MONOTONIC_COARSE, now);
}

static inline __nonull(1) __nothrow
void
utime_boot_now(struct timespec * now)
{
	clock_gettime(CLOCK_BOOTTIME, now);
}

static inline __nonull(1) __nothrow
void
utime_proc_now(struct timespec * now)
{
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, now);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

extern int
utime_tspec_cmp(const struct timespec * __restrict first,
                const struct timespec * __restrict second)
	__utils_nonull(1, 2) __utils_pure __utils_nothrow __leaf __warn_result;

static inline __utils_nonull(1, 2) __utils_pure __utils_nothrow __warn_result
bool
utime_tspec_after(const struct timespec * __restrict first,
                  const struct timespec * __restrict second)
{
	return (utime_tspec_cmp(first, second) > 0);
}

static inline __utils_nonull(1, 2) __utils_pure __utils_nothrow __warn_result
bool
utime_tspec_after_eq(const struct timespec * __restrict first,
                     const struct timespec * __restrict second)
{
	return (utime_tspec_cmp(first, second) >= 0);
}

static inline __utils_nonull(1, 2) __utils_pure __utils_nothrow __warn_result
bool
utime_tspec_before(const struct timespec * __restrict first,
                   const struct timespec * __restrict second)
{
	return (utime_tspec_cmp(first, second) < 0);
}

static inline __utils_nonull(1, 2) __utils_pure __utils_nothrow __warn_result
bool
utime_tspec_before_eq(const struct timespec * __restrict first,
                      const struct timespec * __restrict second)
{
	return (utime_tspec_cmp(first, second) <= 0);
}

static inline __utils_const __utils_nothrow __warn_result
struct timespec
utime_tspec_from_msec(unsigned int msec)
{
	utime_assert_api(msec <= INT_MAX);

	const struct timespec tspec = {
		.tv_sec  = (time_t)(msec / 1000U),
		.tv_nsec = (long)((msec % 1000U) * 1000000U)
	};

	return tspec;
}

extern int
utime_msec_from_tspec(const struct timespec * __restrict tspec)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int
utime_msec_from_tspec_clamp(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	int msec = utime_msec_from_tspec(tspec);

	return (msec >= 0) ? msec : INT_MAX;
}

extern int
utime_tspec_add(struct timespec * __restrict       result,
                const struct timespec * __restrict amount)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow
void
utime_tspec_add_clamp(struct timespec * __restrict       result,
                      const struct timespec * __restrict amount)
{
	utime_assert_tspec_api(result);
	utime_assert_tspec_api(amount);
	utime_assert_api(result != amount);

	if (utime_tspec_add(result, amount) < 0)
		*result = UTIME_TSPEC_MAX;
}

extern int
utime_tspec_add_msec(struct timespec * __restrict result, unsigned int msec)
	__utils_nonull(1) __utils_nothrow __warn_result;

static inline __utils_nonull(1) __utils_nothrow
void
utime_tspec_add_msec_clamp(struct timespec * __restrict result,
                           unsigned int                 msec)
{
	utime_assert_tspec_api(result);
	utime_assert_api(msec <= INT_MAX);

	if (utime_tspec_add_msec(result, msec) < 0)
		*result = UTIME_TSPEC_MAX;
}

extern int
utime_tspec_add_sec(struct timespec * __restrict result, unsigned int sec)
	__utils_nonull(1) __utils_nothrow __leaf __warn_result;

static inline __utils_nonull(1) __utils_nothrow
void
utime_tspec_add_sec_clamp(struct timespec * __restrict result,
                          unsigned int                 sec)
{
	utime_assert_tspec_api(result);
	utime_assert_api(sec <= INT_MAX);

	if (utime_tspec_add_sec(result, sec) < 0)
		*result = UTIME_TSPEC_MAX;
}

extern int
utime_tspec_sub(struct timespec * __restrict       result,
                const struct timespec * __restrict amount)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

extern int
utime_tspec_sub_msec(struct timespec * __restrict result, unsigned int msec)
	__utils_nonull(1) __utils_nothrow __warn_result;

extern int
utime_tspec_sub_sec(struct timespec * __restrict result, unsigned int sec)
	__utils_nonull(1) __utils_nothrow __warn_result;

extern long
utime_tspec_diff_msec(const struct timespec * __restrict first,
                      const struct timespec * __restrict second)
	__utils_nonull(1, 2) __utils_pure __utils_nothrow __warn_result;

extern long
utime_tspec_diff_sec(const struct timespec * __restrict first,
                     const struct timespec * __restrict second)
	__utils_nonull(1, 2) __utils_pure __utils_nothrow __warn_result;

#if defined(CONFIG_UTILS_ASSERT_API)

static inline __utils_nonull(1, 2) __utils_nothrow
void
utime_gmtime_from_tspec(struct tm * __restrict             time,
                        const struct timespec * __restrict tspec)
{
	utime_assert_api(time);
	utime_assert_tspec_api(tspec);

	utime_assert_api(gmtime_r(&tspec->tv_sec, time));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(1, 2) __utils_nothrow
void
utime_gmtime_from_tspec(struct tm * __restrict             time,
                        const struct timespec * __restrict tspec)
{
	gmtime_r(&tspec->tv_sec, time);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#endif /* _UTILS_TIME_H */
