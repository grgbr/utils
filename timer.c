/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/timer.h"

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define utimer_assert_intern(_expr) \
	stroll_assert("utils:utimer", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define utimer_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

static struct stroll_dlist_node utimer_list = STROLL_DLIST_INIT(utimer_list);

static __utils_nonull(1, 2) __utils_pure __utils_nothrow
int
utimer_cmp(const struct stroll_dlist_node * __restrict first,
           const struct stroll_dlist_node * __restrict second,
           void *                                      data __unused)
{
	utimer_assert_intern(first);
	utimer_assert_intern(second);

	const struct utimer * fst = stroll_dlist_entry(first,
	                                               struct utimer,
	                                               node);
	const struct utimer * snd = stroll_dlist_entry(second,
	                                               struct utimer,
	                                               node);

	return utime_tspec_cmp(&fst->date, &snd->date);
}

void
utimer_arm(struct utimer * __restrict timer)
{
	utimer_assert_timer(timer);
	utime_assert_tspec(&timer->date);

	stroll_dlist_remove_init(&timer->node);
	stroll_dlist_insert_inorder_back(&utimer_list,
	                                 &timer->node,
	                                 utimer_cmp,
	                                 NULL);
}

void
utimer_arm_msec(struct utimer * __restrict timer, unsigned long msec)
{
	utimer_assert_timer(timer);
	utimer_assert_api(msec);

	utime_monotonic_now(&timer->date);
	utime_tspec_add_msec(&timer->date, msec);
	utimer_arm(timer);
}

void
utimer_arm_sec(struct utimer * __restrict timer, unsigned long sec)
{
	utimer_assert_timer(timer);
	utimer_assert_api(sec);

	utime_monotonic_now(&timer->date);
	utime_tspec_add_sec(&timer->date, sec);
	utimer_arm(timer);
}

const struct timespec *
utimer_issue_date(void)
{
	if (!stroll_dlist_empty(&utimer_list)) {
		const struct utimer * timer;

		timer = stroll_dlist_entry(stroll_dlist_next(&utimer_list),
		                           struct utimer,
		                           node);
		utimer_assert_timer(timer);
		utime_assert_tspec(&timer->date);

		return &timer->date;
	}

	return NULL;
}

long
utimer_issue_msec(void)
{
	const struct timespec * tspec;

	tspec = utimer_issue_date();
	if (tspec) {
		struct timespec now;

		utime_monotonic_now(&now);

		return stroll_max(utime_tspec_diff_msec(tspec, &now), 0L);
	}

	return -1;
}

void
utimer_run(void)
{
	struct timespec now;

	utime_monotonic_now(&now);

	while (!stroll_dlist_empty(&utimer_list)) {
		struct utimer * timer;

		timer = stroll_dlist_entry(stroll_dlist_next(&utimer_list),
		                           struct utimer,
		                           node);
		utimer_assert_timer(timer);

		if (utime_tspec_after_eq(&timer->date, &now))
			break;

		stroll_dlist_remove_init(&timer->node);
		timer->expire(timer);
	}
}
