/**
 * @file      poll.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      04 Oct 2021
 * @copyright GNU Public License v3
 *
 * Polling interface
 *
 * @defgroup poll Polling
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
#ifndef _UTILS_POLL_H
#define _UTILS_POLL_H

#include <utils/cdefs.h>
#include <stdint.h>
#include <sys/epoll.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __upoll_nonull(_arg_index, ...)

#define upoll_assert(_expr) \
	uassert("upoll", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __upoll_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define upoll_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

struct upoll;
struct upoll_worker;

typedef int (upoll_dispatch_fn)(struct upoll_worker * worker,
                                uint32_t              events,
                                const struct upoll *  poller);

struct upoll_worker {
	upoll_dispatch_fn * dispatch;
	uint32_t            user;
	uint32_t            kernel;
};

static inline void __upoll_nonull(1) __nothrow
upoll_enable_watch(struct upoll_worker * worker, uint32_t events)
{
	upoll_assert(worker);
	upoll_assert(worker->dispatch);
	upoll_assert(events);
	upoll_assert(!(events & ~(EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI)));

	worker->user |= events;
}

static inline void __upoll_nonull(1) __nothrow
upoll_disable_watch(struct upoll_worker * worker, uint32_t events)
{
	upoll_assert(worker);
	upoll_assert(worker->dispatch);
	upoll_assert(events);
	upoll_assert(!(events & ~(EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI)));

	worker->user &= ~events;
}

struct upoll {
	unsigned int nr;
	int          fd;
};

static inline int __upoll_nonull(1) __nothrow __pure
upoll_get_fd(const struct upoll * poller)
{
	upoll_assert(poller);
	upoll_assert(poller->fd >= 0);
	upoll_assert(poller->nr > 0);

	return poller->fd;
}

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __upoll_nonull(1) __nothrow
upoll_unregister(const struct upoll * __restrict poller, int fd)
{
	upoll_assert(poller);
	upoll_assert(poller->fd >= 0);
	upoll_assert(poller->nr > 0);
	upoll_assert(fd >= 0);

	/*
	 * Cannot fail if proper arguments are given...
	 * See <linux>/fs/eventpoll.c
	 */
	upoll_assert(!epoll_ctl(poller->fd, EPOLL_CTL_DEL, fd, NULL));
}

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __upoll_nonull(1) __nothrow
upoll_unregister(const struct upoll * __restrict poller, int fd)
{
	epoll_ctl(poller->fd, EPOLL_CTL_DEL, fd, NULL);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

extern void
upoll_apply(const struct upoll * __restrict poller,
            int                             fd,
            struct upoll_worker *           worker)
	__upoll_nonull(1, 3) __leaf __nothrow;

extern int
upoll_register(const struct upoll * __restrict poller,
               int                             fd,
               uint32_t                        events,
               struct upoll_worker *           worker) 
	__upoll_nonull(1, 4) __leaf __nothrow;

extern int
upoll_process(const struct upoll * poller, int tmout) __upoll_nonull(1);

#if defined(CONFIG_UTILS_TIMER)

extern int
upoll_process_with_timers(const struct upoll * poller) __upoll_nonull(1);

#endif /* defined(CONFIG_UTILS_TIMER) */

extern int
upoll_open(struct upoll * poller, unsigned int nr)
	__upoll_nonull(1) __leaf __nothrow;

extern void
upoll_close(const struct upoll * poller) __upoll_nonull(1) __leaf;

#endif /* _UTILS_POLL_H */
