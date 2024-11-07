/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/thread.h"
#include "utils/time.h"
#include <stdlib.h>
#include <errno.h>

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#define uthr_assert_intern(_expr) \
	stroll_assert("utils:uthr", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define uthr_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

int
uthr_timed_wait_cond_msec(struct uthr_cond * __restrict  cond,
                          struct uthr_mutex * __restrict mutex,
                          unsigned int                   msec)
{
	uthr_assert_api(cond);
	uthr_assert_api(mutex);

	if (msec) {
		struct timespec tmout;

		uthr_cond_now(cond, &tmout);
		utime_tspec_add_msec_clamp(&tmout, msec);

		return uthr_timed_wait_cond(cond, mutex, &tmout);
	}

	return -ETIMEDOUT;
}

int
uthr_init_cond(struct uthr_cond * __restrict cond, clockid_t clock)
{
	uthr_assert_api(cond);

	int                ret;
	pthread_condattr_t attr;

	ret = pthread_condattr_init(&attr);
	uthr_assert_intern(ret != EINVAL);
	if (ret)
		return -ret;

	ret = pthread_condattr_setclock(&attr, clock);
	if (ret)
		goto destroy;

	ret = pthread_cond_init(&cond->pthread, &attr);
	uthr_assert_api(ret != EBUSY);
	uthr_assert_intern(ret != EINVAL);

	cond->clock = clock;

destroy:
	pthread_condattr_destroy(&attr);

	return -ret;
}
