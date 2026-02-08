/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2026 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Pseudo random number generator interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      08 Feb 2026
 * @copyright Copyright (C) 2017-2026 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_PRNG_H
#define _UTILS_PRNG_H

#include <utils/cdefs.h>
#include <stdlib.h>

struct etux_prng {
	struct random_data buff;
	char               state[];
};

#define ETUX_PRNG0_TYPE 8
#define ETUX_PRNG1_TYPE 32
#define ETUX_PRNG2_TYPE 64
#define ETUX_PRNG3_TYPE 128
#define ETUX_PRNG4_TYPE 256

#define _ETUX_PRNG_DEFINE_TYPE(_prng_type) \
	struct STROLL_CONCAT(etux_prng, _prng_type) { \
		struct etux_prng base; \
		char             state[_prng_type]; \
	}

_ETUX_PRNG_DEFINE_TYPE(ETUX_PRNG0_TYPE);
_ETUX_PRNG_DEFINE_TYPE(ETUX_PRNG1_TYPE);
_ETUX_PRNG_DEFINE_TYPE(ETUX_PRNG2_TYPE);
_ETUX_PRNG_DEFINE_TYPE(ETUX_PRNG3_TYPE);
_ETUX_PRNG_DEFINE_TYPE(ETUX_PRNG4_TYPE);

#define ETUX_PRNG_DECLARE(_name, _prng_type) \
	struct STROLL_CONCAT(etux_prng, _prng_type) _name

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_prng_assert_api(_expr) \
	stroll_assert("etux:prng", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_prng_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_prng_assert_intern(_expr) \
	stroll_assert("etux:prng", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_prng_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

/*
 * Return a pseudo random integer in the [0, RAND_MAX] range.
 * RAND_MAX is the maximum positive integer, i.e., 2^31 - 1.
 */
static inline __utils_nonull(1) __utils_nothrow
int
_etux_prng_draw(struct etux_prng * __restrict prng)
{
	etux_prng_assert_api(prng);

	int32_t rnd;
	int     err;

	/*
	 * As stated into random(3), returned pseudo-random numbers are in the
	 * range 0 to RAND_MAX, i.e., 2^31 - 1, inclusive.
	 */
	err = random_r(&prng->buff, &rnd);
	etux_prng_assert_intern(!err);

	return (int)rnd;
}

#define etux_prng_draw(_prng) \
	_etux_prng_draw(&(_prng)->base)

/*
 * Return a pseudo random integer in the [0, 2^bits - 1] range where
 * 0 < bits <= 31.
 * Calling etux_prng_max_bits(prng, 31) is equivalent to calling
 * etux_prng_draw(prng).
 */
static inline __utils_nonull(1) __utils_nothrow
int
_etux_prng_draw_bits_max(struct etux_prng * __restrict prng,
                         unsigned int                  bits)
{
	etux_prng_assert_api(prng);
	etux_prng_assert_api(bits);
	etux_prng_assert_api(bits <= 31);

	return _etux_prng_draw(prng) >> (31 - bits);
}

#define etux_prng_draw_bits_max(_prng, _bits) \
	_etux_prng_draw_bits_max(&(_prng)->base, _bits)

/*
 * Return a pseudo random integer in the [0, high] range where
 * 0 < high <= RAND_MAX.
 * RAND_MAX is the maximum positive integer, i.e., 2^31 - 1.
 * Calling etux_prng_draw_max(prng, RAND_MAX) is equivalent to calling
 * etux_prng_draw(prng).
 */
extern int
_etux_prng_draw_max(struct etux_prng * __restrict prng, int high)
	__utils_nonull(1) __utils_nothrow __leaf;

#define etux_prng_draw_max(_prng, _high) \
	_etux_prng_draw_max(&(_prng)->base, _high)

/*
 * Return a pseudo random integer between [low, high] where:
 *     INT_MIN <= low < RAND_MAX, and
 *     INT_MIN < high <= RAND_MAX, and
 *     0 < high - low <= RAND_MAX
 * INT_MIN is the smallest negative integer, i.e., - 2^31.
 * RAND_MAX is the maximum positive integer, i.e., 2^31 - 1.
 * Calling etux_prng_draw_range(prng, 0, RAND_MAX) is equivalent to calling
 * etux_prng_draw(prng) or etux_prng_draw_max(prng, RAND_MAX).
 */
extern int
_etux_prng_draw_range(struct etux_prng * __restrict prng,
                      int                           low,
                      int                           high)
	__utils_nonull(1) __utils_nothrow;

#define etux_prng_draw_range(_prng, _low, _high) \
	_etux_prng_draw_range(&(_prng)->base, _low, _high)

/*
 * Initialize pseudo-random integers generator logic with the given seed.
 * Should be called once per thread !
 */
static inline __utils_nonull(1) __utils_nothrow
void
_etux_prng_initn_seed(struct etux_prng * __restrict prng,
                      size_t                        type,
                      unsigned int                  seed)
{
	etux_prng_assert_api(prng);
	etux_prng_assert_api((type == ETUX_PRNG0_TYPE) ||
	                     (type == ETUX_PRNG1_TYPE) ||
	                     (type == ETUX_PRNG2_TYPE) ||
	                     (type == ETUX_PRNG3_TYPE) ||
	                     (type == ETUX_PRNG4_TYPE));

	int err;

	/* Setup internal buffer state to NULL as required by random(3). */
	prng->buff.state = NULL;

	err = initstate_r(seed, prng->state, type, &prng->buff);
	etux_prng_assert_intern(!err);
}

#define etux_prng_initn_seed(_prng, _seed) \
	({ \
		struct etux_prng * __prng = &(_prng)->base; \
		\
		_etux_prng_initn_seed(__prng, sizeof((_prng)->state), _seed); \
	 })

/*
 * Initialize pseudo-random integers generator and automatically seed it.
 * Should be called once per thread !
 */
extern void
_etux_prng_init(struct etux_prng * __restrict prng, size_t type)
	__utils_nonull(1) __utils_nothrow __leaf;

#define etux_prng_init(_prng) \
	({ \
		struct etux_prng * __prng = &(_prng)->base; \
		\
		_etux_prng_init(__prng, sizeof((_prng)->state)); \
	 })

#endif /* _UTILS_PRNG_H */
