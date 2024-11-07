/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/time.h"
#include <errno.h>

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#define utime_assert_intern(_expr) \
	stroll_assert("utils:utime", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define utime_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

#define utime_assert_tspec_intern(_tspec) \
	utime_assert_intern(_tspec); \
	utime_assert_intern((_tspec)->tv_sec >= 0); \
	utime_assert_intern((_tspec)->tv_nsec >= 0); \
	utime_assert_intern((_tspec)->tv_nsec < 1000000000L)

int
utime_tspec_cmp(const struct timespec * __restrict first,
                const struct timespec * __restrict second)
{
	utime_assert_tspec_api(first);
	utime_assert_tspec_api(second);
	utime_assert_api(first != second);

	if (first->tv_sec > second->tv_sec)
		return 1;
	else if (first->tv_sec < second->tv_sec)
		return -1;

	if (first->tv_nsec > second->tv_nsec)
		return 1;
	else if (first->tv_nsec < second->tv_nsec)
		return -1;
	else
		return 0;
}

int
utime_msec_from_tspec(const struct timespec * __restrict tspec)
{
	utime_assert_tspec_api(tspec);

	int msec;

	if (!__builtin_mul_overflow(tspec->tv_sec, 1000, &msec) &&
	    !__builtin_sadd_overflow(msec,
	                             (int)tspec->tv_nsec / 1000000,
	                             &msec))
		return msec;

	return -ERANGE;
}

#if UTIME_TIMET_BITS == 64
#	if __WORDSIZE == 64

static inline __nonull(3) __nothrow __warn_result
bool
utime_timet_add_overflow(time_t a, time_t b, time_t * __restrict res)
{
	return __builtin_saddl_overflow((long)a, (long)b, (long *)res);
}

#	elif __WORDSIZE == 32

static inline __nonull(3) __nothrow __warn_result
bool
utime_timet_add_overflow(time_t a, time_t b, time_t * __restrict res)
{
	return __builtin_saddll_overflow((long long)a,
	                                 (long long)b,
	                                 (long long *)res);
}

#	else
#		error "Unsupported machine word size !"
#	endif
#elif UTIME_TIMET_BITS == 32

static inline __nonull(3) __nothrow __warn_result
bool
utime_timet_add_overflow(time_t a, time_t b, time_t * __restrict res)
{
	return __builtin_sadd_overflow((int)a, (int)b, (int *)res);
}

#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

int
utime_tspec_add(struct timespec * __restrict       result,
                const struct timespec * __restrict amount)
{
	utime_assert_tspec_api(result);
	utime_assert_tspec_api(amount);
	utime_assert_api(result != amount);

	long   nsec = result->tv_nsec + amount->tv_nsec;
	time_t sec = amount->tv_sec;

	if (nsec >= 1000000000L) {
		utime_assert_intern(nsec <= 1999999998L);

		if (sec == UTIME_TIMET_MAX)
			return -ERANGE;

		sec++;
		nsec -= 1000000000L;
	}

	if (utime_timet_add_overflow(result->tv_sec, sec, &sec))
		return -ERANGE;

	result->tv_sec = sec;
	result->tv_nsec = nsec;

	return 0;
}

int
utime_tspec_add_msec(struct timespec * __restrict result, unsigned int msec)
{
	utime_assert_tspec_api(result);
	utime_assert_api(msec <= INT_MAX);

	const struct timespec amount = utime_tspec_from_msec(msec);

	return utime_tspec_add(result, &amount);
}

int
utime_tspec_add_sec(struct timespec * __restrict result, unsigned int sec)
{
	utime_assert_tspec_api(result);
	utime_assert_api(sec <= INT_MAX);

	if (!utime_timet_add_overflow(result->tv_sec, sec, &result->tv_sec))
		return 0;

	return -ERANGE;
}

static inline __utils_nonull(1, 2) __utils_const __utils_nothrow __warn_result
struct timespec
utime_tspec_absdiff(const struct timespec * __restrict higher,
                    const struct timespec * __restrict lower)
{
	utime_assert_tspec_intern(higher);
	utime_assert_tspec_intern(lower);
	utime_assert_intern(higher != lower);
	utime_assert_intern(utime_tspec_cmp(higher, lower) >= 0);

	long            nsec = higher->tv_nsec - lower->tv_nsec;
	struct timespec result;

	if (nsec < 0) {
		result.tv_sec = higher->tv_sec - lower->tv_sec - 1;
		result.tv_nsec = nsec + 1000000000L;
	}
	else {
		result.tv_sec = higher->tv_sec - lower->tv_sec;
		result.tv_nsec = nsec;
	}

	return result;
}

int
utime_tspec_sub(struct timespec * __restrict       result,
                const struct timespec * __restrict amount)
{
	utime_assert_tspec_api(result);
	utime_assert_tspec_api(amount);
	utime_assert_api(result != amount);

	if (result->tv_sec > amount->tv_sec)
		goto positive;
	else if (result->tv_sec < amount->tv_sec)
		goto negative;

	if (result->tv_nsec > amount->tv_nsec)
		goto positive;
	else if (result->tv_nsec < amount->tv_nsec)
		goto negative;

	result->tv_sec = 0;
	result->tv_nsec = 0;

	return 0;

positive:
	*result = utime_tspec_absdiff(result, amount);

	return 1;

negative:
	*result = utime_tspec_absdiff(amount, result);

	return -1;
}

int
utime_tspec_sub_msec(struct timespec * __restrict result, unsigned int msec)
{
	utime_assert_tspec_api(result);
	utime_assert_api(msec <= INT_MAX);

	const struct timespec amount = utime_tspec_from_msec(msec);

	return utime_tspec_sub(result, &amount);
}

int
utime_tspec_sub_sec(struct timespec * __restrict result, unsigned int sec)
{
	utime_assert_tspec_api(result);
	utime_assert_api(sec <= INT_MAX);

	const struct timespec amount = {
		.tv_sec  = (long)sec,
		.tv_nsec = 0
	};

	return utime_tspec_sub(result, &amount);
}

static inline __utils_nonull(1, 2, 3) __utils_nothrow __warn_result
int
utime_tspec_diff(struct timespec * __restrict       result,
                 const struct timespec * __restrict first,
                 const struct timespec * __restrict second)
{
	utime_assert_intern(result);
	utime_assert_tspec_intern(first);
	utime_assert_tspec_intern(second);
	utime_assert_intern(result != first);
	utime_assert_intern(result != second);
	utime_assert_intern(first != second);

	if (first->tv_sec > second->tv_sec)
		goto positive;
	else if (first->tv_sec < second->tv_sec)
		goto negative;

	if (first->tv_nsec > second->tv_nsec)
		goto positive;
	else if (first->tv_nsec < second->tv_nsec)
		goto negative;

	return 0;

positive:
	*result = utime_tspec_absdiff(first, second);

	return 1;

negative:
	*result = utime_tspec_absdiff(second, first);

	return -1;
}

long
utime_tspec_diff_msec(const struct timespec * __restrict first,
                      const struct timespec * __restrict second)
{
	utime_assert_tspec_api(first);
	utime_assert_tspec_api(second);
	utime_assert_api(first != second);

	struct timespec diff;

	switch (utime_tspec_diff(&diff, first, second)) {
	case 1:
		return utime_msec_from_tspec(&diff);
	case -1:
		return 0 - utime_msec_from_tspec(&diff);
	case 0:
		return 0;
	default:
		utime_assert_intern(0);
	}

	unreachable();
}

long
utime_tspec_diff_sec(const struct timespec * __restrict first,
                     const struct timespec * __restrict second)
{
	utime_assert_tspec_api(first);
	utime_assert_tspec_api(second);
	utime_assert_api(first != second);

	struct timespec diff;

	switch (utime_tspec_diff(&diff, first, second)) {
	case 1:
		return diff.tv_sec;
	case -1:
		return 0 - diff.tv_sec;
	case 0:
		return 0;
	default:
		utime_assert_intern(0);
	}

	unreachable();
}
