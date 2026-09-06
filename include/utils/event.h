/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2026 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Eventfd object interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      06 Sep 2026
 * @copyright Copyright (C) 2017-2026 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_EVENT_H
#define _UTILS_EVENT_H

#include <utils/fd.h>
#include <sys/eventfd.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define uevt_assert_api(_expr) \
	stroll_assert("utils:uevt", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define uevt_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(2) __warn_result
int
uevt_read(int fd, eventfd_t * value)
{
	uevt_assert_api(fd >= 0);
	uevt_assert_api(value);

	int err;

	err = eventfd_read(fd, value);
	if (!err)
		return 0;

	uevt_assert_api(errno != EIO);

	return -errno;
}

static inline __warn_result
int
uevt_write(int fd, eventfd_t value)
{
	uevt_assert_api(fd >= 0);
	uevt_assert_api(value);
	uevt_assert_api(value < UINT64_MAX);

	int err;

	err = eventfd_write(fd, value);
	if (!err)
		return 0;

	uevt_assert_api(errno != EIO);
	uevt_assert_api(errno != EDESTADDRREQ);
	uevt_assert_api(errno != EDQUOT);
	uevt_assert_api(errno != EFBIG);
	uevt_assert_api(errno != ENOSPC);
	uevt_assert_api(errno != EPERM);
	uevt_assert_api(errno != EPIPE);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow
int
uevt_open(unsigned int initval, int flags)
{
	uevt_assert_api(!(flags &
	                  ~(EFD_SEMAPHORE | EFD_NONBLOCK | EFD_CLOEXEC)));

	int fd;

	fd = eventfd(initval, flags);
	if (fd < 0) {
		uevt_assert_api(errno != EINVAL);

		return -errno;
	}

	return fd;
}

static inline
int
uevt_close(int fd)
{
	uevt_assert_api(fd >= 0);

	int ret;

	ret = ufd_close(fd);

	uevt_assert_api(!ret || (ret == -EINTR));

	return ret;
}

#endif /* _UTILS_EVENT_H */
