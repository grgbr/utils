/**
 * @file      signal.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      04 Oct 2021
 * @copyright GNU Public License v3
 *
 * Process signal implementation
 *
 * @defgroup signal Signal
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
#include <utils/signal.h>

int
usig_read_fd(int fd, struct signalfd_siginfo * infos, unsigned int count)
{
	usig_assert(fd >= 0);
	usig_assert(infos);
	usig_assert(count);

	ssize_t ret;

	ret = read(fd, infos, count * sizeof(*infos));
	if (ret < 0) {
		usig_assert(errno != EBADF);
		usig_assert(errno != EFAULT);
		usig_assert(errno != EINVAL);
		usig_assert(errno != EIO);
		usig_assert(errno != EISDIR);

		return -errno;
	}

	return ret ? (int)(ret / sizeof(*infos)) : -EAGAIN;
}
