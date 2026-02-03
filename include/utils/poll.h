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

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define upoll_assert_intern(_expr) \
	stroll_assert("utils:upoll", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define upoll_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

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

static inline __utils_nonull(1) __utils_pure __utils_nothrow
uint32_t
upoll_watched_events(const struct upoll_worker * __restrict worker)
{
	upoll_assert_api(worker);
	upoll_assert_api(worker->dispatch);
	upoll_assert_api(!(worker->user &
	                   ~((uint32_t)
	                     (EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI))));

	return worker->user;
}

static inline __utils_nonull(1) __utils_nothrow
void
upoll_setup_watch(struct upoll_worker * __restrict worker, uint32_t events)
{
	upoll_assert_api(worker);
	upoll_assert_api(worker->dispatch);
	upoll_assert_api(events);
	upoll_assert_api(!(events &
	                   ~((uint32_t)
	                     (EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI))));

	worker->user = events;
}

static inline __utils_nonull(1) __utils_nothrow
void
upoll_enable_watch(struct upoll_worker * __restrict worker, uint32_t events)
{
	upoll_assert_api(worker);
	upoll_assert_api(worker->dispatch);
	upoll_assert_api(events);
	upoll_assert_api(!(events &
	                   ~((uint32_t)
	                     (EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI))));

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
	                   ~((uint32_t)
	                     (EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI))));

	worker->user &= ~events;
}

struct upoll {
	unsigned int         nr;
	int                  fd;
	struct epoll_event * events;
};

static inline __utils_nonull(1) __utils_nothrow __utils_pure
int
upoll_get_fd(const struct upoll * __restrict poller)
{
	upoll_assert_api(poller);
	upoll_assert_intern(poller->fd >= 0);
	upoll_assert_intern(poller->nr > 0);
	upoll_assert_intern(poller->nr <= INT_MAX);
	upoll_assert_intern(poller->events);

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

extern int
upoll_register_dispatch(const struct upoll * __restrict poller,
                        int                             fd,
                        uint32_t                        events,
                        struct upoll_worker *           worker,
                        upoll_dispatch_fn *             dispatch)
	__utils_nonull(1, 4, 5) __utils_nothrow __leaf;

static inline __utils_nonull(1) __utils_nothrow
void
upoll_unregister(const struct upoll * __restrict poller, int fd)
{
	upoll_assert_api(poller);
	upoll_assert_intern(poller->fd >= 0);
	upoll_assert_intern(poller->nr > 0);
	upoll_assert_intern(poller->nr <= INT_MAX);
	upoll_assert_intern(poller->events);
	upoll_assert_api(fd >= 0);

	int err __unused;

	/*
	 * Cannot fail if proper arguments are given...
	 * See <linux>/fs/eventpoll.c
	 */
	err = epoll_ctl(poller->fd, EPOLL_CTL_DEL, fd, NULL);
	upoll_assert_api(!err);
}

extern int
upoll_dispatch(const struct upoll * poller, unsigned int nr)
	__utils_nonull(1);

extern int
upoll_wait(const struct upoll * __restrict poller, int tmout)
	__utils_nonull(1);

extern int
upoll_process(const struct upoll * poller, int tmout)
	__utils_nonull(1);

extern int
upoll_open(struct upoll * __restrict poller, unsigned int nr)
	__utils_nonull(1) __utils_nothrow __leaf;

extern void
upoll_close(const struct upoll * __restrict poller) __utils_nonull(1) __leaf;

#endif /* _UTILS_POLL_H */
