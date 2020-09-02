#ifndef _UTILS_PATH_H
#define _UTILS_PATH_H

#include <utils/cdefs.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __upath_nonull(_arg_index, ...)

#define __upath_pure

#define upath_assert(_expr) \
	uassert("upath", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __upath_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define __upath_pure \
	__pure

#define upath_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline ssize_t __upath_nonull(1) __upath_pure __nothrow
upath_validate_path(const char *path, size_t max_size)
{
	upath_assert(path);
	upath_assert(max_size);

	size_t len;

	len = strnlen(path, max_size);
	if (len && (len < max_size))
		return len;

	return -ENAMETOOLONG;
}

static inline ssize_t __upath_nonull(1) __upath_pure __nothrow
upath_validate_path_name(const char *path)
{
	return upath_validate_path(path, PATH_MAX);
}

static inline bool __upath_nonull(1) __upath_pure __nothrow
upath_is_path_name(const char *path, size_t len)
{
	upath_assert(upath_validate_path_name(path) == (ssize_t)len);

	return !!memchr(path, '/', len);
}

static inline ssize_t __upath_nonull(1) __upath_pure __nothrow
upath_validate_file_name(const char *path)
{
	return upath_validate_path(path, NAME_MAX);
}

static inline bool __upath_nonull(1) __upath_pure __nothrow
upath_is_file_name(const char *path, size_t len)
{
	upath_assert(upath_validate_path_name(path) == (ssize_t)len);

	return !memchr(path, '/', len);
}

extern ssize_t
upath_normalize(const char *path,
                size_t      path_size,
                char       *norm,
                size_t      norm_size);

static inline char *
upath_alloc_resolve(const char *path)
{
	upath_assert(path);

	return realpath(path, NULL);
}

#endif /* _UTILS_PATH_H */
