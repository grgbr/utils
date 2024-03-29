/**
 * @file      fd.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      27 Oct 2021
 * @copyright GNU Public License v3
 *
 * File descriptor interface
 *
 * @defgroup fd File descriptors
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
#ifndef _UTILS_FD_H
#define _UTILS_FD_H

#include <utils/path.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __ufd_nonull(_arg_index, ...)

#define ufd_assert(_expr) \
	uassert("ufd", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __ufd_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define ufd_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline unsigned int __nothrow
ufd_max_nr(void)
{
	struct rlimit lim;
	int           err __unused;

	err = getrlimit(RLIMIT_NOFILE, &lim);
	ufd_assert(!err);

	return (unsigned int)lim.rlim_cur;
}

static inline int __nothrow __warn_result
ufd_fchown(int fd, uid_t owner, gid_t group)
{
	ufd_assert(fd >= 0);

	if (!fchown(fd, owner, group))
		return 0;

	ufd_assert(errno != EBADF);
	ufd_assert(errno != EFAULT);

	return -errno;
}

static inline int __nothrow
ufd_fchmod(int fd, mode_t mode)
{
	ufd_assert(fd >= 0);
	ufd_assert(!(mode & ~((mode_t)ALLPERMS)));

	if (!fchmod(fd, mode))
		return 0;

	ufd_assert(errno != EBADF);
	ufd_assert(errno != EFAULT);

	return -errno;
}

static inline int __ufd_nonull(2) __nothrow
ufd_fstat(int fd, struct stat * __restrict st)
{
	ufd_assert(fd >= 0);
	ufd_assert(st);

	if (!fstat(fd, st))
		return 0;

	ufd_assert(errno != EBADF);
	ufd_assert(errno != EFAULT);

	return -errno;
}

static inline off_t __nothrow
ufd_lseek(int fd, off_t off, int whence)
{
	ufd_assert(fd >= 0);
	ufd_assert((whence == SEEK_SET) ||
	           (whence == SEEK_CUR) ||
	           (whence == SEEK_END) ||
	           (whence == SEEK_DATA) ||
	           (whence == SEEK_HOLE));

	off_t ret;

	ret = lseek(fd, off, whence);
	if (ret >= 0)
		return ret;

	ufd_assert(errno != EBADF);
	ufd_assert(errno != EOVERFLOW);
	ufd_assert(errno != ESPIPE);

	return ret;
}

static inline ssize_t __ufd_nonull(2) __warn_result
ufd_read(int fd, char * data, size_t size)
{
	ufd_assert(fd >= 0);
	ufd_assert(data);
	ufd_assert(size);

	ssize_t ret;

	ret = read(fd, data, size);
	if (ret >= 0)
		return ret;

	ufd_assert(errno != EBADF);
	ufd_assert(errno != EFAULT);
	ufd_assert(errno != EISDIR);

	return -errno;
}

static inline ssize_t __ufd_nonull(2) __warn_result
ufd_write(int fd, const char *data, size_t size)
{
	ufd_assert(fd >= 0);
	ufd_assert(data);
	ufd_assert(size);

	ssize_t ret;

	ret = write(fd, data, size);

	if (ret >= 0)
		return ret;

	ufd_assert(errno != EBADF);
	ufd_assert(errno != EFAULT);
	ufd_assert(errno != EINVAL);

	return -errno;
}

extern ssize_t __ufd_nonull(2) __warn_result
ufd_nointr_write(int fd, const char *data, size_t size);

static inline ssize_t __ufd_nonull(2) __warn_result
ufd_writev(int fd, const struct iovec * vectors, unsigned int count)
{
	ufd_assert(fd >= 0);
	ufd_assert(vectors);
	ufd_assert(count);
	ufd_assert(count < IOV_MAX);

	ssize_t ret;

	ret = writev(fd, vectors, (int)count);

	if (ret >= 0)
		return ret;

	ufd_assert(errno != EBADF);
	ufd_assert(errno != EFAULT);
	ufd_assert(errno != EINVAL);

	return -errno;
}

static inline int __nothrow
ufd_dup2(int old_fd, int new_fd)
{
	ufd_assert(old_fd >= 0);
	ufd_assert(new_fd >= 0);

	if (dup2(old_fd, new_fd) >= 0)
		return 0;

	ufd_assert(errno != EBADF);
	ufd_assert(errno != EINVAL);

	/*
	 * This is related to a possible race condition between new_fd closing
	 * and duplicating operations when called from incorrect multi-threaded
	 * and / or async-signal-unsafe implementations.
	 * See section NOTES of dup2(2) man page for more infos.
	 */
	ufd_assert(errno != EBUSY);

	return -errno;
}

static inline int __ufd_nonull(1)
ufd_open(const char *path, int flags)
{
	ufd_assert(upath_validate_path_name(path) > 0);
	ufd_assert(!((flags & O_DIRECTORY) && (flags & (O_WRONLY | O_RDWR))));
	/* O_TMPFILE requires the (creation) mode argument. */
	ufd_assert((flags & O_TMPFILE) != O_TMPFILE);
	ufd_assert(!(flags & O_CREAT));
	ufd_assert(!(flags & O_EXCL));

	int fd;

	fd = open(path, flags);
	if (fd >= 0)
		return fd;

	ufd_assert(errno != EFAULT);
	ufd_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int
ufd_close(int fd)
{
	ufd_assert(fd >= 0);

	if (!close(fd))
		return 0;

	ufd_assert(errno != EBADF);

	/*
	 * Note that, as specified by POSIX.1-2013: 
	 *    If  close()  is  interrupted  by	a signal that is to be caught,
	 *    it shall return -1 with errno set to EINTR and the state of fildes
	 *    is unspecified.
	 * On Linux, despite the returned error code, the file descriptor is
	 * guaranteed to be closed. Hence, close() must not be called again as
	 * with any error codes other than EINTR.
	 */
	return -errno;
}

#if defined(__NR_close_range) && defined(__USE_GNU)
/*
 * OK, kernel defines the syscall and we are compiling with GNU extensions
 * enabled: define the userspace syscall wrapper if required.
 */

#include <linux/close_range.h>
#include <linux/version.h>

#if !defined(SYS_close_range)
/*
 * Glibc does not provides a definition for close_range(): let's do this.
 * Note: close_range() definition comes with Glibc 2.34 and later versions.
 */

static inline int __nothrow
close_range(unsigned int first, unsigned int last, int flags)
{
	return syscall(__NR_close_range, first, last, flags);
}

#endif /* !defined(SYS_close_range) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0)
/*
 * CLOSE_RANGE_CLOEXEC flag support comes with Linux kernel 5.11 release and
 * later.
 */

#define UFD_CLOSE_RANGE_FLAG_MASK (CLOSE_RANGE_CLOEXEC | CLOSE_RANGE_UNSHARE)

#else /* !(LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0)) */

#define UFD_CLOSE_RANGE_FLAG_MASK (CLOSE_RANGE_UNSHARE)

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0) */

static inline int __nothrow
ufd_close_range(unsigned int first, unsigned int last, unsigned int flags)
{
	ufd_assert(first <= last);
	ufd_assert(!(flags & ~UFD_CLOSE_RANGE_FLAG_MASK));

	if (!close_range(first, last, (int)flags))
		return 0;

	ufd_assert(errno != EINVAL);

	return -errno;
}

static inline int __nothrow
ufd_close_fds(unsigned int first, unsigned int last)
{
	return ufd_close_range(first, last, 0);
}

#else /* !(defined(__NR_close_range) && defined(__USE_GNU)) */

extern int
ufd_close_fds(unsigned int first, unsigned int last);

#endif /* defined(__NR_close_range) && defined(__USE_GNU) */

#endif /* _UTILS_FD_H */
