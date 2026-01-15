/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * System file interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      30 Jul 2020
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_FILE_H
#define _UTILS_FILE_H

#include <utils/fd.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define ufile_assert_api(_expr) \
	stroll_assert("utils:ufile", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define ufile_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#define ufile_assert_intern(_expr) \
	stroll_assert("utils:ufile", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define ufile_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

static inline __utils_nothrow __warn_result
int
ufile_fchown(int fd, uid_t owner, gid_t group)
{
	return ufd_fchown(fd, owner, group);
}

static inline __utils_nothrow
int
ufile_fchmod(int fd, mode_t mode)
{
	return ufd_fchmod(fd, mode);
}

static inline __utils_nonull(2) __utils_nothrow
int
ufile_fstat(int fd, struct stat * __restrict st)
{
	return ufd_fstat(fd, st);
}

static inline __utils_nothrow
off_t
ufile_lseek(int fd, off_t off, int whence)
{
	return ufd_lseek(fd, off, whence);
}

static inline __utils_nonull(2) __warn_result
ssize_t
ufile_read(int fd, char * __restrict data, size_t size)
{
	return ufd_read(fd, data, size);
}

static inline __utils_nonull(2) __warn_result
ssize_t
ufile_nointr_read(int fd, char * __restrict data, size_t size)
{
	return ufd_nointr_read(fd, data, size);
}

extern int
ufile_nointr_full_read(int fd, char * __restrict data, size_t size)
	__utils_nonull(2) __warn_result;


static inline __utils_nonull(2) __warn_result
ssize_t
ufile_readv(int fd, const struct iovec * __restrict vectors, unsigned int count)
{
	return ufd_readv(fd, vectors, count);
}

static inline __utils_nonull(2) __warn_result
ssize_t
ufile_write(int fd, const char * __restrict data, size_t size)
{
	ssize_t ret;

	ret = ufd_write(fd, data, size);
	ufile_assert_api(ret);
	ufile_assert_api(ret != -EDESTADDRREQ);
	ufile_assert_api(ret != -EPIPE);

	return ret;
}

static inline __utils_nonull(2) __warn_result
ssize_t
ufile_nointr_write(int fd, const char * __restrict data, size_t size)
{
	ssize_t ret;

	ret = ufd_nointr_write(fd, data, size);
	ufile_assert_api(ret);
	ufile_assert_api(ret != -EDESTADDRREQ);
	ufile_assert_api(ret != -EPIPE);

	return ret;
}

extern int
ufile_nointr_full_write(int fd, const char * __restrict data, size_t size)
	__utils_nonull(2) __warn_result;

static inline __utils_nonull(2) __warn_result
ssize_t
ufile_writev(int                             fd,
             const struct iovec * __restrict vectors,
             unsigned int                    count)
{
	ssize_t ret;

	ret = ufd_writev(fd, vectors, count);
	ufile_assert_api(ret);
	ufile_assert_api(ret != -EDESTADDRREQ);
	ufile_assert_api(ret != -EPIPE);

	return ret;
}

static inline __utils_nothrow __warn_result
int
ufile_ftruncate(int fd, off_t len)
{
	ufile_assert_api(fd >= 0);
	ufile_assert_api(len >= 0);

	if (!ftruncate(fd, len))
		return 0;

	ufile_assert_api(errno != EBADF);
	ufile_assert_api(errno != EINVAL);
	ufile_assert_api(errno != EACCES);
	ufile_assert_api(errno != EFAULT);
	ufile_assert_api(errno != EISDIR);
	ufile_assert_api(errno != ENAMETOOLONG);
	ufile_assert_api(errno != ENOENT);
	ufile_assert_api(errno != ENOTDIR);

	return -errno;
}

/*
 * See copy_file_range(2).
 *
 * Return:
 * * -EFBIG
 * * -ENOMEM
 * * -ENOSPC
 * * -EPERM
 * * -EXDEV
 * Warning: does not support dst_fd opened with O_APPEND flag (see man page).
 */
static inline
int
ufile_copy_fds(int src_fd, int dst_fd, size_t size)
{
	ufile_assert_api(src_fd >= 0);
	ufile_assert_api(dst_fd >= 0);
	ufile_assert_api(size);

	off64_t src_off = 0;
	off64_t dst_off = 0;

	if (!copy_file_range(src_fd, &src_off, dst_fd, &dst_off, size, 0))
		return 0;

	ufile_assert_api(errno != EBADF);
	ufile_assert_api(errno != EINVAL);
	ufile_assert_api(errno != EISDIR);
	ufile_assert_api(errno != ETXTBSY);

	return -errno;
}

static inline
int
ufile_sync(int fd)
{
	ufile_assert_api(fd >= 0);

	if (!fsync(fd))
		return 0;

	ufile_assert_api(errno != EBADF);
	ufile_assert_api(errno != EINVAL);
	ufile_assert_api(errno != EROFS);

	return -errno;
}

static inline __utils_nonull(1)
int
ufile_open(const char * __restrict path, int flags)
{
	ufile_assert_api(!(flags & O_DIRECTORY));

	int fd;

	fd = ufd_open(path, flags | O_NOCTTY);
	if (fd >= 0)
		return fd;

	ufile_assert_api(fd != -EOPNOTSUPP);

	return fd;
}

static inline __utils_nonull(1)
int
ufile_nointr_open(const char * __restrict path, int flags)
{
	return ufd_nointr_open(path, flags | O_NOCTTY);
}

static inline __utils_nonull(2)
int
ufile_open_at(int dir, const char * __restrict path, int flags)
{
	ufile_assert_api(!(flags & O_DIRECTORY));

	int fd;

	fd = ufd_open_at(dir, path, flags | O_NOCTTY);
	if (fd >= 0)
		return fd;

	ufile_assert_api(errno != EOPNOTSUPP);

	return -errno;
}

static inline __utils_nonull(2)
int
ufile_nointr_open_at(int dir, const char * __restrict path, int flags)
{
	ufile_assert_api(!(flags & O_DIRECTORY));

	int fd;

	fd = ufd_nointr_open_at(dir, path, flags | O_NOCTTY);
	if (fd >= 0)
		return fd;

	ufile_assert_api(errno != EOPNOTSUPP);

	return -errno;
}

static inline __utils_nonull(1)
int
ufile_new(const char * __restrict path, int flags, mode_t mode)
{
	ufile_assert_api(upath_validate_path_name(path) > 0);
	ufile_assert_api((flags & O_TMPFILE) != O_TMPFILE);
	ufile_assert_api(!(flags & O_DIRECTORY));

	int fd;

	fd = open(path, flags | O_CREAT | O_NOCTTY, mode);
	if (fd >= 0)
		return fd;

	ufile_assert_api(errno != EOPNOTSUPP);
	ufile_assert_intern(errno != EFAULT);
	ufile_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

extern int
ufile_nointr_new(const char * __restrict path, int flags, mode_t mode)
	__utils_nonull(1);

static inline __utils_nonull(2)
int
ufile_new_at(int dir, const char * __restrict path, int flags, mode_t mode)
{
	ufile_assert_api(dir >= 0);
	ufile_assert_api(upath_validate_path_name(path) > 0);
	ufile_assert_api((flags & O_TMPFILE) != O_TMPFILE);
	ufile_assert_api(!(flags & O_DIRECTORY));

	int fd;

	fd = openat(dir, path, flags | O_CREAT | O_NOCTTY, mode);
	if (fd >= 0)
		return fd;

	ufile_assert_api(errno != EBADF);
	ufile_assert_api(errno != EOPNOTSUPP);
	ufile_assert_intern(errno != EFAULT);
	ufile_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

extern int
ufile_nointr_new_at(int                     dir,
                    const char * __restrict path,
                    int                     flags,
                    mode_t                  mode) __utils_nonull(2);

static inline
int
ufile_close(int fd)
{
	return ufd_close(fd);
}

static inline __utils_nonull(1) __utils_nothrow
int
ufile_unlink(const char * __restrict path)
{
	return upath_unlink(path);
}

static inline __utils_nonull(2) __utils_nothrow
int
ufile_unlink_at(int dir, const char * __restrict path)
{
	return upath_unlink_at(dir, path);
}

static inline __utils_nonull(1, 2) __utils_nothrow
int
ufile_rename(const char * __restrict old_path, const char * __restrict new_path)
{
	return upath_rename(old_path, new_path);
}

static inline __utils_nonull(2, 4) __utils_nothrow
int
ufile_rename_at(int                     old_dir,
                const char * __restrict old_path,
                int                     new_dir,
                const char * __restrict new_path,
                unsigned int            flags)
{
	return upath_rename_at(old_dir, old_path, new_dir, new_path, flags);
}

#endif /* _UTILS_FILE_H */
