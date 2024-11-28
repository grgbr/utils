/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/time.h"
#include "utest.h"

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
	const struct timespec tspec = { 0, };
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

	cute_expect_assertion(cmp = utime_tspec_cmp(&tspec, NULL));
	cute_expect_assertion(cmp = utime_tspec_cmp(NULL, &tspec));
	cute_expect_assertion(cmp = utime_tspec_cmp(NULL, NULL));
	cute_expect_assertion(cmp = utime_tspec_cmp(&tspec, &sec_neg));
	cute_expect_assertion(cmp = utime_tspec_cmp(&sec_neg, &tspec));
	cute_expect_assertion(cmp = utime_tspec_cmp(&tspec, &nsec_over));
	cute_expect_assertion(cmp = utime_tspec_cmp(&nsec_over, &tspec));
	cute_expect_assertion(cmp = utime_tspec_cmp(&tspec, &nsec_neg));
	cute_expect_assertion(cmp = utime_tspec_cmp(&nsec_neg, &tspec));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_cmp_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static const struct timespec utilsut_utime_values[] = {
	{ .tv_sec = 0,                   .tv_nsec = 0 },
	{ .tv_sec = 0,                   .tv_nsec = 1 },
	{ .tv_sec = 0,                   .tv_nsec = 500000000 },
	{ .tv_sec = 0,                   .tv_nsec = 999999999 },
	{ .tv_sec = 1000,                .tv_nsec = 0 },
	{ .tv_sec = 1000,                .tv_nsec = 1 },
	{ .tv_sec = 1000,                .tv_nsec = 500000000 },
	{ .tv_sec = 1000,                .tv_nsec = 999999999 },
	{ .tv_sec = UTIME_TIMET_MAX / 2, .tv_nsec = 0 },
	{ .tv_sec = UTIME_TIMET_MAX / 2, .tv_nsec = 1 },
	{ .tv_sec = UTIME_TIMET_MAX / 2, .tv_nsec = 500000000 },
	{ .tv_sec = UTIME_TIMET_MAX / 2, .tv_nsec = 999999999 },
	{ .tv_sec = UTIME_TIMET_MAX,     .tv_nsec = 0 },
	{ .tv_sec = UTIME_TIMET_MAX,     .tv_nsec = 1 },
	{ .tv_sec = UTIME_TIMET_MAX,     .tv_nsec = 500000000 },
	{ .tv_sec = UTIME_TIMET_MAX,     .tv_nsec = 999999999 }
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
	const struct timespec tspec = { 0, };
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

	cute_expect_assertion(cmp = utime_tspec_after(&tspec, NULL));
	cute_expect_assertion(cmp = utime_tspec_after(NULL, &tspec));
	cute_expect_assertion(cmp = utime_tspec_after(NULL, NULL));
	cute_expect_assertion(cmp = utime_tspec_after(&tspec, &sec_neg));
	cute_expect_assertion(cmp = utime_tspec_after(&sec_neg, &tspec));
	cute_expect_assertion(cmp = utime_tspec_after(&tspec, &nsec_over));
	cute_expect_assertion(cmp = utime_tspec_after(&nsec_over, &tspec));
	cute_expect_assertion(cmp = utime_tspec_after(&tspec, &nsec_neg));
	cute_expect_assertion(cmp = utime_tspec_after(&nsec_neg, &tspec));

	cute_expect_assertion(cmp = utime_tspec_after_eq(&tspec, NULL));
	cute_expect_assertion(cmp = utime_tspec_after_eq(NULL, &tspec));
	cute_expect_assertion(cmp = utime_tspec_after_eq(NULL, NULL));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&tspec, &sec_neg));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&sec_neg, &tspec));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&tspec, &nsec_over));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&nsec_over, &tspec));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&tspec, &nsec_neg));
	cute_expect_assertion(cmp = utime_tspec_after_eq(&nsec_neg, &tspec));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_after_assert)

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
	const struct timespec tspec = { 0, };
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

	cute_expect_assertion(cmp = utime_tspec_before(&tspec, NULL));
	cute_expect_assertion(cmp = utime_tspec_before(NULL, &tspec));
	cute_expect_assertion(cmp = utime_tspec_before(NULL, NULL));
	cute_expect_assertion(cmp = utime_tspec_before(&tspec, &sec_neg));
	cute_expect_assertion(cmp = utime_tspec_before(&sec_neg, &tspec));
	cute_expect_assertion(cmp = utime_tspec_before(&tspec, &nsec_over));
	cute_expect_assertion(cmp = utime_tspec_before(&nsec_over, &tspec));
	cute_expect_assertion(cmp = utime_tspec_before(&tspec, &nsec_neg));
	cute_expect_assertion(cmp = utime_tspec_before(&nsec_neg, &tspec));

	cute_expect_assertion(cmp = utime_tspec_before_eq(&tspec, NULL));
	cute_expect_assertion(cmp = utime_tspec_before_eq(NULL, &tspec));
	cute_expect_assertion(cmp = utime_tspec_before_eq(NULL, NULL));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&tspec, &sec_neg));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&sec_neg, &tspec));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&tspec, &nsec_over));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&nsec_over, &tspec));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&tspec, &nsec_neg));
	cute_expect_assertion(cmp = utime_tspec_before_eq(&nsec_neg, &tspec));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_before_assert)

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

	cute_expect_assertion(tspec = utime_tspec_from_msec(-1));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_from_msec_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static const struct {
		struct timespec tspec;
		int             msecs;
} utilsut_utime_msecs_tspec_values[] = {
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
		.tspec.tv_sec  = INT_MAX / 1000,
		.tspec.tv_nsec = ((long)INT_MAX % 1000L) * 1000000L,
		.msecs         = INT_MAX
	}
};

CUTE_TEST(utilsut_utime_tspec_from_msec)
{
	unsigned int v;

	for (v = 0;
	     v < stroll_array_nr(utilsut_utime_msecs_tspec_values);
	     v++) {
		struct timespec check;

		check = utime_tspec_from_msec(
			utilsut_utime_msecs_tspec_values[v].msecs);

		cute_check_sint(
			check.tv_sec,
			equal,
			utilsut_utime_msecs_tspec_values[v].tspec.tv_sec);
		cute_check_sint(
			check.tv_nsec,
			equal,
			utilsut_utime_msecs_tspec_values[v].tspec.tv_nsec);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_msec_from_tspec_lower_assert)
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
	long                  msecs __unused;

	cute_expect_assertion(msecs = utime_msec_from_tspec_lower(NULL));
	cute_expect_assertion(msecs = utime_msec_from_tspec_lower(&sec_neg));
	cute_expect_assertion(msecs = utime_msec_from_tspec_lower(&nsec_over));
	cute_expect_assertion(msecs = utime_msec_from_tspec_lower(&nsec_neg));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_msec_from_tspec_lower_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_msec_from_tspec_lower)
{
	unsigned int v;

	for (v = 0;
	     v < stroll_array_nr(utilsut_utime_msecs_tspec_values);
	     v++) {
		int check;

		check = utime_msec_from_tspec_lower(
			&utilsut_utime_msecs_tspec_values[v].tspec);

		cute_check_sint(check, greater_equal, 0);
		cute_check_sint(check,
		                equal,
		                utilsut_utime_msecs_tspec_values[v].msecs);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_tspec_add_assert)
{
	struct timespec tspec = { 0, };
	struct timespec sec_neg = {
		.tv_sec  = -1,
		.tv_nsec = 0
	};
	struct timespec nsec_over = {
		.tv_sec  = 0,
		.tv_nsec = 1000000000L
	};
	struct timespec nsec_neg = {
		.tv_sec  = 0,
		.tv_nsec = -1
	};
	int             err __unused;

	cute_expect_assertion(err = utime_tspec_add(&tspec, NULL));
	cute_expect_assertion(err = utime_tspec_add(NULL, &tspec));
	cute_expect_assertion(err = utime_tspec_add(NULL, NULL));
	cute_expect_assertion(err = utime_tspec_add(&tspec, &sec_neg));
	cute_expect_assertion(err = utime_tspec_add(&sec_neg, &tspec));
	cute_expect_assertion(err = utime_tspec_add(&tspec, &nsec_over));
	cute_expect_assertion(err = utime_tspec_add(&nsec_over, &tspec));
	cute_expect_assertion(err = utime_tspec_add(&tspec, &nsec_neg));
	cute_expect_assertion(err = utime_tspec_add(&nsec_neg, &tspec));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_add_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_tspec_add)
{
	unsigned int r;
	const struct {
		struct timespec first;
		struct timespec second;
		struct timespec result;
	}            ref[] = {
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000000
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999999
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = UTIME_TIMET_MAX / 2,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 0
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 2
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000001
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = 1,
			.result.tv_nsec = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = UTIME_TIMET_MAX / 2,
			.result.tv_nsec = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 1
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000000
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000001
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = 1,
			.result.tv_nsec = 499999999
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = 1,
			.result.tv_nsec = 500000000
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = UTIME_TIMET_MAX / 2,
			.result.tv_nsec = 500000000
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 500000000
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999999
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = 1,
			.result.tv_nsec = 499999999
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = 1,
			.result.tv_nsec = 999999998
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = 1,
			.result.tv_nsec = 999999999
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = UTIME_TIMET_MAX / 2,
			.result.tv_nsec = 999999999
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 999999999
		},
	};

	for (r = 0; r < stroll_array_nr(ref); r++) {
		struct timespec check = ref[r].first;

		utime_tspec_add_clamp(&check, &ref[r].second);

		cute_check_sint(check.tv_sec,
				equal,
				ref[r].result.tv_sec);
		cute_check_sint(check.tv_nsec,
				equal,
				ref[r].result.tv_nsec);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_tspec_add_msec_assert)
{
	struct timespec zero = { 0, 0 };
	struct timespec sec_neg = {
		.tv_sec  = -1,
		.tv_nsec = 0
	};
	struct timespec nsec_over = {
		.tv_sec  = 0,
		.tv_nsec = 1000000000L
	};
	struct timespec nsec_neg = {
		.tv_sec  = 0,
		.tv_nsec = -1
	};
	int             err __unused;

	cute_expect_assertion(err = utime_tspec_add_msec(NULL, 0));
	cute_expect_assertion(err = utime_tspec_add_msec(&zero, -1));
	cute_expect_assertion(err = utime_tspec_add_msec(&sec_neg, 0));
	cute_expect_assertion(err = utime_tspec_add_msec(&nsec_over, 0));
	cute_expect_assertion(err = utime_tspec_add_msec(&nsec_neg, 0));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_add_msec_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_tspec_add_msec)
{
	unsigned int r;
	const struct {
		struct timespec first;
		int             msecs;
		struct timespec result;
	}            ref[] = {
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.msecs          = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.msecs          = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.msecs          = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000000
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.msecs          = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999999
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.msecs          = 0,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.msecs          = 0,
			.result.tv_sec  = UTIME_TIMET_MAX / 2,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.msecs          = 0,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 0
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.msecs          = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1000000
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.msecs          = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1000001
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.msecs          = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 501000000
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.msecs          = 1,
			.result.tv_sec  = 1,
			.result.tv_nsec = 999999
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.msecs          = 1,
			.result.tv_sec  = 1,
			.result.tv_nsec = 1000000
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.msecs          = 1,
			.result.tv_sec  = UTIME_TIMET_MAX / 2,
			.result.tv_nsec = 1000000
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.msecs          = 1,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 1000000
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.msecs          = 2999,
			.result.tv_sec  = 2,
			.result.tv_nsec = 999000000
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.msecs          = 2999,
			.result.tv_sec  = 2,
			.result.tv_nsec = 999000001
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.msecs          = 2999,
			.result.tv_sec  = 3,
			.result.tv_nsec = 499000000
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.msecs          = 2999,
			.result.tv_sec  = 3,
			.result.tv_nsec = 998999999
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.msecs          = 2999,
			.result.tv_sec  = 3,
			.result.tv_nsec = 999000000
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.msecs          = 2999,
			.result.tv_sec  = (UTIME_TIMET_MAX / 2) + 2,
			.result.tv_nsec = 999000000
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX - 2,
			.first.tv_nsec  = 0,
			.msecs          = 2999,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 999000000
		}
	};

	for (r = 0; r < stroll_array_nr(ref); r++) {
		struct timespec check = ref[r].first;

		utime_tspec_add_msec_clamp(&check, ref[r].msecs);

		cute_check_sint(check.tv_sec,
				equal,
				ref[r].result.tv_sec);
		cute_check_sint(check.tv_nsec,
				equal,
				ref[r].result.tv_nsec);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_tspec_add_sec_assert)
{
	struct timespec zero = { 0, 0 };
	struct timespec sec_neg = {
		.tv_sec  = -1,
		.tv_nsec = 0
	};
	struct timespec nsec_over = {
		.tv_sec  = 0,
		.tv_nsec = 1000000000L
	};
	struct timespec nsec_neg = {
		.tv_sec  = 0,
		.tv_nsec = -1
	};
	int             err __unused;

	cute_expect_assertion(err = utime_tspec_add_sec(NULL, 0));
	cute_expect_assertion(err = utime_tspec_add_sec(&zero, - 1));
	cute_expect_assertion(err = utime_tspec_add_sec(&sec_neg, 0));
	cute_expect_assertion(err = utime_tspec_add_sec(&nsec_over, 0));
	cute_expect_assertion(err = utime_tspec_add_sec(&nsec_neg, 0));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_add_sec_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_tspec_add_sec)
{
	unsigned int r;
	const struct {
		struct timespec first;
		int             secs;
		struct timespec result;
	}            ref[] = {
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.secs           = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.secs           = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.secs           = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000000
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.secs           = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999999
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.secs           = 0,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.secs           = 0,
			.result.tv_sec  = UTIME_TIMET_MAX / 2,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.secs           = 0,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 0
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.secs           = 1,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.secs           = 1,
			.result.tv_sec  = 1,
			.result.tv_nsec = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.secs           = 1,
			.result.tv_sec  = 1,
			.result.tv_nsec = 500000000
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.secs           = 1,
			.result.tv_sec  = 1,
			.result.tv_nsec = 999999999
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.secs           = 1,
			.result.tv_sec  = 2,
			.result.tv_nsec = 0
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 500000000,
			.secs           = 1,
			.result.tv_sec  = (UTIME_TIMET_MAX / 2) + 1,
			.result.tv_nsec = 500000000
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX - 1,
			.first.tv_nsec  = 500000000,
			.secs           = 1,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 500000000
		},
	};

	for (r = 0; r < stroll_array_nr(ref); r++) {
		struct timespec check = ref[r].first;

		utime_tspec_add_sec_clamp(&check, ref[r].secs);

		cute_check_sint(check.tv_sec,
				equal,
				ref[r].result.tv_sec);
		cute_check_sint(check.tv_nsec,
				equal,
				ref[r].result.tv_nsec);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_tspec_sub_assert)
{
	struct timespec tspec = { 0, };
	struct timespec sec_neg = {
		.tv_sec  = -1,
		.tv_nsec = 0
	};
	struct timespec nsec_over = {
		.tv_sec  = 0,
		.tv_nsec = 1000000000L
	};
	struct timespec nsec_neg = {
		.tv_sec  = 0,
		.tv_nsec = -1
	};
	int             sign __unused;

	cute_expect_assertion(sign = utime_tspec_sub(&tspec, NULL));
	cute_expect_assertion(sign = utime_tspec_sub(NULL, &tspec));
	cute_expect_assertion(sign = utime_tspec_sub(NULL, NULL));
	cute_expect_assertion(sign = utime_tspec_sub(&tspec, &sec_neg));
	cute_expect_assertion(sign = utime_tspec_sub(&sec_neg, &tspec));
	cute_expect_assertion(sign = utime_tspec_sub(&tspec, &nsec_over));
	cute_expect_assertion(sign = utime_tspec_sub(&nsec_over, &tspec));
	cute_expect_assertion(sign = utime_tspec_sub(&tspec, &nsec_neg));
	cute_expect_assertion(sign = utime_tspec_sub(&nsec_neg, &tspec));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_sub_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_tspec_sub)
{
	unsigned int r;
	const struct {
		struct timespec first;
		struct timespec second;
		struct timespec result;
		int             sign;
	}            ref[] = {
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 0,
			.sign           = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1,
			.sign           = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000000,
			.sign           = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999999,
			.sign           = 1
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = UTIME_TIMET_MAX / 2,
			.result.tv_nsec = 0,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 0,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 0,
			.sign           = 1
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 0,
			.sign           = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 499999999,
			.sign           = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999998,
			.sign           = 1
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999999,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = (UTIME_TIMET_MAX / 2) - 1,
			.result.tv_nsec = 999999999,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 1,
			.result.tv_sec  = UTIME_TIMET_MAX - 1,
			.result.tv_nsec = 999999999,
			.sign           = 1
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000000,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = 0,
			.result.tv_nsec = 499999999,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = 0,
			.result.tv_nsec = 0,
			.sign           = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = 0,
			.result.tv_nsec = 499999999,
			.sign           = 1
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000000,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = (UTIME_TIMET_MAX / 2) - 1,
			.result.tv_nsec = 500000000,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 500000000,
			.result.tv_sec  = UTIME_TIMET_MAX - 1,
			.result.tv_nsec = 500000000,
			.sign           = 1
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999999,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999998,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = 0,
			.result.tv_nsec = 499999999,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = 0,
			.result.tv_nsec = 0,
			.sign           = 0
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = (UTIME_TIMET_MAX / 2) - 1,
			.result.tv_nsec = 1,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.second.tv_sec  = 0,
			.second.tv_nsec = 999999999,
			.result.tv_sec  = UTIME_TIMET_MAX - 1,
			.result.tv_nsec = 1,
			.sign           = 1
		},
	};

	for (r = 0; r < stroll_array_nr(ref); r++) {
		struct timespec check = ref[r].first;
		int             sign;

		sign = utime_tspec_sub(&check, &ref[r].second);

		cute_check_sint(check.tv_sec,
				equal,
				ref[r].result.tv_sec);
		cute_check_sint(check.tv_nsec,
				equal,
				ref[r].result.tv_nsec);
		cute_check_sint(sign, equal, ref[r].sign);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_tspec_sub_msec_assert)
{
	struct timespec zero = { 0, 0 };
	struct timespec sec_neg = {
		.tv_sec  = -1,
		.tv_nsec = 0
	};
	struct timespec nsec_over = {
		.tv_sec  = 0,
		.tv_nsec = 1000000000L
	};
	struct timespec nsec_neg = {
		.tv_sec  = 0,
		.tv_nsec = -1
	};
	int                  sign __unused;

	cute_expect_assertion(sign = utime_tspec_sub_msec(NULL, 0));
	cute_expect_assertion(sign = utime_tspec_sub_msec(&zero, - 1));
	cute_expect_assertion(sign = utime_tspec_sub_msec(&sec_neg, 0));
	cute_expect_assertion(sign = utime_tspec_sub_msec(&nsec_over, 0));
	cute_expect_assertion(sign = utime_tspec_sub_msec(&nsec_neg, 0));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_sub_msec_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_tspec_sub_msec)
{
	unsigned int r;
	const struct {
		struct timespec first;
		int             msecs;
		struct timespec result;
		int             sign;
	}            ref[] = {
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.msecs          = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 0,
			.sign           = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.msecs          = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1,
			.sign           = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.msecs          = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000000,
			.sign           = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.msecs          = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999999,
			.sign           = 1
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.msecs          = 0,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.msecs          = 0,
			.result.tv_sec  = UTIME_TIMET_MAX / 2,
			.result.tv_nsec = 0,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.msecs          = 0,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 0,
			.sign           = 1
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.msecs          = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1000000,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.msecs          = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.msecs          = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 499000000,
			.sign           = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.msecs          = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 998999999,
			.sign           = 1
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.msecs          = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999000000,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.msecs          = 1,
			.result.tv_sec  = (UTIME_TIMET_MAX / 2) - 1,
			.result.tv_nsec = 999000000,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.msecs          = 1,
			.result.tv_sec  = UTIME_TIMET_MAX - 1,
			.result.tv_nsec = 999000000,
			.sign           = 1
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.msecs          = 2999,
			.result.tv_sec  = 2,
			.result.tv_nsec = 999000000,
			.sign           = -1,
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.msecs          = 2999,
			.result.tv_sec  = 2,
			.result.tv_nsec = 998999999,
			.sign           = -1,
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.msecs          = 2999,
			.result.tv_sec  = 2,
			.result.tv_nsec = 499000000,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.msecs          = 2999,
			.result.tv_sec  = 1,
			.result.tv_nsec = 999000001,
			.sign           = -1
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.msecs          = 2999,
			.result.tv_sec  = 1,
			.result.tv_nsec = 999000000,
			.sign           = -1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.msecs          = 2999,
			.result.tv_sec  = (UTIME_TIMET_MAX / 2) - 3,
			.result.tv_nsec = 1000000,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.msecs          = 2999,
			.result.tv_sec  = UTIME_TIMET_MAX - 3,
			.result.tv_nsec = 1000000,
			.sign           = 1
		}
	};

	for (r = 0; r < stroll_array_nr(ref); r++) {
		struct timespec check = ref[r].first;
		int             sign;

		sign = utime_tspec_sub_msec(&check, ref[r].msecs);

		cute_check_sint(check.tv_sec,
				equal,
				ref[r].result.tv_sec);
		cute_check_sint(check.tv_nsec,
				equal,
				ref[r].result.tv_nsec);
		cute_check_sint(sign, equal, ref[r].sign);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(utilsut_utime_tspec_sub_sec_assert)
{
	struct timespec zero = { 0, 0 };
	struct timespec sec_neg = {
		.tv_sec  = -1,
		.tv_nsec = 0
	};
	struct timespec nsec_over = {
		.tv_sec  = 0,
		.tv_nsec = 1000000000L
	};
	struct timespec nsec_neg = {
		.tv_sec  = 0,
		.tv_nsec = -1
	};
	int             sign __unused;

	cute_expect_assertion(sign = utime_tspec_sub_sec(NULL, 0));
	cute_expect_assertion(sign = utime_tspec_sub_sec(&zero, - 1));
	cute_expect_assertion(sign = utime_tspec_sub_sec(&sec_neg, 0));
	cute_expect_assertion(sign = utime_tspec_sub_sec(&nsec_over, 0));
	cute_expect_assertion(sign = utime_tspec_sub_sec(&nsec_neg, 0));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(utilsut_utime_tspec_sub_sec_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST(utilsut_utime_tspec_sub_sec)
{
	unsigned int r;
	const struct {
		struct timespec first;
		int             secs;
		struct timespec result;
		int             sign;
	}            ref[] = {
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.secs           = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 0,
			.sign           = 0
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.secs           = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1,
			.sign           = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.secs           = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000000,
			.sign           = 1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.secs           = 0,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999999,
			.sign           = 1
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.secs           = 0,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 0,
			.secs           = 0,
			.result.tv_sec  = UTIME_TIMET_MAX / 2,
			.result.tv_nsec = 0,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 0,
			.secs           = 0,
			.result.tv_sec  = UTIME_TIMET_MAX,
			.result.tv_nsec = 0,
			.sign           = 1
		},

		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 0,
			.secs           = 1,
			.result.tv_sec  = 1,
			.result.tv_nsec = 0,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 1,
			.secs           = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 999999999,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 500000000,
			.secs           = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 500000000,
			.sign           = -1
		},
		{
			.first.tv_sec   = 0,
			.first.tv_nsec  = 999999999,
			.secs           = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 1,
			.sign           = -1
		},
		{
			.first.tv_sec   = 1,
			.first.tv_nsec  = 0,
			.secs           = 1,
			.result.tv_sec  = 0,
			.result.tv_nsec = 0,
			.sign           = 0
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX / 2,
			.first.tv_nsec  = 500000000,
			.secs           = 1,
			.result.tv_sec  = (UTIME_TIMET_MAX / 2) - 1,
			.result.tv_nsec = 500000000,
			.sign           = 1
		},
		{
			.first.tv_sec   = UTIME_TIMET_MAX,
			.first.tv_nsec  = 500000000,
			.secs           = 1,
			.result.tv_sec  = UTIME_TIMET_MAX - 1,
			.result.tv_nsec = 500000000,
			.sign           = 1
		},
	};

	for (r = 0; r < stroll_array_nr(ref); r++) {
		struct timespec check = ref[r].first;
		int             sign;

		sign = utime_tspec_sub_sec(&check, ref[r].secs);

		cute_check_sint(check.tv_sec,
				equal,
				ref[r].result.tv_sec);
		cute_check_sint(check.tv_nsec,
				equal,
				ref[r].result.tv_nsec);
		cute_check_sint(sign, equal, ref[r].sign);
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
	CUTE_REF(utilsut_utime_msec_from_tspec_lower_assert),
	CUTE_REF(utilsut_utime_msec_from_tspec_lower),

	CUTE_REF(utilsut_utime_tspec_add_assert),
	CUTE_REF(utilsut_utime_tspec_add),
	CUTE_REF(utilsut_utime_tspec_add_msec_assert),
	CUTE_REF(utilsut_utime_tspec_add_msec),
	CUTE_REF(utilsut_utime_tspec_add_sec_assert),
	CUTE_REF(utilsut_utime_tspec_add_sec),

	CUTE_REF(utilsut_utime_tspec_sub_assert),
	CUTE_REF(utilsut_utime_tspec_sub),
	CUTE_REF(utilsut_utime_tspec_sub_msec_assert),
	CUTE_REF(utilsut_utime_tspec_sub_msec),
	CUTE_REF(utilsut_utime_tspec_sub_sec_assert),
	CUTE_REF(utilsut_utime_tspec_sub_sec),
};

CUTE_SUITE_EXTERN(utilsut_time_suite,
                  utilsut_time_group,
                  CUTE_NULL_SETUP,
                  CUTE_NULL_TEARDOWN,
                  CUTE_DFLT_TMOUT);
