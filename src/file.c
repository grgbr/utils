/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/file.h"

int
ufile_nointr_full_read(int fd, char * __restrict data, size_t size)
{
	ufile_assert_api(fd >= 0);
	ufile_assert_api(!(!!data ^ !!size));

	ssize_t off = 0;

	while (size) {
		ssize_t ret;

		ret = ufile_nointr_read(fd, &data[off], size);
		if (ret > 0) {
			off += ret;
			size -= (size_t)ret;
		}
		else if (ret != -EAGAIN)
			return !ret ? -ENODATA : (int)ret;
	}

	return 0;
}

int
ufile_nointr_full_write(int fd, const char * __restrict data, size_t size)
{
	ufile_assert_api(fd >= 0);
	ufile_assert_api(!(!!data ^ !!size));

	ssize_t off = 0;

	while (size) {
		ssize_t ret;

		ret = ufile_nointr_write(fd, &data[off], size);
		if (ret > 0) {
			off += ret;
			size -= (size_t)ret;
		}
		else if (ret != -EAGAIN)
			return (int)ret;
	}

	return 0;
}

int
ufile_nointr_new(const char * __restrict path, int flags, mode_t mode)
{
	int fd;

	do {
		fd = ufile_new(path, flags, mode);
	} while (fd == -EINTR);

	return fd;
}

int
ufile_nointr_new_at(int                     dir,
                    const char * __restrict path,
                    int                     flags,
                    mode_t                  mode)
{
	int fd;

	do {
		fd = ufile_new_at(dir, path, flags, mode);
	} while (fd == -EINTR);

	return fd;
}
