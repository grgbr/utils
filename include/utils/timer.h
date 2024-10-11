/**
 * @file      timer.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2021
 * @copyright GNU Public License v3
 *
 * Timers interface
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
#ifndef _UTILS_TIMER_H
#define _UTILS_TIMER_H

#include <utils/time.h>
#include <stroll/dlist.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __utimer_nonull(_arg_index, ...)

#define __utimer_pure

#define utimer_assert(_expr) \
	uassert("utimer", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __utimer_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define __utimer_pure \
	__pure

#define utimer_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

struct utimer;

typedef void (utimer_expire_fn)(struct utimer * timer);

struct utimer {
	struct stroll_dlist_node node;
	struct timespec          date;
	utimer_expire_fn *       expire;
};

#define UTIMER_INIT(_timer) \
	{ .node   = STROLL_DLIST_INIT((_timer).node) }

#define utimer_assert_timer(_timer) \
	utimer_assert(_timer); \
	utimer_assert((_timer)->expire)

static inline struct timespec * __utimer_nonull(1) __utimer_pure  __nothrow
utimer_expiry_date(const struct utimer * timer)
{
	utimer_assert_timer(timer);

	return (struct timespec *)&timer->date;
}

static inline bool __utimer_nonull(1) __utimer_pure __nothrow
utimer_is_armed(const struct utimer * timer)
{
	utimer_assert_timer(timer);

	return !stroll_dlist_empty(&timer->node);
}

static inline void __utimer_nonull(1) __nothrow
utimer_cancel(struct utimer * timer)
{
	utimer_assert_timer(timer);
	utime_assert_tspec(&timer->date);

	stroll_dlist_remove_init(&timer->node);
}

static inline void __utimer_nonull(1, 2) __nothrow
utimer_setup(struct utimer * __restrict timer,
             utimer_expire_fn *         expire)
{
	utimer_assert(timer);
	utimer_assert(expire);

	timer->expire = expire;
}

static inline void __utimer_nonull(1) __nothrow
utimer_init(struct utimer * timer)
{
	utimer_assert(timer);

	stroll_dlist_init(&timer->node);
}

extern void
utimer_arm(struct utimer * timer) __utimer_nonull(1) __leaf __nothrow;

extern void
utimer_arm_msec(struct utimer * timer, unsigned long msec)
	__utimer_nonull(1) __nothrow;

extern void
utimer_arm_sec(struct utimer * timer, unsigned long sec)
	__utimer_nonull(1) __nothrow;

extern const struct timespec *
utimer_issue_date(void) __utimer_pure __leaf __nothrow;

extern long
utimer_issue_msec(void) __nothrow;

extern void utimer_run(void) __nothrow;

#endif /* _UTILS_TIMER_H */
