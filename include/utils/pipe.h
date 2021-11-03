/**
 * @file      pipe.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      27 Oct 2021
 * @copyright GNU Public License v3
 *
 * Pipe interface
 *
 * @defgroup pipe Pipe
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
#ifndef _UTILS_PIPE_H
#define _UTILS_PIPE_H

#include <utils/fd.h>
#include <fcntl.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __upipe_nonull(_arg_index, ...)

#define upipe_assert(_expr) \
	uassert("upipe", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __upipe_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define upipe_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define UPIPE_READ_END  (0U)
#define UPIPE_WRITE_END (1U)
#define UPIPE_END_NR    (2U)

static ssize_t
upipe_read(int fd, char *data, size_t size)
{
	ssize_t ret;

	ret = read(fd, data, size);
	if (ret >= 0)
		return ret;

	upipe_assert(errno != EBADF);
	upipe_assert(errno != EFAULT);
	upipe_assert(errno != EINVAL);
	upipe_assert(errno != EISDIR);
	upipe_assert(errno != EIO);

	return -errno;
}

static inline int
upipe_open_anon(int fds[UPIPE_END_NR], int flags)
{
	upipe_assert(fds);
	upipe_assert(!(flags & ~(O_CLOEXEC | O_DIRECT | O_NONBLOCK)));

	if (!pipe2(fds, flags))
		return 0;

	upipe_assert(errno != EFAULT);
	upipe_assert(errno != EINVAL);

	return -errno;
}

static inline int
upipe_close(int fd)
{
	upipe_assert(fd >= 0);

	int ret;
	
	ret = ufd_close(fd);

	upipe_assert(ret != -EIO);
	upipe_assert(ret != -ENOSPC);
	upipe_assert(ret != -EDQUOT);

	return ret;
}

#endif /* _UTILS_PIPE_H */
