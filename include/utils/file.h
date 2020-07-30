#ifndef _UTILS_FILE_H
#define _UTILS_FILE_H

#include <utils/path.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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

static inline int __nothrow
ufile_fchmod(int fd, mode_t mode)
{
	ufile_assert(fd >= 0);
	ufile_assert(!(mode & ~ALLPERMS));

	if (!fchmod(fd, mode))
		return 0;

	ufile_assert(errno != EBADF);
	ufile_assert(errno != EFAULT);

	return -errno;
}

static inline int __ufile_nonull(2) __nothrow
ufile_fstat(int fd, struct stat *st)
{
	ufile_assert(fd >= 0);
	ufile_assert(st);

	if (!fstat(fd, st))
		return 0;

	ufile_assert(errno != EBADF);
	ufile_assert(errno != EFAULT);

	return -errno;
}

static inline ssize_t __ufile_nonull(2) __warn_result
ufile_read(int fd, char *data, size_t size)
{
	ufile_assert(fd >= 0);
	ufile_assert(data);
	ufile_assert(size);

	ssize_t ret;

	ret = read(fd, data, size);
	if (ret >= 0)
		return ret;

	ufile_assert(errno != EBADF);
	ufile_assert(errno != EFAULT);
	ufile_assert(errno != EISDIR);

	return -errno;
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
ufile_write(int fd, const char *data, size_t size)
{
	ufile_assert(fd >= 0);
	ufile_assert(data);
	ufile_assert(size);

	ssize_t ret;

	ret = write(fd, data, size);
	ufile_assert(ret);

	if (ret > 0)
		return ret;

	ufile_assert(errno != EBADF);
	ufile_assert(errno != EDESTADDRREQ);
	ufile_assert(errno != EFAULT);
	ufile_assert(errno != EPIPE);

	return -errno;
}

extern ssize_t
ufile_nointr_write(int         fd,
                   const char *data,
                   size_t      size) __ufile_nonull(2) __warn_result;

extern int
ufile_nointr_full_write(int         fd,
                        const char *data,
                        size_t      size) __ufile_nonull(2) __warn_result;

static inline int __ufile_nonull(1)
ufile_open(const char *path, int flags)
{
	ufile_assert(upath_validate_path_name(path) > 0);
	ufile_assert(!(flags & O_TMPFILE));
	ufile_assert(!(flags & O_DIRECTORY));
	ufile_assert(!(flags & O_CREAT));
	ufile_assert(!(flags & O_EXCL));

	int fd;

	fd = open(path, flags | O_NOCTTY);
	if (fd >= 0)
		return fd;

	ufile_assert(errno != EFAULT);
	ufile_assert(errno != ENAMETOOLONG);
	ufile_assert(errno != EOPNOTSUPP);

	return -errno;
}

extern int
ufile_nointr_open(const char *path, int flags) __ufile_nonull(1);

static inline int __ufile_nonull(2)
ufile_open_at(int dir, const char *path, int flags)
{
	ufile_assert(dir >= 0);
	ufile_assert(upath_validate_path_name(path) > 0);
	ufile_assert(!(flags & O_TMPFILE));
	ufile_assert(!(flags & O_DIRECTORY));
	ufile_assert(!(flags & O_CREAT));
	ufile_assert(!(flags & O_EXCL));

	int fd;

	fd = openat(dir, path, flags | O_NOCTTY);
	if (fd >= 0)
		return fd;

	ufile_assert(errno != EFAULT);
	ufile_assert(errno != ENAMETOOLONG);
	ufile_assert(errno != EOPNOTSUPP);
	ufile_assert(errno != EBADF);

	return -errno;
}

extern int
ufile_nointr_open_at(int dir, const char *path, int flags) __ufile_nonull(2);

static inline int __ufile_nonull(1)
ufile_new(const char *path, int flags, mode_t mode)
{
	ufile_assert(upath_validate_path_name(path) > 0);
	ufile_assert(!(flags & O_TMPFILE));
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
	ufile_assert(!(flags & O_TMPFILE));
	ufile_assert(!(flags & O_DIRECTORY));

	int fd;

	fd = openat(dir, path, flags | O_CREAT | O_NOCTTY, mode);
	if (fd >= 0)
		return fd;

	ufile_assert(errno != EFAULT);
	ufile_assert(errno != ENAMETOOLONG);
	ufile_assert(errno != EOPNOTSUPP);
	ufile_assert(errno != EBADF);

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
	ufile_assert(fd >= 0);

	if (!close(fd))
		return 0;

	ufile_assert(errno != EBADF);

	return -errno;
}

extern int
ufile_nointr_close(int fd);

static inline int __ufile_nonull(1) __nothrow
ufile_unlink(const char *path)
{
	ufile_assert(upath_validate_path_name(path) > 0);

	if (!unlink(path))
		return 0;

	ufile_assert(errno != EFAULT);
	ufile_assert(errno != ENAMETOOLONG);

	return -errno;
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
