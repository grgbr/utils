#ifndef _UTILS_DIR_H
#define _UTILS_DIR_H

#include <utils/path.h>
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

static inline int
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
static inline int __utils_nonull(1)
udir_open(const char *path, int flags)
{
	udir_assert_api(upath_validate_path_name(path) > 0);
	udir_assert_api(!(flags & (O_WRONLY | O_RDWR)));

	int fd;

	fd = ufd_open(path, flags | O_RDONLY | O_NOCTTY | O_DIRECTORY);
	if (fd >= 0)
		return fd;

	udir_assert_api(fd != -EOPNOTSUPP);

	return fd;
}

extern int
udir_nointr_open(const char *path, int flags) __utils_nonull(1);

static inline int
udir_close(int fd)
{
	return ufd_close(fd);
}

#endif /* _UTILS_DIR_H */
