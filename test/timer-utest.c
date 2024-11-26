/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "../timer/common.h"
#include "utest.h"

/* Just make sure that clock_gettime() syscall mocking operates properly. */
CUTE_TEST(etuxut_timer_monotonic_now)
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
etuxut_timer_expire(struct etux_timer * __restrict timer __unused)
{
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(etuxut_timer_is_armed_assert)
{
	struct etux_timer tmr = ETUX_TIMER_INIT(tmr, NULL);
	bool          armed __unused;

	cute_expect_assertion(armed = etux_timer_is_armed(NULL));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(etuxut_timer_is_armed_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static struct etux_timer etuxut_the_timer;

static void
etuxut_timer_setup(void)
{
	etux_timer_init(&etuxut_the_timer, etuxut_timer_expire);
}

static void
etuxut_timer_teardown(void)
{
	etux_timer_cancel(&etuxut_the_timer);
	utilsut_expect_monotonic_now(NULL);
}

CUTE_TEST_STATIC(etuxut_timer_is_armed,
                 etuxut_timer_setup,
                 etuxut_timer_teardown,
                 CUTE_DFLT_TMOUT)
{
	const struct timespec exp = { 10, 0};

	cute_check_bool(etux_timer_is_armed(&etuxut_the_timer), is, false);
	etux_timer_arm_tspec(&etuxut_the_timer, &exp);
	cute_check_bool(etux_timer_is_armed(&etuxut_the_timer), is, true);
	etux_timer_cancel(&etuxut_the_timer);
	cute_check_bool(etux_timer_is_armed(&etuxut_the_timer), is, false);
}

static const struct timespec etuxut_clock[] = {
	{
		.tv_sec  = 0,
		.tv_nsec = 0
	},
	{
		.tv_sec  = 0,
		.tv_nsec = ETUX_TIMER_TICK_NSEC - 1
	},

#if ETUX_TIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = 0,
		.tv_nsec = ETUX_TIMER_TICK_NSEC
	},
	{
		.tv_sec  = 0,
		.tv_nsec = ETUX_TIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = 0,
		.tv_nsec = (ETUX_TIMER_TICKS_PER_SEC * ETUX_TIMER_TICK_NSEC) -
		           1
	},

	{
		.tv_sec  = 1,
		.tv_nsec = 0
	},
	{
		.tv_sec  = 1,
		.tv_nsec = ETUX_TIMER_TICK_NSEC - 1
	},
#if ETUX_TIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = 1,
		.tv_nsec = ETUX_TIMER_TICK_NSEC
	},
	{
		.tv_sec  = 1,
		.tv_nsec = ETUX_TIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = 1,
		.tv_nsec = (ETUX_TIMER_TICKS_PER_SEC * ETUX_TIMER_TICK_NSEC) -
		           1
	},

	{
		.tv_sec  = 365 * 24 * 60 * 60,
		.tv_nsec = 0,
	},
	{
		.tv_sec  = 365 * 24 * 60 * 60,
		.tv_nsec = ETUX_TIMER_TICK_NSEC - 1
	},
#if ETUX_TIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = 365 * 24 * 60 * 60,
		.tv_nsec = ETUX_TIMER_TICK_NSEC
	},
	{
		.tv_sec  = 365 * 24 * 60 * 60,
		.tv_nsec = ETUX_TIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = 365 * 24 * 60 * 60,
		.tv_nsec = (ETUX_TIMER_TICKS_PER_SEC * ETUX_TIMER_TICK_NSEC) -
		           1
	},

	{
		.tv_sec  = INT_MAX / 1000,
		.tv_nsec = 0,
	},
	{
		.tv_sec  = INT_MAX / 1000,
		.tv_nsec = ETUX_TIMER_TICK_NSEC - 1
	},
#if ETUX_TIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = INT_MAX / 1000,
		.tv_nsec = ETUX_TIMER_TICK_NSEC
	},
	{
		.tv_sec  = INT_MAX / 1000,
		.tv_nsec = ETUX_TIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = INT_MAX / 1000,
		.tv_nsec = (ETUX_TIMER_TICKS_PER_SEC * ETUX_TIMER_TICK_NSEC) -
		           1
	},

	{
		.tv_sec  = ((time_t)INT_MAX + 999) / 1000,
		.tv_nsec = 0,
	},
	{
		.tv_sec  = ((time_t)INT_MAX + 999) / 1000,
		.tv_nsec = ETUX_TIMER_TICK_NSEC - 1
	},
#if ETUX_TIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = ((time_t)INT_MAX + 999) / 1000,
		.tv_nsec = ETUX_TIMER_TICK_NSEC
	},
	{
		.tv_sec  = ((time_t)INT_MAX + 999) / 1000,
		.tv_nsec = ETUX_TIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = ((time_t)INT_MAX + 999) / 1000,
		.tv_nsec = (ETUX_TIMER_TICKS_PER_SEC * ETUX_TIMER_TICK_NSEC) -
		           1
	},

	{
		.tv_sec  = ETUX_TIMER_TICK_MAX >> ETUX_TIMER_TICK_SUBSEC_BITS,
		.tv_nsec = ETUX_TIMER_TICK_NSEC - 1
	},
#if ETUX_TIMER_TICK_SUBSEC_BITS > 0
	{
		.tv_sec  = ETUX_TIMER_TICK_MAX >> ETUX_TIMER_TICK_SUBSEC_BITS,
		.tv_nsec = ETUX_TIMER_TICK_NSEC
	},
	{
		.tv_sec  = ETUX_TIMER_TICK_MAX >> ETUX_TIMER_TICK_SUBSEC_BITS,
		.tv_nsec = ETUX_TIMER_TICK_NSEC + 1
	},
#endif
	{
		.tv_sec  = ETUX_TIMER_TICK_MAX >> ETUX_TIMER_TICK_SUBSEC_BITS,
		.tv_nsec = (ETUX_TIMER_TICK_MAX & ETUX_TIMER_TICK_SUBSEC_MASK) *
		           ETUX_TIMER_TICK_NSEC
	}
};

CUTE_TEST_STATIC(etuxut_timer_issue_msec,
                 etuxut_timer_setup,
                 etuxut_timer_teardown,
                 CUTE_DFLT_TMOUT)
{
	unsigned int e;
	unsigned int c;

	cute_check_sint(etux_timer_issue_msec(), equal, -1);
	cute_check_bool(etux_timer_is_armed(&etuxut_the_timer), is, false);

	for (e = 0; e < stroll_array_nr(etuxut_clock); e++) {
		struct timespec exp = etuxut_clock[e];

		etux_timer_arm_tspec(&etuxut_the_timer, &exp);
		cute_check_bool(etux_timer_is_armed(&etuxut_the_timer),
		                is,
		                true);

		for (c = 0; c < stroll_array_nr(etuxut_clock); c++) {
			struct timespec diff = exp;
			struct timespec clk = etuxut_clock[c];
			int             msec;

			diff.tv_nsec = stroll_round_upper(diff.tv_nsec,
			                                  ETUX_TIMER_TICK_NSEC);
			if (diff.tv_nsec >= 1000000000L) {
				if (diff.tv_sec < (ETUX_TIMER_TICK_MAX >>
				                   ETUX_TIMER_TICK_SUBSEC_BITS)) {
					diff.tv_sec += 1;
					diff.tv_nsec -= 1000000000L;
				}
				else {
					diff.tv_sec  = ETUX_TIMER_TICK_MAX >>
					               ETUX_TIMER_TICK_SUBSEC_BITS;
					diff.tv_nsec = (ETUX_TIMER_TICK_MAX &
					                ETUX_TIMER_TICK_SUBSEC_MASK)
					               * ETUX_TIMER_TICK_NSEC;
				}
			}
			if (utime_tspec_sub(&diff, &clk) > 0)
				msec = utime_msec_from_tspec_upper_clamp(&diff);
			else
				msec = 0;

			utilsut_expect_monotonic_now(&etuxut_clock[c]);
			cute_check_sint(etux_timer_issue_msec(), equal, msec);
			utilsut_expect_monotonic_now(NULL);
		}

		etux_timer_cancel(&etuxut_the_timer);
		cute_check_sint(etux_timer_issue_msec(), equal, -1);
	}
}

struct etuxut_timer {
	struct etux_timer         base;
	const struct timespec expire;
	int                   count;
	struct timespec       expected;
};

static struct etuxut_timer etuxut_timers[] = {
	{
		.expire.tv_sec  = 0,
		.expire.tv_nsec = 0
	},
	{
		.expire.tv_sec  = 0,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC - 1
	},
#if ETUX_TIMER_TICK_SUBSEC_BITS > 0
	{
		.expire.tv_sec  = 0,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC
	},
	{
		.expire.tv_sec  = 0,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC + 1
	},
#endif
	{
		.expire.tv_sec  = 0,
		.expire.tv_nsec = (ETUX_TIMER_TICKS_PER_SEC * ETUX_TIMER_TICK_NSEC) -
		                  1
	},

	{
		.expire.tv_sec  = 1,
		.expire.tv_nsec = 0
	},
	{
		.expire.tv_sec  = 1,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC - 1
	},
#if ETUX_TIMER_TICK_SUBSEC_BITS > 0
	{
		.expire.tv_sec  = 1,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC
	},
	{
		.expire.tv_sec  = 1,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC + 1
	},
#endif
	{
		.expire.tv_sec  = 1,
		.expire.tv_nsec = (ETUX_TIMER_TICKS_PER_SEC * ETUX_TIMER_TICK_NSEC) -
		                  1
	},

	{
		.expire.tv_sec  = 37 * 60,
		.expire.tv_nsec = 0
	},
	{
		.expire.tv_sec  = 37 * 60,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC - 1
	},
#if ETUX_TIMER_TICK_SUBSEC_BITS > 0
	{
		.expire.tv_sec  = 37 * 60,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC
	},
	{
		.expire.tv_sec  = 37 * 60,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC + 1
	},
#endif
	{
		.expire.tv_sec  = 37 * 60,
		.expire.tv_nsec = (ETUX_TIMER_TICKS_PER_SEC * ETUX_TIMER_TICK_NSEC) -
		                  1
	},

	{
		.expire.tv_sec  = 59 * 60,
		.expire.tv_nsec = 0
	},
	{
		.expire.tv_sec  = 59 * 60,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC - 1
	},
#if ETUX_TIMER_TICK_SUBSEC_BITS > 0
	{
		.expire.tv_sec  = 59 * 60,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC
	},
	{
		.expire.tv_sec  = 59 * 60,
		.expire.tv_nsec = ETUX_TIMER_TICK_NSEC + 1
	},
#endif
	{
		.expire.tv_sec  = 59 * 60,
		.expire.tv_nsec = (ETUX_TIMER_TICKS_PER_SEC * ETUX_TIMER_TICK_NSEC) -
		                  1
	},
};

static const struct timespec * etuxut_curr_clock;

static void
etuxut_timer_expire_arm(struct etux_timer * __restrict timer)
{
	struct etuxut_timer * tmr = (struct etuxut_timer *)timer;

	tmr->count--;

	cute_check_sint(etuxut_curr_clock->tv_sec,
	                equal,
	                tmr->expected.tv_sec);
	cute_check_sint(etuxut_curr_clock->tv_nsec,
	                equal,
	                tmr->expected.tv_nsec);
}

static void
etuxut_timer_setup_arm(void)
{
	unsigned int t;

	for (t = 0; t < stroll_array_nr(etuxut_timers); t++) {
		struct etuxut_timer * tmr = &etuxut_timers[t];

		etux_timer_init(&tmr->base, etuxut_timer_expire_arm);
	}
}

static void
etuxut_timer_teardown_arm(void)
{
	unsigned int t;

	for (t = 0; t < stroll_array_nr(etuxut_timers); t++) {
		struct etuxut_timer * tmr = &etuxut_timers[t];

		etux_timer_cancel(&tmr->base);
	}

	utilsut_expect_monotonic_now(NULL);
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(etuxut_timer_arm_tspec_assert)
{
	struct etux_timer         inval_tmr = ETUX_TIMER_INIT(inval_tmr, NULL);
	struct etux_timer         valid_tmr = ETUX_TIMER_INIT(valid_tmr,
	                                              etuxut_timer_expire);
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

	cute_expect_assertion(etux_timer_arm_tspec(NULL, &valid_exp));
	cute_expect_assertion(etux_timer_arm_tspec(&valid_tmr, NULL));
	cute_expect_assertion(etux_timer_arm_tspec(&inval_tmr, &valid_exp));
	cute_expect_assertion(etux_timer_arm_tspec(&valid_tmr, &sec_neg));
	cute_expect_assertion(etux_timer_arm_tspec(&valid_tmr, &nsec_over));
	cute_expect_assertion(etux_timer_arm_tspec(&valid_tmr, &nsec_neg));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(etuxut_timer_arm_tspec_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST_STATIC(etuxut_timer_arm_tspec,
                 etuxut_timer_setup_arm,
                 etuxut_timer_teardown_arm,
                 CUTE_DFLT_TMOUT)
{
	unsigned int       m;
	const unsigned int msecs[] = {
		/* Must be divisors of 1000 millisecond. */
		1, 2, 5, 8, 20, 25, 40, 50, 100, 200, 500, 1000
	};

	for (m = 0; m < stroll_array_nr(msecs); m++) {
		unsigned int          t;
		struct timespec       clk = { .tv_sec = 0, clk.tv_nsec = 0 };
		const struct timespec last_clk = {
			.tv_sec  = 60 * 60,
			.tv_nsec = 0
		};

		for (t = 0; t < stroll_array_nr(etuxut_timers); t++) {
			struct etuxut_timer * tmr = &etuxut_timers[t];

			utilsut_expect_monotonic_now(&clk);
			etux_timer_arm_tspec(&tmr->base, &tmr->expire);
			utilsut_expect_monotonic_now(NULL);
			cute_check_bool(etux_timer_is_armed(&tmr->base), is, true);

			tmr->count = 1;

			tmr->expected = tmr->expire;
			tmr->expected.tv_nsec =
				stroll_round_upper(tmr->expected.tv_nsec,
				                   ETUX_TIMER_TICK_NSEC);
			tmr->expected.tv_nsec =
				stroll_round_upper(tmr->expected.tv_nsec,
				                   msecs[m] * 1000000);
			if (tmr->expected.tv_nsec >= 1000000000L) {
				tmr->expected.tv_sec += 1;
				tmr->expected.tv_nsec -= 1000000000L;
			}
		}

		while (utime_tspec_before_eq(&clk, &last_clk)) {
			etuxut_curr_clock = &clk;
			utilsut_expect_monotonic_now(&clk);
			etux_timer_run();
			utilsut_expect_monotonic_now(NULL);

			utime_tspec_add_msec_clamp(&clk, (int)msecs[m]);
		}

		for (t = 0; t < stroll_array_nr(etuxut_timers); t++) {
			cute_check_bool(
				etux_timer_is_armed(&etuxut_timers[t].base),
				is,
				false);
			cute_check_sint(etuxut_timers[t].count, equal, 0);
		}
	}
}

#define ETUXUT_TIMER_REARM_SEC (90)

static struct etuxut_timer etuxut_timer_rearm;

static void
etuxut_timer_expire_rearm(struct etux_timer * __restrict timer)
{
	struct etuxut_timer * tmr = (struct etuxut_timer *)timer;

	tmr->count++;

	cute_check_sint(etuxut_curr_clock->tv_sec,
	                equal,
	                tmr->expected.tv_sec);
	cute_check_sint(etuxut_curr_clock->tv_nsec,
	                equal,
	                tmr->expected.tv_nsec);

	tmr->expected = *etux_timer_expiry_tspec(timer);
	utime_tspec_add_sec_clamp(&tmr->expected, ETUXUT_TIMER_REARM_SEC);

	etux_timer_arm_tspec(timer, &tmr->expected);
	cute_check_bool(etux_timer_is_armed(timer), is, true);
}

static void
etuxut_timer_setup_rearm(void)
{
	etux_timer_init(&etuxut_timer_rearm.base, etuxut_timer_expire_rearm);
}

static void
etuxut_timer_teardown_rearm(void)
{
	etux_timer_cancel(&etuxut_timer_rearm.base);
	utilsut_expect_monotonic_now(NULL);
}

CUTE_TEST_STATIC(etuxut_timer_rearm_tspec,
                 etuxut_timer_setup_rearm,
                 etuxut_timer_teardown_rearm,
                 CUTE_DFLT_TMOUT)
{
	unsigned int       m;
	const unsigned int msecs[] = {
		/* Must be divisors of 1000 millisecond. */
		1, 2, 5, 8, 20, 25, 40, 50, 100, 200, 500, 1000
	};

	for (m = 0; m < stroll_array_nr(msecs); m++) {
		struct timespec       clk = { .tv_sec = 0, clk.tv_nsec = 0 };
		const struct timespec last_clk = {
			.tv_sec  = 60 * 60,
			.tv_nsec = 0
		};

		etuxut_timer_rearm.count = 0;
		etuxut_timer_rearm.expected.tv_sec = 0;
		etuxut_timer_rearm.expected.tv_nsec = 0;
		utilsut_expect_monotonic_now(&clk);
		etux_timer_arm_tspec(&etuxut_timer_rearm.base,
		                 &etuxut_timer_rearm.expected);
		utilsut_expect_monotonic_now(NULL);
		cute_check_bool(etux_timer_is_armed(&etuxut_timer_rearm.base),
		                is,
		                true);

		while (utime_tspec_before(&clk, &last_clk)) {
			etuxut_curr_clock = &clk;
			utilsut_expect_monotonic_now(&clk);
			etux_timer_run();
			utilsut_expect_monotonic_now(NULL);

			utime_tspec_add_msec_clamp(&clk, (int)msecs[m]);
		}

		cute_check_bool(etux_timer_is_armed(&etuxut_timer_rearm.base),
		                is,
		                true);
		cute_check_sint(etuxut_timer_rearm.count,
		                equal,
		                last_clk.tv_sec / ETUXUT_TIMER_REARM_SEC);
		etux_timer_cancel(&etuxut_timer_rearm.base);
	}
}

static void
etuxut_timer_expire_cancel(struct etux_timer * __restrict timer)
{
	struct etuxut_timer * tmr = (struct etuxut_timer *)timer;

	tmr->count--;
	cute_check_sint(tmr->count, equal, 0);
}

static void
etuxut_timer_setup_cancel(void)
{
	unsigned int t;

	for (t = 0; t < stroll_array_nr(etuxut_timers); t++) {
		struct etuxut_timer * tmr = &etuxut_timers[t];

		etux_timer_init(&tmr->base, etuxut_timer_expire_cancel);
	}
}

#if defined(CONFIG_UTILS_ASSERT_API)

CUTE_TEST(etuxut_timer_cancel_assert)
{
	struct etux_timer inval_tmr = {
		.state = ETUX_TIMER_PEND_STAT,
		.expire = NULL
	};

	cute_expect_assertion(etux_timer_cancel(NULL));
	cute_expect_assertion(etux_timer_cancel(&inval_tmr));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

UTILSUT_NOASSERT_TEST(etuxut_timer_cancel_assert)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

CUTE_TEST_STATIC(etuxut_timer_cancel,
                 etuxut_timer_setup_cancel,
                 etuxut_timer_teardown_arm,
                 CUTE_DFLT_TMOUT)
{
	unsigned int       m;
	const unsigned int msecs[] = {
		/* Must be divisors of 1000 millisecond. */
		1, 2, 5, 8, 20, 25, 40, 50, 100, 200, 500, 1000
	};

	for (m = 0; m < stroll_array_nr(msecs); m++) {
		unsigned int          t;
		unsigned int          nr = stroll_array_nr(etuxut_timers);
		struct timespec       clk = { .tv_sec = 0, clk.tv_nsec = 0 };
		const struct timespec last_clk = {
			.tv_sec  = 60 * 60,
			.tv_nsec = 0
		};

		for (t = 0; t < nr; t++) {
			struct etuxut_timer * tmr = &etuxut_timers[t];

			utilsut_expect_monotonic_now(&clk);
			etux_timer_arm_tspec(&tmr->base, &tmr->expire);
			utilsut_expect_monotonic_now(NULL);
			cute_check_bool(etux_timer_is_armed(&tmr->base), is, true);

			tmr->count = 1;
		}

		utilsut_expect_monotonic_now(&clk);
		etux_timer_cancel(&etuxut_timers[0].base);
		utilsut_expect_monotonic_now(NULL);
		cute_check_bool(etux_timer_is_armed(&etuxut_timers[0].base),
		                is,
		                false);
		etuxut_timers[0].count = 0;

		utilsut_expect_monotonic_now(&clk);
		etux_timer_cancel(&etuxut_timers[nr / 2].base);
		utilsut_expect_monotonic_now(NULL);
		cute_check_bool(etux_timer_is_armed(&etuxut_timers[nr / 2].base),
		                is,
		                false);
		etuxut_timers[nr / 2].count = 0;

		utilsut_expect_monotonic_now(&clk);
		etux_timer_cancel(&etuxut_timers[nr - 1].base);
		utilsut_expect_monotonic_now(NULL);
		cute_check_bool(etux_timer_is_armed(&etuxut_timers[nr - 1].base),
		                is,
		                false);
		etuxut_timers[nr - 1].count = 0;

		while (utime_tspec_before_eq(&clk, &last_clk)) {
			etuxut_curr_clock = &clk;
			utilsut_expect_monotonic_now(&clk);
			etux_timer_run();
			utilsut_expect_monotonic_now(NULL);

			utime_tspec_add_msec_clamp(&clk, (int)msecs[m]);
		}

		for (t = 0; t < nr; t++) {
			cute_check_bool(
				etux_timer_is_armed(&etuxut_timers[t].base),
				is,
				false);
			cute_check_sint(etuxut_timers[t].count, equal, 0);
		}
	}
}

CUTE_GROUP(etuxut_timer_group) = {
	CUTE_REF(etuxut_timer_monotonic_now),

	CUTE_REF(etuxut_timer_is_armed_assert),
	CUTE_REF(etuxut_timer_is_armed),

	CUTE_REF(etuxut_timer_issue_msec),

	CUTE_REF(etuxut_timer_arm_tspec_assert),
	CUTE_REF(etuxut_timer_arm_tspec),
	CUTE_REF(etuxut_timer_rearm_tspec),

	CUTE_REF(etuxut_timer_cancel_assert),
	CUTE_REF(etuxut_timer_cancel),
};

CUTE_SUITE_STATIC(etuxut_timer_suite,
                  etuxut_timer_group,
                  CUTE_NULL_SETUP,
                  CUTE_NULL_TEARDOWN,
                  CUTE_DFLT_TMOUT);

CUTE_MAIN(etuxut_timer_suite, "eTux timer", UTILS_VERSION_STRING)
