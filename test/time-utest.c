/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/time.h"
#include <cute/cute.h>
#include <cute/check.h>
#include <cute/expect.h>

#define UTILSUT_NOASSERT_TEST(_test) \
	CUTE_TEST(_test) { }

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_realtime_now_assert)
{
	cute_expect_assertion(utime_realtime_now(NULL));
}

CUTE_TEST(utilsut_utime_monotonic_now_assert)
{
	cute_expect_assertion(utime_monotonic_now(NULL));
}

CUTE_TEST(utilsut_utime_boot_now_assert)
{
	cute_expect_assertion(utime_boot_now(NULL));
}

CUTE_TEST(utilsut_utime_coarse_now_assert)
{
	cute_expect_assertion(utime_coarse_now(NULL));
}

CUTE_TEST(utilsut_utime_proc_now_assert)
{
	cute_expect_assertion(utime_proc_now(NULL));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_realtime_now_assert)
UTILSUT_NOASSERT_TEST(utilsut_utime_monotonic_now_assert)
UTILSUT_NOASSERT_TEST(utilsut_utime_boot_now_assert)
UTILSUT_NOASSERT_TEST(utilsut_utime_coarse_now_assert)
UTILSUT_NOASSERT_TEST(utilsut_utime_proc_now_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_realtime_now)
{
	struct timespec now = { 0, };

	utime_realtime_now(&now);

	cute_check_sint(now.tv_sec, greater, 0);
	cute_check_sint(now.tv_nsec, greater, 0);
}

CUTE_TEST(utilsut_utime_monotonic_now)
{
	struct timespec now = { 0, };

	utime_monotonic_now(&now);

	cute_check_sint(now.tv_sec, greater, 0);
	cute_check_sint(now.tv_nsec, greater, 0);
}

CUTE_TEST(utilsut_utime_boot_now)
{
	struct timespec now = { 0, };

	utime_boot_now(&now);

	cute_check_sint(now.tv_sec, greater, 0);
	cute_check_sint(now.tv_nsec, greater, 0);
}

CUTE_TEST(utilsut_utime_coarse_now)
{
	struct timespec now = { 0, };

	utime_coarse_now(&now);

	cute_check_sint(now.tv_sec, greater, 0);
	cute_check_sint(now.tv_nsec, greater, 0);
}

CUTE_TEST(utilsut_utime_proc_now)
{
	struct timespec now = { 0, };

	utime_proc_now(&now);

	cute_check_sint(now.tv_sec, greater_equal, 0);
	cute_check_sint(now.tv_nsec, greater, 0);
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_tspec_cmp_assert)
{
	const struct timespec first = { 0, };
	const struct timespec second = { 0, };
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
	int                   cmp __unused;

	cute_expect_assertion(cmp = utime_tspec_cmp(&first, NULL));
	cute_expect_assertion(cmp = utime_tspec_cmp(NULL, &second));
	cute_expect_assertion(cmp = utime_tspec_cmp(NULL, NULL));
	cute_expect_assertion(cmp = utime_tspec_cmp(&first, &sec_neg));
	cute_expect_assertion(cmp = utime_tspec_cmp(&sec_neg, &first));
	cute_expect_assertion(cmp = utime_tspec_cmp(&first, &nsec_over));
	cute_expect_assertion(cmp = utime_tspec_cmp(&nsec_over, &first));
	cute_expect_assertion(cmp = utime_tspec_cmp(&first, &nsec_neg));
	cute_expect_assertion(cmp = utime_tspec_cmp(&nsec_neg, &first));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_cmp_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static const struct timespec utilsut_utime_values[] = {
	{ .tv_sec = 0,            .tv_nsec = 0 },
	{ .tv_sec = 0,            .tv_nsec = 1 },
	{ .tv_sec = 0,            .tv_nsec = 500000000 },
	{ .tv_sec = 0,            .tv_nsec = 999999999 },
	{ .tv_sec = 1000,         .tv_nsec = 0 },
	{ .tv_sec = 1000,         .tv_nsec = 1 },
	{ .tv_sec = 1000,         .tv_nsec = 500000000 },
	{ .tv_sec = 1000,         .tv_nsec = 999999999 },
	{ .tv_sec = LONG_MAX / 2, .tv_nsec = 0 },
	{ .tv_sec = LONG_MAX / 2, .tv_nsec = 1 },
	{ .tv_sec = LONG_MAX / 2, .tv_nsec = 500000000 },
	{ .tv_sec = LONG_MAX / 2, .tv_nsec = 999999999 },
	{ .tv_sec = LONG_MAX,     .tv_nsec = 0 },
	{ .tv_sec = LONG_MAX,     .tv_nsec = 1 },
	{ .tv_sec = LONG_MAX,     .tv_nsec = 500000000 },
	{ .tv_sec = LONG_MAX,     .tv_nsec = 999999999 }
};

CUTE_TEST(utilsut_utime_tspec_cmp)
{
	unsigned int v;

	for (v = 1; v < stroll_array_nr(utilsut_utime_values); v++) {
		struct timespec tmp = utilsut_utime_values[v];

		cute_check_sint(utime_tspec_cmp(&utilsut_utime_values[v - 1],
		                                &utilsut_utime_values[v]),
		                lower,
		                0);
		cute_check_sint(utime_tspec_cmp(&utilsut_utime_values[v],
		                                &tmp),
		                equal,
		                0);
		cute_check_sint(utime_tspec_cmp(&utilsut_utime_values[v],
		                                &utilsut_utime_values[v - 1]),
		                greater,
		                0);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_tspec_after_assert)
{
	const struct timespec first = { 0, };
	const struct timespec second = { 0, };
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
	int                   cmp __unused;

	cute_expect_assertion(cmp = utime_tspec_after(&first, NULL));
	cute_expect_assertion(cmp = utime_tspec_after(NULL, &second));
	cute_expect_assertion(cmp = utime_tspec_after(NULL, NULL));
	cute_expect_assertion(cmp = utime_tspec_after(&first, &sec_neg));
	cute_expect_assertion(cmp = utime_tspec_after(&sec_neg, &first));
	cute_expect_assertion(cmp = utime_tspec_after(&first, &nsec_over));
	cute_expect_assertion(cmp = utime_tspec_after(&nsec_over, &first));
	cute_expect_assertion(cmp = utime_tspec_after(&first, &nsec_neg));
	cute_expect_assertion(cmp = utime_tspec_after(&nsec_neg, &first));

	cute_expect_assertion(cmp = utime_tspec_after_eq(&first, NULL));
	cute_expect_assertion(cmp = utime_tspec_after_eq(NULL, &second));
	cute_expect_assertion(cmp = utime_tspec_after_eq(NULL, NULL));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&first, &sec_neg));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&sec_neg, &first));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&first, &nsec_over));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&nsec_over, &first));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&first, &nsec_neg));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&nsec_neg, &first));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_cmp_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_tspec_after)
{
	unsigned int v;

	for (v = 1; v < stroll_array_nr(utilsut_utime_values); v++) {
		struct timespec tmp = utilsut_utime_values[v];

		cute_check_bool(utime_tspec_after(&utilsut_utime_values[v],
		                                  &utilsut_utime_values[v - 1]),
		                is,
		                true);
		cute_check_bool(utime_tspec_after(&utilsut_utime_values[v - 1],
		                                  &utilsut_utime_values[v]),
		                is,
		                false);
		cute_check_bool(
			utime_tspec_after_eq(&utilsut_utime_values[v],
			                     &utilsut_utime_values[v - 1]),
			is,
			true);
		cute_check_bool(
			utime_tspec_after_eq(&utilsut_utime_values[v - 1],
			                     &utilsut_utime_values[v]),
			is,
			false);
		cute_check_bool(
			utime_tspec_after_eq(&utilsut_utime_values[v],
			                     &tmp),
			is,
			true);
		cute_check_bool(
			utime_tspec_after_eq(&tmp,
			                     &utilsut_utime_values[v]),
			is,
			true);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_tspec_before_assert)
{
	const struct timespec first = { 0, };
	const struct timespec second = { 0, };
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
	int                   cmp __unused;

	cute_expect_assertion(cmp = utime_tspec_before(&first, NULL));
	cute_expect_assertion(cmp = utime_tspec_before(NULL, &second));
	cute_expect_assertion(cmp = utime_tspec_before(NULL, NULL));
	cute_expect_assertion(cmp = utime_tspec_before(&first, &sec_neg));
	cute_expect_assertion(cmp = utime_tspec_before(&sec_neg, &first));
	cute_expect_assertion(cmp = utime_tspec_before(&first, &nsec_over));
	cute_expect_assertion(cmp = utime_tspec_before(&nsec_over, &first));
	cute_expect_assertion(cmp = utime_tspec_before(&first, &nsec_neg));
	cute_expect_assertion(cmp = utime_tspec_before(&nsec_neg, &first));

	cute_expect_assertion(cmp = utime_tspec_before_eq(&first, NULL));
	cute_expect_assertion(cmp = utime_tspec_before_eq(NULL, &second));
	cute_expect_assertion(cmp = utime_tspec_before_eq(NULL, NULL));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&first, &sec_neg));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&sec_neg, &first));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&first, &nsec_over));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&nsec_over, &first));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&first, &nsec_neg));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&nsec_neg, &first));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_cmp_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_tspec_before)
{
	unsigned int v;

	for (v = 1; v < stroll_array_nr(utilsut_utime_values); v++) {
		struct timespec tmp = utilsut_utime_values[v];

		cute_check_bool(utime_tspec_before(&utilsut_utime_values[v - 1],
		                                   &utilsut_utime_values[v]),
		                is,
		                true);
		cute_check_bool(utime_tspec_before(&utilsut_utime_values[v],
		                                   &utilsut_utime_values[v - 1]),
		                is,
		                false);
		cute_check_bool(
			utime_tspec_before_eq(&utilsut_utime_values[v - 1],
			                      &utilsut_utime_values[v]),
			is,
			true);
		cute_check_bool(
			utime_tspec_before_eq(&utilsut_utime_values[v],
			                      &utilsut_utime_values[v - 1]),
			is,
			false);
		cute_check_bool(
			utime_tspec_before_eq(&utilsut_utime_values[v],
			                      &tmp),
			is,
			true);
		cute_check_bool(
			utime_tspec_before_eq(&tmp,
			                      &utilsut_utime_values[v]),
			is,
			true);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_tspec_from_msec_assert)
{
	struct timespec tspec __unused;

	cute_expect_assertion(tspec = utime_tspec_from_msec((unsigned long)
	                                                    LONG_MAX + 1));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_from_msec_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_tspec_from_msec)
{
	const struct {
		struct timespec tspec;
		unsigned long   msecs;
	} values[] = {
		{
			.tspec.tv_sec  = 0,
			.tspec.tv_nsec = 0,
			.msecs         = 0
		},
		{
			.tspec.tv_sec  = 0,
			.tspec.tv_nsec = 1000000L,
			.msecs         = 1
		},
		{
			.tspec.tv_sec  = 0,
			.tspec.tv_nsec = 2000000L,
			.msecs         = 2
		},
		{
			.tspec.tv_sec  = 0,
			.tspec.tv_nsec = 999000000L,
			.msecs         = 999
		},
		{
			.tspec.tv_sec  = 1,
			.tspec.tv_nsec = 0L,
			.msecs         = 1000
		},
		{
			.tspec.tv_sec  = 2,
			.tspec.tv_nsec = 0L,
			.msecs         = 2000
		},
		{
			.tspec.tv_sec  = 2,
			.tspec.tv_nsec = 1000000L,
			.msecs         = 2001
		},
		{
			.tspec.tv_sec  = 2,
			.tspec.tv_nsec = 999000000L,
			.msecs         = 2999
		},
		{
			.tspec.tv_sec  = LONG_MAX / 1000L,
			.tspec.tv_nsec = (LONG_MAX % 1000L) * 1000000L,
			.msecs         = LONG_MAX
		}
	};
	unsigned int            v;

	for (v = 0; v < stroll_array_nr(values); v++) {
		struct timespec check;

		check = utime_tspec_from_msec(values[v].msecs);

		cute_check_sint(check.tv_sec, equal, values[v].tspec.tv_sec);
		cute_check_sint(check.tv_nsec, equal, values[v].tspec.tv_nsec);
	}
}

CUTE_GROUP(utilsut_time_group) = {
	CUTE_REF(utilsut_utime_realtime_now_assert),
	CUTE_REF(utilsut_utime_realtime_now),
	CUTE_REF(utilsut_utime_monotonic_now_assert),
	CUTE_REF(utilsut_utime_monotonic_now),
	CUTE_REF(utilsut_utime_boot_now_assert),
	CUTE_REF(utilsut_utime_boot_now),
	CUTE_REF(utilsut_utime_coarse_now_assert),
	CUTE_REF(utilsut_utime_coarse_now),
	CUTE_REF(utilsut_utime_proc_now_assert),
	CUTE_REF(utilsut_utime_proc_now),

	CUTE_REF(utilsut_utime_tspec_cmp_assert),
	CUTE_REF(utilsut_utime_tspec_cmp),
	CUTE_REF(utilsut_utime_tspec_after_assert),
	CUTE_REF(utilsut_utime_tspec_after),
	CUTE_REF(utilsut_utime_tspec_before_assert),
	CUTE_REF(utilsut_utime_tspec_before),

	CUTE_REF(utilsut_utime_tspec_from_msec_assert),
	CUTE_REF(utilsut_utime_tspec_from_msec),
};

CUTE_SUITE_EXTERN(utilsut_time_suite,
                  utilsut_time_group,
                  CUTE_NULL_SETUP,
                  CUTE_NULL_TEARDOWN,
                  CUTE_DFLT_TMOUT);
