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

/* Just make sure that clock_gettime() syscall mocking operates properly. */
CUTE_TEST(utilsut_utimer_monotonic_now)
{
	struct timespec now = { 0, };

	/* Schedule next timespec returned by clock_gettime() syscall. */
	utilsut_expect_monotonic_now(10, 1000);
	/* Call clock_gettime() thanks to our library. */
	utime_monotonic_now(&now);

	/* Check that values scheduled by utilsut_expect_monotonic_now() call
	 * above have effectively been stored into the timespec structure.
	 */
	cute_check_sint(now.tv_sec, equal, 10);
	cute_check_sint(now.tv_nsec, equal, 1000);
}

static void
utilsut_utimer_expire(struct utimer * __restrict timer __unused)
{

}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utimer_arm_tspec_assert)
{
	struct utimer         inval_tmr = UTIMER_INIT(inval_tmr, NULL);
	struct utimer         valid_tmr = UTIMER_INIT(valid_tmr,
	                                              utilsut_utimer_expire);
	const struct timespec valid_exp = { 10, 0 };
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

	cute_expect_assertion(utimer_arm_tspec(NULL, &valid_exp));
	cute_expect_assertion(utimer_arm_tspec(&valid_tmr, NULL));
	cute_expect_assertion(utimer_arm_tspec(&inval_tmr, &valid_exp));
	cute_expect_assertion(utimer_arm_tspec(&valid_tmr, &sec_neg));
	cute_expect_assertion(utimer_arm_tspec(&valid_tmr, &nsec_over));
	cute_expect_assertion(utimer_arm_tspec(&valid_tmr, &nsec_neg));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utimer_arm_tspec_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utimer_is_armed_assert)
{
	struct utimer tmr = UTIMER_INIT(tmr, NULL);
	bool          armed __unused;

	cute_expect_assertion(armed = utimer_is_armed(NULL));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utimer_is_armed_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static struct utimer utilsut_utimer_the_timer;

static void
utilsut_utimer_setup(void)
{
	utimer_init(&utilsut_utimer_the_timer, utilsut_utimer_expire);
}

static void
utilsut_utimer_teardown(void)
{
	utimer_cancel(&utilsut_utimer_the_timer);
}

CUTE_TEST_STATIC(utilsut_utimer_is_armed,
                 utilsut_utimer_setup,
                 utilsut_utimer_teardown,
                 CUTE_DFLT_TMOUT)
{
	const struct timespec exp = { 10, 0};

	cute_check_bool(utimer_is_armed(&utilsut_utimer_the_timer), is, false);
	utimer_arm_tspec(&utilsut_utimer_the_timer, &exp);
	cute_check_bool(utimer_is_armed(&utilsut_utimer_the_timer), is, true);
	utimer_cancel(&utilsut_utimer_the_timer);
	cute_check_bool(utimer_is_armed(&utilsut_utimer_the_timer), is, false);
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utimer_expiry_tick_assert)
{
	struct utimer tmr = UTIMER_INIT(tmr, NULL);
	uint64_t      tick __unused;

	cute_expect_assertion(tick = utimer_expiry_tick(NULL));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utimer_expiry_tick_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST_STATIC(utilsut_utimer_expiry_tick,
                 utilsut_utimer_setup,
                 utilsut_utimer_teardown,
                 CUTE_DFLT_TMOUT)
{
	unsigned int t;

	for (t = 0; t < stroll_array_nr(utilsut_utimer_tspecs); t++) {
		uint64_t tick;

		utimer_arm_tspec(&utilsut_utimer_the_timer,
		                 &utilsut_utimer_tspecs[t]);

		tick = utimer_expiry_tick(&utilsut_utimer_the_timer);
		cute_check_uint(
			tick,
			equal,
			UTILSUT_TSPEC2TICKS_UPPER(&utilsut_utimer_tspecs[t]));
	}
}

CUTE_TEST_STATIC(utilsut_utimer_cancelled_expiry_tick,
                 utilsut_utimer_setup,
                 utilsut_utimer_teardown,
                 CUTE_DFLT_TMOUT)
{
	unsigned int t;

	for (t = 0; t < stroll_array_nr(utilsut_utimer_tspecs); t++) {
		uint64_t tick;

		utimer_arm_tspec(&utilsut_utimer_the_timer,
		                 &utilsut_utimer_tspecs[t]);
		utimer_cancel(&utilsut_utimer_the_timer);

		tick = utimer_expiry_tick(&utilsut_utimer_the_timer);
		cute_check_uint(
			tick,
			equal,
			UTILSUT_TSPEC2TICKS_UPPER(&utilsut_utimer_tspecs[t]));
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

	CUTE_REF(utilsut_utimer_monotonic_now),

	CUTE_REF(utilsut_utimer_arm_tspec_assert),

	CUTE_REF(utilsut_utimer_is_armed_assert),
	CUTE_REF(utilsut_utimer_is_armed),

	CUTE_REF(utilsut_utimer_expiry_tick_assert),
	CUTE_REF(utilsut_utimer_expiry_tick),
	CUTE_REF(utilsut_utimer_cancelled_expiry_tick),
};

CUTE_SUITE_EXTERN(utilsut_timer_suite,
                  utilsut_timer_group,
                  CUTE_NULL_SETUP,
                  CUTE_NULL_TEARDOWN,
                  CUTE_DFLT_TMOUT);
