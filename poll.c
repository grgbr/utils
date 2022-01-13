/**
 * @file      poll.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      04 Oct 2021
 * @copyright GNU Public License v3
 *
 * Polling implementation
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
#include "utils/poll.h"
#include "utils/fd.h"
#include <string.h>

void
upoll_apply(const struct upoll * __restrict poller,
            int                             fd,
            struct upoll_worker *           worker)
{
	upoll_assert(poller);
	upoll_assert(poller->fd >= 0);
	upoll_assert(poller->nr > 0);
	upoll_assert(fd >= 0);
	upoll_assert(worker->user);
	upoll_assert(!(worker->user &
	               ~(EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI)));
	upoll_assert(worker);
	upoll_assert(worker->dispatch);
	upoll_assert(worker->kernel);

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
		upoll_assert(!err);

		worker->kernel = worker->user;
	}
}

int
upoll_register(const struct upoll * __restrict poller,
               int                             fd,
               uint32_t                        events,
               struct upoll_worker *           worker)
{
	upoll_assert(poller);
	upoll_assert(poller->fd >= 0);
	upoll_assert(poller->nr > 0);
	upoll_assert(fd >= 0);
	upoll_assert(events);
	upoll_assert(!(events & ~(EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI)));
	upoll_assert(worker);
	upoll_assert(worker->dispatch);

	struct epoll_event evt = {
		.events   = events,
		.data.ptr = (void *)worker
	};

	if (epoll_ctl(poller->fd, EPOLL_CTL_ADD, fd, &evt)) {
		upoll_assert(errno != EBADF);
		upoll_assert(errno != EEXIST);
		upoll_assert(errno != EINVAL);
		upoll_assert(errno != ELOOP);
		upoll_assert(errno != ENOENT);
		upoll_assert(errno != EPERM);

		return -errno;
	}

	worker->user = events;
	worker->kernel = events;

	return 0;
}

static int __upoll_nonull(1, 2)
upoll_process_dispatch(const struct upoll *       poller,
                       const struct epoll_event * events,
                       unsigned int               nr)
{
	upoll_assert(poller);
	upoll_assert(poller->fd >= 0);
	upoll_assert(poller->nr > 0);
	upoll_assert(events);
	upoll_assert(nr);
	upoll_assert(nr <= poller->nr);

	unsigned int e;

	for (e = 0; e < nr; e++) {
		const struct epoll_event * evt = &events[e];
		struct upoll_worker *      wk = evt->data.ptr;
		int                        ret;

		upoll_assert(wk);
		upoll_assert(wk->dispatch);

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
	upoll_assert(poller);
	upoll_assert(poller->fd >= 0);
	upoll_assert(poller->nr > 0);

	int                ret;
	struct epoll_event evts[poller->nr];

	ret = epoll_wait(poller->fd, evts, poller->nr, tmout);
	if (ret < 0) {
		upoll_assert(errno != EBADF);
		upoll_assert(errno != EFAULT);
		upoll_assert(errno != EINVAL);

		return -errno;
	}

	if (!ret && (tmout >= 0))
		return -ETIME;

	return upoll_process_dispatch(poller, evts, (unsigned int)ret);
}

#if defined(CONFIG_UTILS_TIMER)

#include <utils/timer.h>

int
upoll_process_with_timers(const struct upoll * poller)
{
	upoll_assert(poller);
	upoll_assert(poller->fd >= 0);
	upoll_assert(poller->nr > 0);

	int                ret;
	int                tmout;
	struct epoll_event evts[poller->nr];

	tmout = (int)utimer_issue_msec();

	ret = epoll_wait(poller->fd, evts, poller->nr, tmout);
	if (!ret) {
		/* Timer(s) expired before any activity. */
		upoll_assert(tmout >= 0);
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
		upoll_assert(errno != EBADF);
		upoll_assert(errno != EFAULT);
		upoll_assert(errno != EINVAL);

		return -errno;
	}

	/* Activity has been detected before a timer expired. */
	return upoll_process_dispatch(poller, evts, (unsigned int)ret);
}

#endif /* defined(CONFIG_UTILS_TIMER) */

int
upoll_open(struct upoll * poller, unsigned int nr)
{
	upoll_assert(poller);
	upoll_assert(nr);

	int fd;

	fd = epoll_create1(EPOLL_CLOEXEC);
	if (fd < 0) {
		upoll_assert(errno != EINVAL);

		return -errno;
	}

	poller->fd = fd;
	poller->nr = nr;

	return 0;
}

void
upoll_close(const struct upoll * poller)
{
	upoll_assert(poller);
	upoll_assert(poller->fd >= 0);
	upoll_assert(poller->nr > 0);

	int err __unused;

	err = ufd_close(poller->fd);

	upoll_assert(err != -EBADF);
	upoll_assert(err != -ENOSPC);
	upoll_assert(err != -EDQUOT);

	return;
}
