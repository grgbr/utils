/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/poll.h"
#include "utils/fd.h"
#include <string.h>

void
upoll_apply(const struct upoll * __restrict poller,
            int                             fd,
            struct upoll_worker *           worker)
{
	upoll_assert_api(poller);
	upoll_assert_intern(poller->fd >= 0);
	upoll_assert_intern(poller->nr > 0);
	upoll_assert_intern(poller->nr <= INT_MAX);
	upoll_assert_api(fd >= 0);
	upoll_assert_api(worker->user);
	upoll_assert_api(!(worker->user &
	                   ~((uint32_t)
	                     EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI)));
	upoll_assert_api(worker);
	upoll_assert_api(worker->dispatch);
	upoll_assert_intern(worker->kernel);

	if (worker->user != worker->kernel) {
		int                err __unused;
		struct epoll_event evt = {
			.events   = worker->user,
			.data.ptr = (void *)worker
		};

		/*
		 * Cannot fail if proper arguments are given...
		 * See <linux>/fs/eventpoll.c
		 */
		err = epoll_ctl(poller->fd, EPOLL_CTL_MOD, fd, &evt);
		upoll_assert_api(!err);

		worker->kernel = worker->user;
	}
}

int
upoll_register(const struct upoll * __restrict  poller,
               int                              fd,
               uint32_t                         events,
               struct upoll_worker * __restrict worker)
{
	upoll_assert_api(poller);
	upoll_assert_intern(poller->fd >= 0);
	upoll_assert_intern(poller->nr > 0);
	upoll_assert_intern(poller->nr <= INT_MAX);
	upoll_assert_api(fd >= 0);
	upoll_assert_api(events);
	upoll_assert_api(!(events &
	                   ~((uint32_t)
	                     EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI)));
	upoll_assert_api(worker);
	upoll_assert_api(worker->dispatch);

	struct epoll_event evt = {
		.events   = events,
		.data.ptr = (void *)worker
	};

	if (epoll_ctl(poller->fd, EPOLL_CTL_ADD, fd, &evt)) {
		upoll_assert_intern(errno != EBADF);
		upoll_assert_api(errno != EEXIST);
		upoll_assert_intern(errno != EINVAL);
		upoll_assert_api(errno != ELOOP);
		upoll_assert_intern(errno != ENOENT);
		upoll_assert_api(errno != EPERM);

		return -errno;
	}

	worker->user = events;
	worker->kernel = events;

	return 0;
}

static __utils_nonull(1, 2)
int
upoll_process_dispatch(const struct upoll *                  poller,
                       const struct epoll_event * __restrict events,
                       unsigned int                          nr)
{
	upoll_assert_api(poller);
	upoll_assert_intern(poller->fd >= 0);
	upoll_assert_intern(poller->nr > 0);
	upoll_assert_intern(poller->nr <= INT_MAX);
	upoll_assert_api(events);
	upoll_assert_api(nr);
	upoll_assert_api(nr <= poller->nr);

	unsigned int e;

	for (e = 0; e < nr; e++) {
		const struct epoll_event * evt = &events[e];
		struct upoll_worker *      wk = evt->data.ptr;
		int                        ret;

		upoll_assert_intern(wk);
		upoll_assert_intern(wk->dispatch);

		wk->user = wk->kernel;

		ret = wk->dispatch(wk, evt->events, poller);
		if (ret)
			return ret;
	}

	return 0;
}

int
upoll_process(const struct upoll * poller, int tmout)
{
	upoll_assert_api(poller);
	upoll_assert_intern(poller->fd >= 0);
	upoll_assert_intern(poller->nr > 0);
	upoll_assert_intern(poller->nr <= INT_MAX);

	int                ret;
	struct epoll_event evts[poller->nr];

	ret = epoll_wait(poller->fd, evts, (int)poller->nr, tmout);
	if (ret < 0) {
		upoll_assert_intern(errno != EBADF);
		upoll_assert_intern(errno != EFAULT);
		upoll_assert_intern(errno != EINVAL);

		return -errno;
	}

	if (!ret && (tmout >= 0))
		return -ETIME;

	return upoll_process_dispatch(poller, evts, (unsigned int)ret);
}

#if defined(CONFIG_UTILS_TIMER)

#include "utils/timer.h"

int
upoll_process_with_timers(const struct upoll * poller)
{
	upoll_assert_api(poller);
	upoll_assert_intern(poller->fd >= 0);
	upoll_assert_intern(poller->nr > 0);
	upoll_assert_intern(poller->nr <= INT_MAX);

	int                ret;
	int                tmout;
	struct epoll_event evts[poller->nr];

	tmout = (int)utimer_issue_msec();

	ret = epoll_wait(poller->fd, evts, (int)poller->nr, tmout);
	if (!ret) {
		/* Timer(s) expired before any activity. */
		upoll_assert_intern(tmout >= 0);
		utimer_run();

		return 0;
	}

	if (!tmout)
		/*
		 * Always run timer list if zero timeout was computed, i.e.,
		 * when it contains at leas one timer that has passed its expiry
		 * date.
		 */
		utimer_run();

	if (ret < 0) {
		upoll_assert_intern(errno != EBADF);
		upoll_assert_intern(errno != EFAULT);
		upoll_assert_intern(errno != EINVAL);

		return -errno;
	}

	/* Activity has been detected before a timer expired. */
	return upoll_process_dispatch(poller, evts, (unsigned int)ret);
}

#endif /* defined(CONFIG_UTILS_TIMER) */

int
upoll_open(struct upoll * __restrict poller, unsigned int nr)
{
	upoll_assert_api(poller);
	upoll_assert_api(nr);
	upoll_assert_api(nr <= INT_MAX);

	int fd;

	fd = epoll_create1(EPOLL_CLOEXEC);
	if (fd < 0) {
		upoll_assert_intern(errno != EINVAL);

		return -errno;
	}

	poller->fd = fd;
	poller->nr = nr;

	return 0;
}

void
upoll_close(const struct upoll * __restrict poller)
{
	upoll_assert_api(poller);
	upoll_assert_intern(poller->fd >= 0);
	upoll_assert_intern(poller->nr > 0);
	upoll_assert_intern(poller->nr <= INT_MAX);

	int err __unused;

	err = ufd_close(poller->fd);

	upoll_assert_intern(err != -ENOSPC);
	upoll_assert_intern(err != -EDQUOT);

	return;
}
