/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Pipe interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      27 Oct 2021
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_PIPE_H
#define _UTILS_PIPE_H

#include <utils/fd.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define upipe_assert_api(_expr) \
	stroll_assert("utils:upipe", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define upipe_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#define UPIPE_READ_END  (0U)
#define UPIPE_WRITE_END (1U)
#define UPIPE_END_NR    (2U)

static inline __utils_nonull(2) __warn_result
ssize_t
upipe_read(int fd, char * __restrict data, size_t size)
{
	ssize_t ret;

	ret = ufd_read(fd, data, size);
	if (ret >= 0)
		return ret;

	upipe_assert_api(errno != EIO);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow __warn_result
int
upipe_open_anon(int fds[UPIPE_END_NR], int flags)
{
	upipe_assert_api(fds);
	upipe_assert_api(!(flags & ~(O_CLOEXEC | O_DIRECT | O_NONBLOCK)));

	if (!pipe2(fds, flags))
		return 0;

	upipe_assert_api(errno != EFAULT);
	upipe_assert_api(errno != EINVAL);

	return -errno;
}

static inline
int
upipe_close(int fd)
{
	upipe_assert_api(fd >= 0);

	int ret;
	
	ret = ufd_close(fd);

	upipe_assert_api(ret != -EIO);
	upipe_assert_api(ret != -ENOSPC);
	upipe_assert_api(ret != -EDQUOT);

	return ret;
}

#endif /* _UTILS_PIPE_H */
