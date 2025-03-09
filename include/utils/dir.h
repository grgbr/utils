/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * System directory interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      30 Jul 2020
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_DIR_H
#define _UTILS_DIR_H

#include <utils/fd.h>
#include <fcntl.h>
#include <unistd.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define udir_assert_api(_expr) \
	stroll_assert("utils:udir", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define udir_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static inline
int
udir_sync(int fd)
{
	udir_assert_api(fd >= 0);

	if (fsync(fd)) {
		udir_assert_api(errno != EBADF);
		udir_assert_api(errno != EINVAL);
		udir_assert_api(errno != EROFS);

		return -errno;
	}

	return 0;
}

/*
 * May only be opened in read-only mode. Otherwise returns with an errno set to
 * -EISDIR.
 */
static inline __utils_nonull(1)
int
udir_open(const char * __restrict path, int flags)
{
	udir_assert_api(!(flags & (O_WRONLY | O_RDWR)));
	udir_assert_api(!(flags & O_TRUNC));
	udir_assert_api(!(flags & O_TMPFILE));

	return ufd_open(path, flags | O_RDONLY | O_NOCTTY | O_DIRECTORY);
}

static inline __utils_nonull(2)
int
udir_open_at(int dir, const char * __restrict path, int flags)
{
	udir_assert_api(!(flags & (O_WRONLY | O_RDWR)));
	udir_assert_api(!(flags & O_TRUNC));
	udir_assert_api(!(flags & O_TMPFILE));

	return ufd_open_at(dir,
	                   path,
	                   flags | O_RDONLY | O_NOCTTY | O_DIRECTORY);
}

extern int
udir_nointr_open(const char * __restrict path, int flags) __utils_nonull(1);

static inline
int
udir_close(int fd)
{
	return ufd_close(fd);
}

#endif /* _UTILS_DIR_H */
