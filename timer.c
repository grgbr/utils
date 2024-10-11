/**
 * @file      timer.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2021
 * @copyright GNU Public License v3
 *
 * Timers implementation
 *
 * @defgroup timer Timers
 *
 * This file is part of Utils
 *
 * Copyright (C) 2021 Grégor Boirie <gregor.boirie@free.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "utils/timer.h"

static struct stroll_dlist_node utimer_list = STROLL_DLIST_INIT(utimer_list);

void
utimer_arm(struct utimer * timer)
{
	utimer_assert_timer(timer);
	utime_assert_tspec(&timer->date);

	struct stroll_dlist_node * node;
        const struct timespec *    date = &timer->date;

	stroll_dlist_remove_init(&timer->node);

	for (node = stroll_dlist_prev(&utimer_list);
	     node != &utimer_list;
	     node = stroll_dlist_prev(node)) {
		const struct utimer * t = stroll_dlist_entry(node,
		                                             struct utimer,
		                                             node);
		if (utime_tspec_after_eq(date, &t->date))
			break;
	}

	stroll_dlist_append(node, &timer->node);
}

void
utimer_arm_msec(struct utimer * timer, unsigned long msec)
{
	utimer_assert_timer(timer);
	utime_assert(msec);

	utime_monotonic_now(&timer->date);
	utime_tspec_add_msec(&timer->date, msec);
	utimer_arm(timer);
}

void
utimer_arm_sec(struct utimer * timer, unsigned long sec)
{
	utimer_assert_timer(timer);
	utime_assert(sec);

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

		return umax(utime_tspec_diff_msec(tspec, &now), 0L);
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
