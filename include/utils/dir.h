#ifndef _UTILS_DIR_H
#define _UTILS_DIR_H

#include <utils/path.h>
#include <fcntl.h>
#include <unistd.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __udir_nonull(_arg_index, ...)

#define udir_assert(_expr) \
	uassert("udir", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __udir_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define udir_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline int __udir_nonull(1)
udir_open(const char *path, int flags)
{
	udir_assert(upath_validate_path_name(path) > 0);

	int fd;

	fd = open(path, flags | O_NOCTTY | O_DIRECTORY);
	if (fd >= 0)
		return fd;

	udir_assert(errno != EFAULT);
	udir_assert(errno != ENAMETOOLONG);
	udir_assert(errno != EOPNOTSUPP);

	return -errno;
}

extern int
udir_nointr_open(const char *path, int flags) __udir_nonull(1);

static inline int
udir_close(int fd)
{
	udir_assert(fd >= 0);

	if (!close(fd))
		return 0;

	udir_assert(errno != EBADF);

	return -errno;
}

extern int
udir_nointr_close(int fd);

#endif /* _UTILS_DIR_H */
