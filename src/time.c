/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/time.h"

int
utime_tspec_cmp(const struct timespec * __restrict fst,
                const struct timespec * __restrict snd)
{
	utime_assert_tspec(fst);
	utime_assert_tspec(snd);
	utime_assert_api(fst != snd);

	if (fst->tv_sec > snd->tv_sec)
		return 1;
	else if (fst->tv_sec < snd->tv_sec)
		return -1;

	if (fst->tv_nsec > snd->tv_nsec)
		return 1;
	else if (fst->tv_nsec < snd->tv_nsec)
		return -1;
	else
		return 0;
}

void
utime_tspec_add(struct timespec * __restrict       result,
                const struct timespec * __restrict amount)
{
	utime_assert_tspec(result);
	utime_assert_tspec(amount);
	utime_assert_api(result != amount);

	unsigned long nsec = (unsigned long)result->tv_nsec +
	                     (unsigned long)amount->tv_nsec;

	utime_assert_api(((unsigned long)result->tv_sec +
	                  (unsigned long)amount->tv_sec +
	                  (nsec / 1000000000UL)) <= (unsigned long)LONG_MAX);

	if (nsec >= 1000000000UL) {
		result->tv_sec += amount->tv_sec + 1;
		result->tv_nsec = (long)(nsec - 1000000000UL);
	}
	else {
		result->tv_sec += amount->tv_sec;
		result->tv_nsec = (long)nsec;
	}
}

void
utime_tspec_add_msec(struct timespec * __restrict result, unsigned long msec)
{
	struct timespec tspec;

	tspec = utime_tspec_from_msec(msec);
	utime_tspec_add(result, &tspec);
}

void
utime_tspec_sub(struct timespec * __restrict       result,
                const struct timespec * __restrict amount)
{
	utime_assert_tspec(result);
	utime_assert_tspec(amount);
	utime_assert_api(amount->tv_sec >= 0);

	long nsec = result->tv_nsec - amount->tv_nsec;

	if (nsec < 0) {
		result->tv_sec -= amount->tv_sec + 1;
		result->tv_nsec = nsec + 1000000000L;
	}
	else {
		result->tv_sec -= amount->tv_sec;
		result->tv_nsec = nsec;
	}
}

void
utime_tspec_sub_msec(struct timespec * __restrict result, unsigned long msec)
{
	struct timespec tspec;

	tspec = utime_tspec_from_msec(msec);
	utime_tspec_sub(result, &tspec);
}

long
utime_tspec_diff_msec(const struct timespec * __restrict fst,
                      const struct timespec * __restrict snd)
{
	struct timespec tmp = *fst;

	utime_tspec_sub(&tmp, snd);

	return utime_msec_from_tspec(&tmp);
}
