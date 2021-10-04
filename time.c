/**
 * @file      time.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2017
 * @copyright GNU Public License v3
 *
 * Time keeping implementation
 *
 * @defgroup time Time keeping
 *
 * This file is part of Utils
 *
 * Copyright (C) 2017 Grégor Boirie <gregor.boirie@free.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "utils/time.h"

int
utime_tspec_cmp(const struct timespec * __restrict fst,
                const struct timespec * __restrict snd)
{
	utime_assert_tspec(fst);
	utime_assert_tspec(snd);

	if (fst->tv_sec > snd->tv_sec)
		return 1;
	else if (fst->tv_sec < snd->tv_sec)
		return -1;
	else {
		if (fst->tv_nsec > snd->tv_nsec)
			return 1;
		if (fst->tv_nsec < snd->tv_nsec)
			return -1;
		else
			return 0;
	}
}

void
utime_tspec_add(struct timespec       * __restrict result,
                const struct timespec * __restrict amount)
{
	utime_assert_tspec(result);
	utime_assert_tspec(amount);
	utime_assert(amount->tv_sec >= 0);

	long nsec = result->tv_nsec + amount->tv_nsec;

	if (nsec >= 1000000000L) {
		result->tv_sec += amount->tv_sec + 1;
		result->tv_nsec = nsec - 1000000000L;
	}
	else {
		result->tv_sec += amount->tv_sec;
		result->tv_nsec = nsec;
	}
}

void
utime_tspec_add_msec(struct timespec * result, unsigned long msec)
{
	struct timespec tspec;

	tspec = utime_tspec_from_msec(msec);
	utime_tspec_add(result, &tspec);
}

void
utime_tspec_sub(struct timespec       * __restrict result,
                const struct timespec * __restrict amount)
{
	utime_assert_tspec(result);
	utime_assert_tspec(amount);
	utime_assert(amount->tv_sec >= 0);

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
utime_tspec_sub_msec(struct timespec * result, unsigned long msec)
{
	struct timespec tspec;

	tspec = utime_tspec_from_msec(msec);
	utime_tspec_sub(result, &tspec);
}

long
utime_tspec_diff_msec(const struct timespec * __restrict fst,
                      const struct timespec * __restrict snd)
{
	utime_assert(fst);

	struct timespec tmp = *fst;

	utime_tspec_sub(&tmp, snd);

	return utime_msec_from_tspec(&tmp);
}
