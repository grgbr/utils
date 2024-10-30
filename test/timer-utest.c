/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/timer.h"
#include "utest.h"

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utimer_tick_from_tspec_lower_assert)
{
	const struct timespec sec_neg = {
		.tv_sec  = -1,
		.tv_nsec = 0
	};
	const struct timespec nsec_over = {
		.tv_sec  = 0,
		.tv_nsec = 1000000000L
	};
	const struct timespec nsec_neg = {
		.tv_sec  = 0,
		.tv_nsec = -1
	};
	uint64_t              tick __unused;

	cute_expect_assertion(tick = utimer_tick_from_tspec_lower(NULL));
	cute_expect_assertion(tick = utimer_tick_from_tspec_lower(&sec_neg));
	cute_expect_assertion(tick = utimer_tick_from_tspec_lower(&nsec_over));
	cute_expect_assertion(tick = utimer_tick_from_tspec_lower(&nsec_neg));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utimer_tick_from_tspec_lower_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static const struct timespec utilsut_utimer_tspecs[] = {
	{ .tv_sec = 0,        .tv_nsec = 0 },
	{ .tv_sec = 0,        .tv_nsec = 1 },
	{ .tv_sec = 0,        .tv_nsec = UTIMER_TICK_NSEC - 1 },
#if UTIMER_TICK_SUBSEC_BITS > 0
	{ .tv_sec = 0,        .tv_nsec = UTIMER_TICK_NSEC },
	{ .tv_sec = 0,        .tv_nsec = UTIMER_TICK_NSEC + 1 },
#endif /* UTIMER_TICK_SUBSEC_BITS > 0 */
	{ .tv_sec = 0,        .tv_nsec = (UTIMER_TICKS_PER_SEC / 2) *
	                                 UTIMER_TICK_NSEC },
	{ .tv_sec = 0,        .tv_nsec = (UTIMER_TICKS_PER_SEC - 1) *
	                                 UTIMER_TICK_NSEC },
	{ .tv_sec = 0,        .tv_nsec = (UTIMER_TICKS_PER_SEC *
	                                  UTIMER_TICK_NSEC) - 1 },

	{ .tv_sec = 1,        .tv_nsec = 0 },
	{ .tv_sec = 1,        .tv_nsec = 1 },
	{ .tv_sec = 1,        .tv_nsec = UTIMER_TICK_NSEC - 1 },
#if UTIMER_TICK_SUBSEC_BITS > 0
	{ .tv_sec = 1,        .tv_nsec = UTIMER_TICK_NSEC },
	{ .tv_sec = 1,        .tv_nsec = UTIMER_TICK_NSEC + 1 },
#endif /* UTIMER_TICK_SUBSEC_BITS > 0 */
	{ .tv_sec = 1,        .tv_nsec = (UTIMER_TICKS_PER_SEC / 2) *
	                                 UTIMER_TICK_NSEC },
	{ .tv_sec = 1,        .tv_nsec = (UTIMER_TICKS_PER_SEC - 1) *
	                                 UTIMER_TICK_NSEC },
	{ .tv_sec = 1,        .tv_nsec = (UTIMER_TICKS_PER_SEC *
	                                  UTIMER_TICK_NSEC) - 1 },

	{ .tv_sec = 1000,     .tv_nsec = 0 },
	{ .tv_sec = 1000,     .tv_nsec = 1 },
	{ .tv_sec = 1000,     .tv_nsec = UTIMER_TICK_NSEC - 1 },
#if UTIMER_TICK_SUBSEC_BITS > 0
	{ .tv_sec = 1000,     .tv_nsec = UTIMER_TICK_NSEC },
	{ .tv_sec = 1000,     .tv_nsec = UTIMER_TICK_NSEC + 1 },
#endif /* UTIMER_TICK_SUBSEC_BITS > 0 */
	{ .tv_sec = 1000,     .tv_nsec = (UTIMER_TICKS_PER_SEC / 2) *
	                                 UTIMER_TICK_NSEC },
	{ .tv_sec = 1000,     .tv_nsec = (UTIMER_TICKS_PER_SEC - 1) *
	                                 UTIMER_TICK_NSEC },
	{ .tv_sec = 1000,     .tv_nsec = (UTIMER_TICKS_PER_SEC *
	                                  UTIMER_TICK_NSEC) - 1 },

	{ .tv_sec = LONG_MAX, .tv_nsec = 0 },
	{ .tv_sec = LONG_MAX, .tv_nsec = 1 },
	{ .tv_sec = LONG_MAX, .tv_nsec = UTIMER_TICK_NSEC - 1 },
#if UTIMER_TICK_SUBSEC_BITS > 0
	{ .tv_sec = LONG_MAX, .tv_nsec = UTIMER_TICK_NSEC },
	{ .tv_sec = LONG_MAX, .tv_nsec = UTIMER_TICK_NSEC + 1 },
#endif /* UTIMER_TICK_SUBSEC_BITS > 0 */
	{ .tv_sec = LONG_MAX, .tv_nsec = (UTIMER_TICKS_PER_SEC / 2) *
	                                 UTIMER_TICK_NSEC },
	{ .tv_sec = LONG_MAX, .tv_nsec = (UTIMER_TICKS_PER_SEC - 1) *
	                                 UTIMER_TICK_NSEC },
	{ .tv_sec = LONG_MAX, .tv_nsec = (UTIMER_TICKS_PER_SEC *
	                                  UTIMER_TICK_NSEC) - 1 },
};

#define UTILSUT_TSPEC2TICKS_LOWER(_tspec) \
	(((uint64_t)((_tspec)->tv_sec) * UTIMER_TICKS_PER_SEC) + \
	 ((uint64_t)((_tspec)->tv_nsec) / UTIMER_TICK_NSEC))

CUTE_TEST(utilsut_utimer_tick_from_tspec_lower)
{
	unsigned int s;

	for (s = 0; s < stroll_array_nr(utilsut_utimer_tspecs); s++) {
		uint64_t tick;

		tick = utimer_tick_from_tspec_lower(&utilsut_utimer_tspecs[s]);
		cute_check_uint(
			tick,
			equal,
			UTILSUT_TSPEC2TICKS_LOWER(&utilsut_utimer_tspecs[s]));
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utimer_tick_from_tspec_upper_assert)
{
	const struct timespec sec_neg = {
		.tv_sec  = -1,
		.tv_nsec = 0
	};
	const struct timespec nsec_over = {
		.tv_sec  = 0,
		.tv_nsec = 1000000000L
	};
	const struct timespec nsec_neg = {
		.tv_sec  = 0,
		.tv_nsec = -1
	};
	uint64_t              tick __unused;

	cute_expect_assertion(tick = utimer_tick_from_tspec_upper(NULL));
	cute_expect_assertion(tick = utimer_tick_from_tspec_upper(&sec_neg));
	cute_expect_assertion(tick = utimer_tick_from_tspec_upper(&nsec_over));
	cute_expect_assertion(tick = utimer_tick_from_tspec_upper(&nsec_neg));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utimer_tick_from_tspec_upper_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#define UTILSUT_TSPEC2TICKS_UPPER(_tspec) \
	(((uint64_t)((_tspec)->tv_sec) * UTIMER_TICKS_PER_SEC) + \
	 (((uint64_t)((_tspec)->tv_nsec) + UTIMER_TICK_NSEC - 1) / \
	  UTIMER_TICK_NSEC))

CUTE_TEST(utilsut_utimer_tick_from_tspec_upper)
{
	unsigned int s;

	for (s = 0; s < stroll_array_nr(utilsut_utimer_tspecs); s++) {
		uint64_t tick;

		tick = utimer_tick_from_tspec_upper(&utilsut_utimer_tspecs[s]);
		cute_check_uint(
			tick,
			equal,
			UTILSUT_TSPEC2TICKS_UPPER(&utilsut_utimer_tspecs[s]));
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utimer_tspec_from_tick_assert)
{
	struct timespec tspec;

	cute_expect_assertion(utimer_tspec_from_tick(UTIMER_TICK_MAX + 1,
	                                             &tspec));
	cute_expect_assertion(utimer_tspec_from_tick(0, NULL));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utimer_tspec_from_tick_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utimer_tspec_from_tick)
{
	const uint64_t ticks[] = {
		0,
		1,
		UTIMER_TICKS_PER_SEC - 1,
		UTIMER_TICKS_PER_SEC,
		UTIMER_TICKS_PER_SEC + 1,
		2 * UTIMER_TICKS_PER_SEC - 1,
		2 * UTIMER_TICKS_PER_SEC,
		2 * UTIMER_TICKS_PER_SEC + 1,
		(UTIMER_TICK_MAX / 2) - 1,
		UTIMER_TICK_MAX / 2,
		(UTIMER_TICK_MAX / 2) + 1,
		UTIMER_TICK_MAX - 1,
		UTIMER_TICK_MAX,
	};
	unsigned int   t;

	for (t = 0; t < stroll_array_nr(ticks); t++) {
		struct timespec tspec = { 0, };

		utimer_tspec_from_tick(ticks[t], &tspec);

		cute_check_sint(tspec.tv_sec,
		                equal,
		                (time_t)(ticks[t] / UTIMER_TICKS_PER_SEC));
		cute_check_sint(tspec.tv_nsec,
		                equal,
		                (long)((ticks[t] % UTIMER_TICKS_PER_SEC) *
		                       UTIMER_TICK_NSEC));
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utimer_msec_from_tick_lower_assert)
{
	cute_expect_assertion(
		utimer_msec_from_tick_lower(UTIMER_TICK_MSEC_MAX + 1));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utimer_msec_from_tick_lower_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utimer_msec_from_tick_lower)
{
	const uint64_t ticks[] = {
		0,
		1,
		UTIMER_TICKS_PER_SEC / 2,
		UTIMER_TICKS_PER_SEC - 1,
		UTIMER_TICKS_PER_SEC,
		UTIMER_TICKS_PER_SEC + 1,
		2 * UTIMER_TICKS_PER_SEC - 1,
		2 * UTIMER_TICKS_PER_SEC,
		2 * UTIMER_TICKS_PER_SEC + 1,
		(UTIMER_TICK_MSEC_MAX / 2) - 1,
		UTIMER_TICK_MSEC_MAX / 2,
		(UTIMER_TICK_MSEC_MAX / 2) + 1,
		UTIMER_TICK_MSEC_MAX - 1,
		UTIMER_TICK_MSEC_MAX
	};
	unsigned int   t;

	for (t = 0; t < stroll_array_nr(ticks); t++) {
		unsigned long msecs;

		msecs = utimer_msec_from_tick_lower(ticks[t]);

		cute_check_uint(msecs,
		                equal,
		                ((ticks[t] / UTIMER_TICKS_PER_SEC) *
		                 UINT64_C(1000)) +
		                ((ticks[t] % UTIMER_TICKS_PER_SEC) *
		                 UTIMER_TICK_NSEC) / UINT64_C(1000000));
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utimer_msec_from_tick_upper_assert)
{
	cute_expect_assertion(
		utimer_msec_from_tick_upper(UTIMER_TICK_MSEC_MAX + 1));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utimer_msec_from_tick_upper_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utimer_msec_from_tick_upper)
{
	const uint64_t ticks[] = {
		0,
		1,
		UTIMER_TICKS_PER_SEC / 2,
		UTIMER_TICKS_PER_SEC - 1,
		UTIMER_TICKS_PER_SEC,
		UTIMER_TICKS_PER_SEC + 1,
		2 * UTIMER_TICKS_PER_SEC - 1,
		2 * UTIMER_TICKS_PER_SEC,
		2 * UTIMER_TICKS_PER_SEC + 1,
		(UTIMER_TICK_MSEC_MAX / 2) - 1,
		UTIMER_TICK_MSEC_MAX / 2,
		(UTIMER_TICK_MSEC_MAX / 2) + 1,
		UTIMER_TICK_MSEC_MAX - 1,
		UTIMER_TICK_MSEC_MAX
	};
	unsigned int   t;

	for (t = 0; t < stroll_array_nr(ticks); t++) {
		unsigned long msecs;

		msecs = utimer_msec_from_tick_upper(ticks[t]);

		cute_check_uint(msecs,
		                equal,
		                ((ticks[t] / UTIMER_TICKS_PER_SEC) *
		                 UINT64_C(1000)) +
		                (((ticks[t] %
		                   UTIMER_TICKS_PER_SEC) * UTIMER_TICK_NSEC) +
		                 UINT64_C(1000000) - 1) /
		                UINT64_C(1000000));
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utimer_sec_from_tick_lower_assert)
{
	cute_expect_assertion(
		utimer_sec_from_tick_lower(UTIMER_TICK_SEC_MAX + 1));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utimer_sec_from_tick_lower_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utimer_sec_from_tick_lower)
{
	const uint64_t ticks[] = {
		0,
		1,
		UTIMER_TICKS_PER_SEC / 2,
		UTIMER_TICKS_PER_SEC - 1,
		UTIMER_TICKS_PER_SEC,
		UTIMER_TICKS_PER_SEC + 1,
		2 * UTIMER_TICKS_PER_SEC - 1,
		2 * UTIMER_TICKS_PER_SEC,
		2 * UTIMER_TICKS_PER_SEC + 1,
		(UTIMER_TICK_SEC_MAX / 2) - 1,
		UTIMER_TICK_SEC_MAX / 2,
		(UTIMER_TICK_SEC_MAX / 2) + 1,
		UTIMER_TICK_SEC_MAX - 1,
		UTIMER_TICK_SEC_MAX
	};
	unsigned int   t;

	for (t = 0; t < stroll_array_nr(ticks); t++) {
		unsigned long secs;

		secs = utimer_sec_from_tick_lower(ticks[t]);

		cute_check_uint(secs, equal, ticks[t] / UTIMER_TICKS_PER_SEC);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utimer_sec_from_tick_upper_assert)
{
	cute_expect_assertion(
		utimer_sec_from_tick_upper(UTIMER_TICK_SEC_MAX + 1));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utimer_sec_from_tick_upper_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utimer_sec_from_tick_upper)
{
	const uint64_t ticks[] = {
		0,
		1,
		UTIMER_TICKS_PER_SEC / 2,
		UTIMER_TICKS_PER_SEC - 1,
		UTIMER_TICKS_PER_SEC,
		UTIMER_TICKS_PER_SEC + 1,
		2 * UTIMER_TICKS_PER_SEC - 1,
		2 * UTIMER_TICKS_PER_SEC,
		2 * UTIMER_TICKS_PER_SEC + 1,
		(UTIMER_TICK_SEC_MAX / 2) - 1,
		UTIMER_TICK_SEC_MAX / 2,
		(UTIMER_TICK_SEC_MAX / 2) + 1,
		UTIMER_TICK_SEC_MAX - 1,
		UTIMER_TICK_SEC_MAX
	};
	unsigned int   t;

	for (t = 0; t < stroll_array_nr(ticks); t++) {
		unsigned long secs;

		secs = utimer_sec_from_tick_upper(ticks[t]);

		cute_check_uint(secs,
		                equal,
		                (ticks[t] + UTIMER_TICKS_PER_SEC - 1) /
		                UTIMER_TICKS_PER_SEC);
	}
}

CUTE_GROUP(utilsut_timer_group) = {
	CUTE_REF(utilsut_utimer_tick_from_tspec_lower_assert),
	CUTE_REF(utilsut_utimer_tick_from_tspec_lower),
	CUTE_REF(utilsut_utimer_tick_from_tspec_upper_assert),
	CUTE_REF(utilsut_utimer_tick_from_tspec_upper),
	CUTE_REF(utilsut_utimer_tspec_from_tick_assert),
	CUTE_REF(utilsut_utimer_tspec_from_tick),
	CUTE_REF(utilsut_utimer_msec_from_tick_lower_assert),
	CUTE_REF(utilsut_utimer_msec_from_tick_lower),
	CUTE_REF(utilsut_utimer_msec_from_tick_upper_assert),
	CUTE_REF(utilsut_utimer_msec_from_tick_upper),
	CUTE_REF(utilsut_utimer_sec_from_tick_lower_assert),
	CUTE_REF(utilsut_utimer_sec_from_tick_lower),
	CUTE_REF(utilsut_utimer_sec_from_tick_upper_assert),
	CUTE_REF(utilsut_utimer_sec_from_tick_upper),
};

CUTE_SUITE_EXTERN(utilsut_timer_suite,
                  utilsut_timer_group,
                  CUTE_NULL_SETUP,
                  CUTE_NULL_TEARDOWN,
                  CUTE_DFLT_TMOUT);
