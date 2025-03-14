/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * File descriptor interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      27 Oct 2021
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_FD_H
#define _UTILS_FD_H

#include <utils/cdefs.h>
#include <utils/path.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define ufd_assert_api(_expr) \
	stroll_assert("utils:ufd", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define ufd_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#define ufd_assert_intern(_expr) \
	stroll_assert("utils:ufd", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define ufd_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

static inline __utils_nothrow
unsigned int
ufd_max_nr(void)
{
	struct rlimit lim;
	int           err __unused;

	err = getrlimit(RLIMIT_NOFILE, &lim);
	ufd_assert_intern(!err);

	return (unsigned int)lim.rlim_cur;
}

static inline __utils_nothrow __warn_result
int
ufd_fchown(int fd, uid_t owner, gid_t group)
{
	ufd_assert_api(fd >= 0);

	if (!fchown(fd, owner, group))
		return 0;

	ufd_assert_api(errno != EBADF);

	return -errno;
}

static inline __utils_nothrow
int
ufd_fchmod(int fd, mode_t mode)
{
	ufd_assert_api(fd >= 0);
	ufd_assert_api(!(mode & ~((mode_t)ALLPERMS)));

	if (!fchmod(fd, mode))
		return 0;

	ufd_assert_api(errno != EBADF);

	return -errno;
}

static inline __utils_nonull(2) __utils_nothrow
int 
ufd_fstat(int fd, struct stat * __restrict st)
{
	ufd_assert_api(fd >= 0);
	ufd_assert_api(st);

	if (!fstat(fd, st))
		return 0;

	ufd_assert_api(errno != EBADF);
	ufd_assert_api(errno != EFAULT);
	ufd_assert_intern(errno != EOVERFLOW);

	return -errno;
}

static inline __utils_nonull(2, 3) __utils_nothrow
int
ufd_fstat_at(int                      fd,
             const char * __restrict  path,
             struct stat * __restrict st,
             int                      flags)
{
	ufd_assert_api(upath_validate_path_name(path) > 0);
	ufd_assert_api((path[0] == '/') || (fd >= 0) || (fd == AT_FDCWD));
	ufd_assert_api(st);
	ufd_assert_api(!(flags & ~(AT_NO_AUTOMOUNT | AT_SYMLINK_NOFOLLOW)));

	if (!fstatat(fd, path, st, flags))
		return 0;

	ufd_assert_api(errno != EBADF);
	ufd_assert_intern(errno != EFAULT);
	ufd_assert_intern(errno != ENAMETOOLONG);
	ufd_assert_intern(errno != EOVERFLOW);
	ufd_assert_api(errno != EINVAL);

	return -errno;
}

static inline __utils_nothrow
off_t
ufd_lseek(int fd, off_t off, int whence)
{
	ufd_assert_api(fd >= 0);
	ufd_assert_api((whence == SEEK_SET) ||
	               (whence == SEEK_CUR) ||
	               (whence == SEEK_END) ||
	               (whence == SEEK_DATA) ||
	               (whence == SEEK_HOLE));

	off_t ret;

	ret = lseek(fd, off, whence);
	if (ret >= 0)
		return ret;

	ufd_assert_api(errno != EBADF);
	ufd_assert_intern(errno != EOVERFLOW);
	ufd_assert_api(errno != ESPIPE);

	return ret;
}

static inline __utils_nonull(2) __warn_result
ssize_t
ufd_read(int fd, char * __restrict data, size_t size)
{
	ufd_assert_api(fd >= 0);
	ufd_assert_api(data);
	ufd_assert_api(size);

	ssize_t ret;

	ret = read(fd, data, size);
	if (ret >= 0)
		return ret;

	ufd_assert_api(errno != EBADF);
	ufd_assert_api(errno != EFAULT);
	ufd_assert_api(errno != EINVAL);
	ufd_assert_api(errno != EISDIR);

	return -errno;
}

extern ssize_t
ufd_nointr_read(int fd, char * __restrict data, size_t size)
	__utils_nonull(2) __warn_result;

static inline __utils_nonull(2) __warn_result
ssize_t
ufd_readv(int fd, const struct iovec * __restrict vectors, unsigned int count)
{
	ufd_assert_api(fd >= 0);
	ufd_assert_api(vectors);
	ufd_assert_api(count);
	ufd_assert_api(count < IOV_MAX);

	ssize_t ret;

	ret = readv(fd, vectors, (int)count);

	if (ret >= 0)
		return ret;

	ufd_assert_api(errno != EBADF);
	ufd_assert_api(errno != EFAULT);
	ufd_assert_api(errno != EINVAL);
	ufd_assert_api(errno != EISDIR);

	return -errno;
}

static inline __utils_nonull(2) __warn_result
ssize_t
ufd_write(int fd, const char * __restrict data, size_t size)
{
	ufd_assert_api(fd >= 0);
	ufd_assert_api(data);
	ufd_assert_api(size);

	ssize_t ret;

	ret = write(fd, data, size);

	if (ret >= 0)
		return ret;

	ufd_assert_api(errno != EBADF);
	ufd_assert_api(errno != EFAULT);
	ufd_assert_api(errno != EINVAL);

	return -errno;
}

extern ssize_t
ufd_nointr_write(int fd, const char * __restrict data, size_t size)
	__utils_nonull(2) __warn_result;

static inline __utils_nonull(2) __warn_result
ssize_t
ufd_writev(int fd, const struct iovec * __restrict vectors, unsigned int count)
{
	ufd_assert_api(fd >= 0);
	ufd_assert_api(vectors);
	ufd_assert_api(count);
	ufd_assert_api(count < IOV_MAX);

	ssize_t ret;

	ret = writev(fd, vectors, (int)count);

	if (ret >= 0)
		return ret;

	ufd_assert_api(errno != EBADF);
	ufd_assert_api(errno != EFAULT);
	ufd_assert_api(errno != EINVAL);

	return -errno;
}

static inline __utils_nothrow
int
ufd_dup2(int old_fd, int new_fd)
{
	ufd_assert_api(old_fd >= 0);
	ufd_assert_api(new_fd >= 0);

	if (dup2(old_fd, new_fd) >= 0)
		return 0;

	ufd_assert_api(errno != EBADF);

	/*
	 * This is related to a possible race condition between new_fd closing
	 * and duplicating operations when called from incorrect multi-threaded
	 * and / or async-signal-unsafe implementations.
	 * See section NOTES of dup2(2) man page for more infos.
	 */
	ufd_assert_api(errno != EBUSY);

	return -errno;
}

static inline __utils_nonull(1)
int
ufd_open(const char * __restrict path, int flags)
{
	ufd_assert_api(upath_validate_path_name(path) > 0);
	ufd_assert_api(!((flags & O_DIRECTORY) &&
	                 (flags & (O_WRONLY | O_RDWR))));
	/* O_TMPFILE requires the (creation) mode argument. */
	ufd_assert_api((flags & O_TMPFILE) != O_TMPFILE);
	ufd_assert_api(!(flags & O_CREAT));
	ufd_assert_api(!(flags & O_EXCL));

	int fd;

	fd = open(path, flags);
	if (fd >= 0)
		return fd;

	ufd_assert_intern(errno != EFAULT);
	ufd_assert_intern(errno != ENAMETOOLONG);
	ufd_assert_intern(errno != EOVERFLOW);

	return -errno;
}

extern int
ufd_nointr_open(const char * __restrict path, int flags) __utils_nonull(1);

static inline __utils_nonull(2)
int
ufd_open_at(int dir, const char * __restrict path, int flags)
{
	ufd_assert_api(upath_validate_path_name(path) > 0);
	ufd_assert_api((path[0] == '/') || (dir >= 0) || (dir == AT_FDCWD));
	ufd_assert_api(!((flags & O_DIRECTORY) &&
	                 (flags & (O_WRONLY | O_RDWR))));
	/* O_TMPFILE requires the (creation) mode argument. */
	ufd_assert_api((flags & O_TMPFILE) != O_TMPFILE);
	ufd_assert_api(!(flags & O_CREAT));
	ufd_assert_api(!(flags & O_EXCL));

	int fd;

	fd = openat(dir, path, flags);
	if (fd >= 0)
		return fd;

	ufd_assert_api(errno != EBADF);
	ufd_assert_intern(errno != EFAULT);
	ufd_assert_intern(errno != ENAMETOOLONG);
	ufd_assert_intern(errno != EOVERFLOW);

	return -errno;
}

extern int
ufd_nointr_open_at(int dir, const char * __restrict path, int flags)
	__utils_nonull(2);

static inline
int
ufd_close(int fd)
{
	ufd_assert_api(fd >= 0);

	if (!close(fd))
		return 0;

	ufd_assert_api(errno != EBADF);

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

static inline __nothrow
int
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

static inline __utils_nothrow
int
ufd_close_range(unsigned int first, unsigned int last, unsigned int flags)
{
	ufd_assert_api(first <= last);
	ufd_assert_api(!(flags & ~UFD_CLOSE_RANGE_FLAG_MASK));

	if (!close_range(first, last, (int)flags))
		return 0;

	ufd_assert_api(errno != EINVAL);

	return -errno;
}

static inline __utils_nothrow
int
ufd_close_fds(unsigned int first, unsigned int last)
{
	return ufd_close_range(first, last, 0);
}

#else /* !(defined(__NR_close_range) && defined(__USE_GNU)) */

extern int
ufd_close_fds(unsigned int first, unsigned int last) __leaf;

#endif /* defined(__NR_close_range) && defined(__USE_GNU) */

#endif /* _UTILS_FD_H */
