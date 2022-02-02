#ifndef _UTILS_FILE_H
#define _UTILS_FILE_H

#include <utils/fd.h>
#include <stdio.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __ufile_nonull(_arg_index, ...)

#define ufile_assert(_expr) \
	uassert("ufile", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __ufile_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define ufile_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline int __nothrow __warn_result
ufile_fchown(int fd, uid_t owner, gid_t group)
{
	ufile_assert(fd >= 0);

	return ufd_fchown(fd, owner, group);
}

static inline int __nothrow
ufile_fchmod(int fd, mode_t mode)
{
	ufile_assert(fd >= 0);
	ufile_assert(!(mode & ~ALLPERMS));

	return ufd_fchmod(fd, mode);
}

static inline int __ufile_nonull(2) __nothrow
ufile_fstat(int fd, struct stat * __restrict st)
{
	ufile_assert(fd >= 0);
	ufile_assert(st);

	return ufd_fstat(fd, st);
}

static inline off_t __nothrow
ufile_lseek(int fd, off_t off, int whence)
{
	ufile_assert(fd >= 0);
	ufile_assert((whence == SEEK_SET) ||
	             (whence == SEEK_CUR) ||
	             (whence == SEEK_END) ||
	             (whence == SEEK_DATA) ||
	             (whence == SEEK_HOLE));

	return ufd_lseek(fd, off, whence);
}

static inline ssize_t __ufile_nonull(2) __warn_result
ufile_read(int fd, char * data, size_t size)
{
	ufile_assert(fd >= 0);
	ufile_assert(data);
	ufile_assert(size);

	return ufd_read(fd, data, size);
}

extern ssize_t
ufile_nointr_read(int     fd,
                  char   *data,
                  size_t  size) __ufile_nonull(2) __warn_result;

extern int
ufile_nointr_full_read(int     fd,
                       char   *data,
                       size_t  size) __ufile_nonull(2) __warn_result;

static inline ssize_t __ufile_nonull(2) __warn_result
ufile_write(int fd, const char * data, size_t size)
{
	ufile_assert(fd >= 0);
	ufile_assert(data);
	ufile_assert(size);

	ssize_t ret;

	ret = ufd_write(fd, data, size);
	ufile_assert(ret);
	ufile_assert(ret != -EDESTADDRREQ);
	ufile_assert(ret != -EPIPE);

	return ret;
}

extern ssize_t
ufile_nointr_write(int          fd,
                   const char * data,
                   size_t       size) __ufile_nonull(2) __warn_result;

extern int
ufile_nointr_full_write(int         fd,
                        const char *data,
                        size_t      size) __ufile_nonull(2) __warn_result;

static inline ssize_t __ufile_nonull(2) __warn_result
ufile_writev(int fd, const struct iovec * vectors, unsigned int count)
{
	ufile_assert(fd >= 0);
	ufile_assert(vectors);
	ufile_assert(count);
	ufile_assert(count < IOV_MAX);

	ssize_t ret;

	ret = ufd_writev(fd, vectors, count);
	ufile_assert(ret);
	ufile_assert(ret != -EDESTADDRREQ);
	ufile_assert(ret != -EPIPE);

	return ret;
}

static inline int
ufile_ftruncate(int fd, off_t len)
{
	ufile_assert(fd >= 0);
	ufile_assert(len >= 0);

	if (!ftruncate(fd, len))
		return 0;

	ufile_assert(errno != EBADF);
	ufile_assert(errno != EINVAL);
	ufile_assert(errno != EACCES);
	ufile_assert(errno != EFAULT);
	ufile_assert(errno != EISDIR);
	ufile_assert(errno != ENAMETOOLONG);
	ufile_assert(errno != ENOENT);
	ufile_assert(errno != ENOTDIR);

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
static inline int
ufile_copy_fds(int src_fd, int dst_fd, size_t size)
{
	ufile_assert(src_fd >= 0);
	ufile_assert(dst_fd >= 0);
	ufile_assert(size);

	off64_t src_off = 0;
	off64_t dst_off = 0;

	if (!copy_file_range(src_fd, &src_off, dst_fd, &dst_off, size, 0))
		return 0;

	ufile_assert(errno != EBADF);
	ufile_assert(errno != EINVAL);
	ufile_assert(errno != EISDIR);
	ufile_assert(errno != EOVERFLOW);
	ufile_assert(errno != ETXTBSY);

	return -errno;
}

static inline int
ufile_sync(int fd)
{
	ufile_assert(fd >= 0);

	if (!fsync(fd))
		return 0;

	ufile_assert(errno != EBADF);
	ufile_assert(errno != EINVAL);
	ufile_assert(errno != EROFS);

	return -errno;
}

static inline int __ufile_nonull(1)
ufile_open(const char *path, int flags)
{
	ufile_assert(upath_validate_path_name(path) > 0);
	ufile_assert((flags & O_TMPFILE) != O_TMPFILE);
	ufile_assert(!(flags & O_DIRECTORY));
	ufile_assert(!(flags & O_CREAT));
	ufile_assert(!(flags & O_EXCL));

	int fd;

	fd = ufd_open(path, flags | O_NOCTTY);
	if (fd >= 0)
		return fd;

	ufile_assert(fd != -EOPNOTSUPP);

	return fd;
}

extern int
ufile_nointr_open(const char *path, int flags) __ufile_nonull(1);

static inline int __ufile_nonull(2)
ufile_open_at(int dir, const char *path, int flags)
{
	ufile_assert(dir >= 0);
	ufile_assert(upath_validate_path_name(path) > 0);
	ufile_assert((flags & O_TMPFILE) != O_TMPFILE);
	ufile_assert(!(flags & O_DIRECTORY));
	ufile_assert(!(flags & O_CREAT));
	ufile_assert(!(flags & O_EXCL));

	int fd;

	fd = openat(dir, path, flags | O_NOCTTY);
	if (fd >= 0)
		return fd;

	ufile_assert(errno != EBADF);
	ufile_assert(errno != EFAULT);
	ufile_assert(errno != ENAMETOOLONG);
	ufile_assert(errno != EOPNOTSUPP);

	return -errno;
}

extern int
ufile_nointr_open_at(int dir, const char *path, int flags) __ufile_nonull(2);

static inline int __ufile_nonull(1)
ufile_new(const char *path, int flags, mode_t mode)
{
	ufile_assert(upath_validate_path_name(path) > 0);
	ufile_assert((flags & O_TMPFILE) != O_TMPFILE);
	ufile_assert(!(flags & O_DIRECTORY));

	int fd;

	fd = open(path, flags | O_CREAT | O_NOCTTY, mode);
	if (fd >= 0)
		return fd;

	ufile_assert(errno != EFAULT);
	ufile_assert(errno != ENAMETOOLONG);
	ufile_assert(errno != EOPNOTSUPP);

	return -errno;
}

extern int
ufile_nointr_new(const char *path, int flags, mode_t mode) __ufile_nonull(1);

static inline int __ufile_nonull(2)
ufile_new_at(int dir, const char *path, int flags, mode_t mode)
{
	ufile_assert(dir >= 0);
	ufile_assert(upath_validate_path_name(path) > 0);
	ufile_assert((flags & O_TMPFILE) != O_TMPFILE);
	ufile_assert(!(flags & O_DIRECTORY));

	int fd;

	fd = openat(dir, path, flags | O_CREAT | O_NOCTTY, mode);
	if (fd >= 0)
		return fd;

	ufile_assert(errno != EBADF);
	ufile_assert(errno != EFAULT);
	ufile_assert(errno != ENAMETOOLONG);
	ufile_assert(errno != EOPNOTSUPP);

	return -errno;
}

extern int
ufile_nointr_new_at(int         dir,
                    const char *path,
                    int         flags,
                    mode_t      mode) __ufile_nonull(2);

static inline int
ufile_close(int fd)
{
	return ufd_close(fd);
}

static inline int __ufile_nonull(1) __nothrow
ufile_unlink(const char * path)
{
	return upath_unlink(path);
}

static inline int __ufile_nonull(2) __nothrow
ufile_unlink_at(int dir, const char *path)
{
	ufile_assert(dir >= 0);
	ufile_assert(upath_validate_path_name(path) > 0);

	if (!unlinkat(dir, path, 0))
		return 0;

	ufile_assert(errno != EFAULT);
	ufile_assert(errno != ENAMETOOLONG);
	ufile_assert(errno != EBADF);
	ufile_assert(errno != EINVAL);

	return -errno;
}

static inline int __ufile_nonull(1, 2) __nothrow
ufile_rename(const char *old_path, const char *new_path)
{
	ufile_assert(upath_validate_path_name(old_path) > 0);
	ufile_assert(upath_validate_path_name(new_path) > 0);

	if (!rename(old_path, new_path))
		return 0;

	ufile_assert(errno != EFAULT);
	ufile_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int __ufile_nonull(2, 3) __nothrow
ufile_rename_at(int dir, const char *old_path, const char *new_path)
{
	ufile_assert(dir >= 0);
	ufile_assert(upath_validate_path_name(old_path) > 0);
	ufile_assert(upath_validate_path_name(new_path) > 0);

	if (!renameat(dir, old_path, dir, new_path))
		return 0;

	ufile_assert(errno != EFAULT);
	ufile_assert(errno != ENAMETOOLONG);
	ufile_assert(errno != EBADF);

	return -errno;
}

#endif /* _UTILS_FILE_H */
