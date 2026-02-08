/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2026 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/prng.h"
#include "utils/time.h"
#include <unistd.h>

int
_etux_prng_draw_max(struct etux_prng * __restrict prng, int high)
{
	etux_prng_assert_api(high);
	etux_prng_assert_api(high <= RAND_MAX);

	int rnd;

	rnd = (int)((unsigned int)_etux_prng_draw(prng) %
	            ((unsigned int)high + 1));
	etux_prng_assert_intern(rnd >= 0);
	etux_prng_assert_intern(rnd <= high);

	return rnd;
}

int
_etux_prng_draw_range(struct etux_prng * __restrict prng, int low, int high)
{
	etux_prng_assert_api(prng);
	etux_prng_assert_api(low < RAND_MAX);
	etux_prng_assert_api(high > INT_MIN);
	etux_prng_assert_api(high > low);
	etux_prng_assert_api((high - low) <= RAND_MAX);

	int rnd;

	rnd = low + _etux_prng_draw_max(prng, high - low);
	etux_prng_assert_intern(rnd >= low);
	etux_prng_assert_intern(rnd <= high);

	return rnd;
}

#if UTIME_TIMET_BITS == 64

static __utils_const __utils_nothrow
unsigned int
etux_prng_timet_seed(unsigned int seed, time_t duration)
{
	seed ^= (unsigned int)(duration >> 32);

	return seed ^ (unsigned int)(duration & UINT_MAX);
}

#elif UTIME_TIMET_BITS == 32

static __utils_const __utils_nothrow
unsigned int
etux_prng_timet_seed(unsigned int seed, time_t duration)
{
	return seed ^ (unsigned int)duration;
}

#else
#error Unsupported time_t bit width !
#endif

static __utils_nonull(2) __utils_pure __utils_nothrow
unsigned int
etux_prng_tspec_seed(unsigned int                       seed,
                     const struct timespec * __restrict tspec)
{
	etux_prng_assert_api(tspec);

	seed = etux_prng_timet_seed(seed, tspec->tv_sec);

	return seed ^ (unsigned int)tspec->tv_nsec;
}

void
_etux_prng_init(struct etux_prng * __restrict prng, size_t type)
{
	etux_prng_assert_api(prng);
	etux_prng_assert_api((type == ETUX_PRNG0_TYPE) ||
	                     (type == ETUX_PRNG1_TYPE) ||
	                     (type == ETUX_PRNG2_TYPE) ||
	                     (type == ETUX_PRNG3_TYPE) ||
	                     (type == ETUX_PRNG4_TYPE));

	struct timespec tspec;
	unsigned int    seed;

	utime_tai_now(&tspec);
	seed = etux_prng_tspec_seed(0, &tspec);
	seed ^= (unsigned int)(gethostid() & UINT_MAX);
	seed ^= (unsigned int)getuid();
	seed ^= (unsigned int)gettid();

	_etux_prng_initn_seed(prng, type, seed);
}
