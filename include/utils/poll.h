/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Polling interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      04 Oct 2021
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_POLL_H
#define _UTILS_POLL_H

#include <utils/cdefs.h>
#include <stdint.h>
#include <sys/epoll.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define upoll_assert_api(_expr) \
	stroll_assert("utils:upoll", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define upoll_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

struct upoll;
struct upoll_worker;

typedef int (upoll_dispatch_fn)(struct upoll_worker *,
                                uint32_t,
                                const struct upoll *);

struct upoll_worker {
	upoll_dispatch_fn * dispatch;
	uint32_t            user;
	uint32_t            kernel;
};

static inline __utils_nonull(1) __utils_nothrow
void
upoll_enable_watch(struct upoll_worker * __restrict worker, uint32_t events)
{
	upoll_assert_api(worker);
	upoll_assert_api(worker->dispatch);
	upoll_assert_api(events);
	upoll_assert_api(!(events &
	                   ~(EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI)));

	worker->user |= events;
}

static inline __utils_nonull(1) __utils_nothrow
void
upoll_disable_watch(struct upoll_worker * __restrict worker, uint32_t events)
{
	upoll_assert_api(worker);
	upoll_assert_api(worker->dispatch);
	upoll_assert_api(events);
	upoll_assert_api(!(events &
	                   ~(EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI)));

	worker->user &= ~events;
}

struct upoll {
	unsigned int nr;
	int          fd;
};

static inline __utils_nonull(1) __utils_nothrow __utils_pure
int
upoll_get_fd(const struct upoll * __restrict poller)
{
	upoll_assert_api(poller);
	upoll_assert_api(poller->fd >= 0);
	upoll_assert_api(poller->nr > 0);

	return poller->fd;
}

extern void
upoll_apply(const struct upoll * __restrict  poller,
            int                              fd,
            struct upoll_worker * __restrict worker)
	__utils_nonull(1, 3) __utils_nothrow __leaf;

extern int
upoll_register(const struct upoll * __restrict poller,
               int                             fd,
               uint32_t                        events,
               struct upoll_worker *           worker) 
	__utils_nonull(1, 4) __utils_nothrow __leaf;

#if defined(CONFIG_UTILS_ASSERT_API)

static inline __utils_nonull(1) __utils_nothrow
void
upoll_unregister(const struct upoll * __restrict poller, int fd)
{
	upoll_assert_api(poller);
	upoll_assert_api(poller->fd >= 0);
	upoll_assert_api(poller->nr > 0);
	upoll_assert_api(fd >= 0);

	/*
	 * Cannot fail if proper arguments are given...
	 * See <linux>/fs/eventpoll.c
	 */
	upoll_assert_api(!epoll_ctl(poller->fd, EPOLL_CTL_DEL, fd, NULL));
}

#else /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(1) __utils_nothrow
void
upoll_unregister(const struct upoll * __restrict poller, int fd)
{
	epoll_ctl(poller->fd, EPOLL_CTL_DEL, fd, NULL);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

extern int
upoll_process(const struct upoll * poller, int tmout)
	__utils_nonull(1);

#if defined(CONFIG_UTILS_TIMER)

extern int
upoll_process_with_timers(const struct upoll * poller) __utils_nonull(1);

#endif /* defined(CONFIG_UTILS_TIMER) */

extern int
upoll_open(struct upoll * __restrict poller, unsigned int nr)
	__utils_nonull(1) __utils_nothrow __leaf;

extern void
upoll_close(const struct upoll * __restrict poller) __utils_nonull(1) __leaf;

#endif /* _UTILS_POLL_H */
