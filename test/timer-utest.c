/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "../src/timer.h"
#include "utest.h"

/* Just make sure that clock_gettime() syscall mocking operates properly. */
CUTE_TEST(utilsut_utimer_monotonic_now)
{
	const struct timespec exp = { .tv_sec = 10, .tv_nsec = 1000 };
	struct timespec now = { -1, -1 };

	/* Schedule next timespec returned by clock_gettime() syscall. */
	utilsut_expect_monotonic_now(&exp);
	/* Call clock_gettime() thanks to our library. */
	utime_monotonic_now(&now);
	utilsut_expect_monotonic_now(NULL);

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
	utilsut_expect_monotonic_now(NULL);
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

static const struct timespec utilsut_clock[] = {
	{
		.tv_sec  = 0,
		.tv_nsec = 0
	},
	{
		.tv_sec  = 0,
		.tv_nsec = UTIMER_TICK_NSEC - 1
	},

#if UTIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = 0,
		.tv_nsec = UTIMER_TICK_NSEC
	},
	{
		.tv_sec  = 0,
		.tv_nsec = UTIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = 0,
		.tv_nsec = (UTIMER_TICKS_PER_SEC * UTIMER_TICK_NSEC) -
		           1
	},

	{
		.tv_sec  = 1,
		.tv_nsec = 0
	},
	{
		.tv_sec  = 1,
		.tv_nsec = UTIMER_TICK_NSEC - 1
	},
#if UTIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = 1,
		.tv_nsec = UTIMER_TICK_NSEC
	},
	{
		.tv_sec  = 1,
		.tv_nsec = UTIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = 1,
		.tv_nsec = (UTIMER_TICKS_PER_SEC * UTIMER_TICK_NSEC) -
		           1
	},

	{
		.tv_sec  = 365 * 24 * 60 * 60,
		.tv_nsec = 0,
	},
	{
		.tv_sec  = 365 * 24 * 60 * 60,
		.tv_nsec = UTIMER_TICK_NSEC - 1
	},
#if UTIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = 365 * 24 * 60 * 60,
		.tv_nsec = UTIMER_TICK_NSEC
	},
	{
		.tv_sec  = 365 * 24 * 60 * 60,
		.tv_nsec = UTIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = 365 * 24 * 60 * 60,
		.tv_nsec = (UTIMER_TICKS_PER_SEC * UTIMER_TICK_NSEC) -
		           1
	},

	{
		.tv_sec  = INT_MAX / 1000,
		.tv_nsec = 0,
	},
	{
		.tv_sec  = INT_MAX / 1000,
		.tv_nsec = UTIMER_TICK_NSEC - 1
	},
#if UTIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = INT_MAX / 1000,
		.tv_nsec = UTIMER_TICK_NSEC
	},
	{
		.tv_sec  = INT_MAX / 1000,
		.tv_nsec = UTIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = INT_MAX / 1000,
		.tv_nsec = (UTIMER_TICKS_PER_SEC * UTIMER_TICK_NSEC) -
		           1
	},

	{
		.tv_sec  = ((time_t)INT_MAX + 999) / 1000,
		.tv_nsec = 0,
	},
	{
		.tv_sec  = ((time_t)INT_MAX + 999) / 1000,
		.tv_nsec = UTIMER_TICK_NSEC - 1
	},
#if UTIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = ((time_t)INT_MAX + 999) / 1000,
		.tv_nsec = UTIMER_TICK_NSEC
	},
	{
		.tv_sec  = ((time_t)INT_MAX + 999) / 1000,
		.tv_nsec = UTIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = ((time_t)INT_MAX + 999) / 1000,
		.tv_nsec = (UTIMER_TICKS_PER_SEC * UTIMER_TICK_NSEC) -
		           1
	},

	{
		.tv_sec  = UTIMER_TICK_MAX >> UTIMER_TICK_SUBSEC_BITS,
		.tv_nsec = UTIMER_TICK_NSEC - 1
	},
#if UTIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = UTIMER_TICK_MAX >> UTIMER_TICK_SUBSEC_BITS,
		.tv_nsec = UTIMER_TICK_NSEC
	},
	{
		.tv_sec  = UTIMER_TICK_MAX >> UTIMER_TICK_SUBSEC_BITS,
		.tv_nsec = UTIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = UTIMER_TICK_MAX >> UTIMER_TICK_SUBSEC_BITS,
		.tv_nsec = (UTIMER_TICK_MAX & UTIMER_TICK_SUBSEC_MASK) *
		           UTIMER_TICK_NSEC
	}
};

CUTE_TEST_STATIC(utilsut_utimer_issue_msec,
                 utilsut_utimer_setup,
                 utilsut_utimer_teardown,
                 CUTE_DFLT_TMOUT)
{
	unsigned int e;
	unsigned int c;

	cute_check_sint(utimer_issue_msec(), equal, -1);
	cute_check_bool(utimer_is_armed(&utilsut_utimer_the_timer), is, false);

	for (e = 0; e < stroll_array_nr(utilsut_clock); e++) {
		struct timespec exp = utilsut_clock[e];

		utimer_arm_tspec(&utilsut_utimer_the_timer, &exp);
		cute_check_bool(utimer_is_armed(&utilsut_utimer_the_timer),
		                is,
		                true);

		for (c = 0; c < stroll_array_nr(utilsut_clock); c++) {
			struct timespec diff = exp;
			struct timespec clk = utilsut_clock[c];
			int             msec;

			diff.tv_nsec = stroll_round_upper(diff.tv_nsec,
			                                  UTIMER_TICK_NSEC);
			if (diff.tv_nsec >= 1000000000L) {
				if (diff.tv_sec < (UTIMER_TICK_MAX >>
				                   UTIMER_TICK_SUBSEC_BITS)) {
					diff.tv_sec += 1;
					diff.tv_nsec -= 1000000000L;
				}
				else {
					diff.tv_sec  = UTIMER_TICK_MAX >>
					               UTIMER_TICK_SUBSEC_BITS;
					diff.tv_nsec = (UTIMER_TICK_MAX &
					                UTIMER_TICK_SUBSEC_MASK)
					               * UTIMER_TICK_NSEC;
				}
			}
			if (utime_tspec_sub(&diff, &clk) > 0)
				msec = utime_msec_from_tspec_upper_clamp(&diff);
			else
				msec = 0;

			utilsut_expect_monotonic_now(&utilsut_clock[c]);
			cute_check_sint(utimer_issue_msec(), equal, msec);
			utilsut_expect_monotonic_now(NULL);
		}

		utimer_cancel(&utilsut_utimer_the_timer);
		cute_check_sint(utimer_issue_msec(), equal, -1);
	}
}

struct utilsut_timer {
	struct utimer         base;
	const struct timespec expire;
	int                   count;
	struct timespec       expected;
};

static struct utilsut_timer utilsut_timers[] = {
	{
		.expire.tv_sec  = 0,
		.expire.tv_nsec = 0
	},
	{
		.expire.tv_sec  = 0,
		.expire.tv_nsec = UTIMER_TICK_NSEC - 1
	},
#if UTIMER_TICK_SUBSEC_BITS > 0
	{
		.expire.tv_sec  = 0,
		.expire.tv_nsec = UTIMER_TICK_NSEC
	},
	{
		.expire.tv_sec  = 0,
		.expire.tv_nsec = UTIMER_TICK_NSEC + 1
	},
#endif
	{
		.expire.tv_sec  = 0,
		.expire.tv_nsec = (UTIMER_TICKS_PER_SEC * UTIMER_TICK_NSEC) -
		                  1
	},

	{
		.expire.tv_sec  = 1,
		.expire.tv_nsec = 0
	},
	{
		.expire.tv_sec  = 1,
		.expire.tv_nsec = UTIMER_TICK_NSEC - 1
	},
#if UTIMER_TICK_SUBSEC_BITS > 0
	{
		.expire.tv_sec  = 1,
		.expire.tv_nsec = UTIMER_TICK_NSEC
	},
	{
		.expire.tv_sec  = 1,
		.expire.tv_nsec = UTIMER_TICK_NSEC + 1
	},
#endif
	{
		.expire.tv_sec  = 1,
		.expire.tv_nsec = (UTIMER_TICKS_PER_SEC * UTIMER_TICK_NSEC) -
		                  1
	},

	{
		.expire.tv_sec  = 37 * 60,
		.expire.tv_nsec = 0
	},
	{
		.expire.tv_sec  = 37 * 60,
		.expire.tv_nsec = UTIMER_TICK_NSEC - 1
	},
#if UTIMER_TICK_SUBSEC_BITS > 0
	{
		.expire.tv_sec  = 37 * 60,
		.expire.tv_nsec = UTIMER_TICK_NSEC
	},
	{
		.expire.tv_sec  = 37 * 60,
		.expire.tv_nsec = UTIMER_TICK_NSEC + 1
	},
#endif
	{
		.expire.tv_sec  = 37 * 60,
		.expire.tv_nsec = (UTIMER_TICKS_PER_SEC * UTIMER_TICK_NSEC) -
		                  1
	},

	{
		.expire.tv_sec  = 59 * 60,
		.expire.tv_nsec = 0
	},
	{
		.expire.tv_sec  = 59 * 60,
		.expire.tv_nsec = UTIMER_TICK_NSEC - 1
	},
#if UTIMER_TICK_SUBSEC_BITS > 0
	{
		.expire.tv_sec  = 59 * 60,
		.expire.tv_nsec = UTIMER_TICK_NSEC
	},
	{
		.expire.tv_sec  = 59 * 60,
		.expire.tv_nsec = UTIMER_TICK_NSEC + 1
	},
#endif
	{
		.expire.tv_sec  = 59 * 60,
		.expire.tv_nsec = (UTIMER_TICKS_PER_SEC * UTIMER_TICK_NSEC) -
		                  1
	},
};

static const struct timespec * utilsut_curr_clock;

static void
utilsut_utimer_expire_arm(struct utimer * __restrict timer)
{
	struct utilsut_timer * tmr = (struct utilsut_timer *)timer;

	tmr->count--;

	cute_check_sint(utilsut_curr_clock->tv_sec,
	                equal,
	                tmr->expected.tv_sec);
	cute_check_sint(utilsut_curr_clock->tv_nsec,
	                equal,
	                tmr->expected.tv_nsec);
}

static void
utilsut_utimer_setup_arm(void)
{
	unsigned int t;

	for (t = 0; t < stroll_array_nr(utilsut_timers); t++) {
		struct utilsut_timer * tmr = &utilsut_timers[t];

		utimer_init(&tmr->base, utilsut_utimer_expire_arm);
	}
}

static void
utilsut_utimer_teardown_arm(void)
{
	unsigned int t;

	for (t = 0; t < stroll_array_nr(utilsut_timers); t++) {
		struct utilsut_timer * tmr = &utilsut_timers[t];

		utimer_cancel(&tmr->base);
	}

	utilsut_expect_monotonic_now(NULL);
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

CUTE_TEST_STATIC(utilsut_utimer_arm_tspec,
                 utilsut_utimer_setup_arm,
                 utilsut_utimer_teardown_arm,
                 CUTE_DFLT_TMOUT)
{
	unsigned int       m;
	const unsigned int msecs[] = {
		1, 2, 5, 8, 20, 25, 40, 50, 100, 200, 500, 1000
	};

	for (m = 0; m < stroll_array_nr(msecs); m++) {
		unsigned int          t;
		struct timespec       clk = { .tv_sec = 0, clk.tv_nsec = 0 };
		const struct timespec last_clk = {
			.tv_sec  = 60 * 60,
			.tv_nsec = 0
		};

		for (t = 0; t < stroll_array_nr(utilsut_timers); t++) {
			struct utilsut_timer * tmr = &utilsut_timers[t];

			utimer_arm_tspec(&tmr->base, &tmr->expire);
			cute_check_bool(utimer_is_armed(&tmr->base), is, true);

			tmr->count = 1;

			tmr->expected = tmr->expire;
			tmr->expected.tv_nsec =
				stroll_round_upper(tmr->expected.tv_nsec,
				                   UTIMER_TICK_NSEC);
			tmr->expected.tv_nsec =
				stroll_round_upper(tmr->expected.tv_nsec,
				                   msecs[m] * 1000000);
			if (tmr->expected.tv_nsec >= 1000000000L) {
				tmr->expected.tv_sec += 1;
				tmr->expected.tv_nsec -= 1000000000L;
			}
		}

		while (utime_tspec_before_eq(&clk, &last_clk)) {
			utilsut_curr_clock = &clk;
			utilsut_expect_monotonic_now(&clk);
			utimer_run();
			utilsut_expect_monotonic_now(NULL);

			utime_tspec_add_msec_clamp(&clk, (int)msecs[m]);
		}

		for (t = 0; t < stroll_array_nr(utilsut_timers); t++) {
			cute_check_bool(
				utimer_is_armed(&utilsut_timers[t].base),
				is,
				false);
			cute_check_sint(utilsut_timers[t].count, equal, 0);
		}
	}
}

CUTE_GROUP(utilsut_timer_group) = {
	CUTE_REF(utilsut_utimer_monotonic_now),

	CUTE_REF(utilsut_utimer_is_armed_assert),
	CUTE_REF(utilsut_utimer_is_armed),

	CUTE_REF(utilsut_utimer_issue_msec),

	CUTE_REF(utilsut_utimer_arm_tspec_assert),
	CUTE_REF(utilsut_utimer_arm_tspec),
};

CUTE_SUITE_EXTERN(utilsut_timer_suite,
                  utilsut_timer_group,
                  CUTE_NULL_SETUP,
                  CUTE_NULL_TEARDOWN,
                  CUTE_DFLT_TMOUT);
