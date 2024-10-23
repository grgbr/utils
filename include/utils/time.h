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

static inline __nonull(1) __utils_nothrow
void
utime_realtime_now(struct timespec * now)
{
	clock_gettime(CLOCK_REALTIME, now);
}

static inline __nonull(1) __utils_nothrow
void
utime_monotonic_now(struct timespec * now)
{
	clock_gettime(CLOCK_MONOTONIC, now);
}

static inline __nonull(1) __utils_nothrow
void
utime_coarse_now(struct timespec * now)
{
	clock_gettime(CLOCK_MONOTONIC_COARSE, now);
}

static inline __nonull(1) __utils_nothrow
void
utime_boot_now(struct timespec * now)
{
	clock_gettime(CLOCK_BOOTTIME, now);
}

static inline __nonull(1) __utils_nothrow
void
utime_proc_now(struct timespec * now)
{
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, now);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#define utime_assert_tspec(_tspec) \
	utime_assert_api(_tspec); \
	utime_assert_api((_tspec)->tv_nsec >= 0); \
	utime_assert_api((_tspec)->tv_nsec < 1000000000L)

extern int
utime_tspec_cmp(const struct timespec * __restrict fst,
                const struct timespec * __restrict snd)
	__utils_nonull(1, 2) __utils_pure __utils_nothrow __leaf;

static inline __utils_nonull(1, 2) __utils_pure __utils_nothrow
bool
utime_tspec_after(const struct timespec * __restrict fst,
                  const struct timespec * __restrict snd)
{
	return (utime_tspec_cmp(fst, snd) > 0);
}

static inline __utils_nonull(1, 2) __utils_pure __utils_nothrow
bool
utime_tspec_after_eq(const struct timespec * __restrict fst,
                     const struct timespec * __restrict snd)
{
	return (utime_tspec_cmp(fst, snd) >= 0);
}

static inline __utils_nonull(1, 2) __utils_pure __utils_nothrow
bool
utime_tspec_before(const struct timespec * __restrict fst,
                   const struct timespec * __restrict snd)
{
	return (utime_tspec_cmp(fst, snd) < 0);
}

static inline __utils_nonull(1, 2) __utils_pure __utils_nothrow
bool
utime_tspec_before_eq(const struct timespec * __restrict fst,
                      const struct timespec * __restrict snd)
{
	return (utime_tspec_cmp(fst, snd) <= 0);
}

static inline __stroll_const __utils_nothrow
struct timespec
utime_tspec_from_msec(unsigned long msec)
{
	utime_assert_api(msec <= LONG_MAX);

	ldiv_t          res;
	struct timespec tspec;

	res = ldiv((long)msec, 1000L);

	tspec.tv_sec = (time_t)res.quot;
	tspec.tv_nsec = (long)res.rem;

	return tspec;
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow
long
utime_msec_from_tspec(const struct timespec * __restrict tspec)
{
	return (tspec->tv_sec * 1000L) + (tspec->tv_nsec / 1000000L);
}

static inline __utils_nonull(1) __utils_nothrow
void
utime_tspec_add_sec(struct timespec * __restrict result, unsigned long sec)
{
	utime_assert_api(result);

	result->tv_sec += (time_t)sec;
}

extern void
utime_tspec_add(struct timespec * __restrict       result,
                const struct timespec * __restrict amount)
	__utils_nonull(1, 2) __utils_nothrow __leaf;

extern void
utime_tspec_add_msec(struct timespec * __restrict result, unsigned long msec)
	__utils_nonull(1) __utils_nothrow;

extern void
utime_tspec_sub(struct timespec * __restrict       result,
                const struct timespec * __restrict amount)
	__utils_nonull(1, 2) __utils_nothrow __leaf;

extern void
utime_tspec_sub_msec(struct timespec * __restrict result, unsigned long msec)
	__utils_nonull(1) __utils_nothrow;

static inline __utils_nonull(1) __utils_nothrow
void
utime_tspec_sub_sec(struct timespec * __restrict result, unsigned long sec)
{
	utime_assert_api(result);
	utime_assert_api(sec);

	result->tv_sec -= (time_t)sec;
}

extern long
utime_tspec_diff_msec(const struct timespec * __restrict fst,
                      const struct timespec * __restrict snd)
	__utils_nonull(1, 2) __utils_pure __utils_nothrow;

#if defined(CONFIG_UTILS_ASSERT_API)

static inline __utils_nonull(1, 2) __utils_nothrow
void
utime_gmtime_from_tspec(struct tm * __restrict             time,
                        const struct timespec * __restrict tspec)
{
	utime_assert_api(time);
	utime_assert_api(tspec);

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
