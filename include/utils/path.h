#ifndef _UTILS_PATH_H
#define _UTILS_PATH_H

#include <utils/cdefs.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

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

extern int
upath_parse_mode(const char * __restrict string, mode_t * __restrict mode)
	__upath_nonull(1, 2) __nothrow __leaf;

/******************************************************************************
 * Path name checkers
 ******************************************************************************/

extern ssize_t
upath_validate_path(const char *path, size_t max_size)
	__upath_nonull(1) __upath_pure __nothrow __leaf;

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

/******************************************************************************
 * Path components iterator
 ******************************************************************************/

struct upath_comp {
	const char *start;
	size_t      len;
};

static inline bool
upath_comp_is_current(const struct upath_comp *comp)
{
	upath_assert(comp);

	return (comp->len == 1) && (*comp->start == '.');
}

static inline bool
upath_comp_is_parent(const struct upath_comp *comp)
{
	upath_assert(comp);

	return (comp->len == 2) &&
	       (*comp->start == '.') &&
	       (*(comp->start + 1) == '.');
}

extern int
upath_next_comp(struct upath_comp *comp, const char *path, size_t size);

extern int
upath_prev_comp(struct upath_comp *comp, const char *path, size_t size);

struct upath_comp_iter {
	struct upath_comp  curr;
	const char        *stop;
};

extern const struct upath_comp *
upath_comp_iter_next(struct upath_comp_iter *iter);

extern const struct upath_comp *
upath_comp_iter_first(struct upath_comp_iter *iter,
                      const char             *path,
                      size_t                  size);

#define upath_foreach_comp_forward(_iter, _comp, _path, _size) \
	for (_comp = upath_comp_iter_first(_iter, _path, _size); \
	     _comp; \
	     _comp = upath_comp_iter_next(_iter))

extern const struct upath_comp *
upath_comp_iter_prev(struct upath_comp_iter *iter);

extern const struct upath_comp *
upath_comp_iter_last(struct upath_comp_iter *iter,
                     const char             *path,
                     size_t                  size);

#define upath_foreach_comp_backward(_iter, _comp, _path, _size) \
	for (_comp = upath_comp_iter_last(_iter, _path, _size); \
	     _comp; \
	     _comp = upath_comp_iter_prev(_iter))

/******************************************************************************
 * Path normalization
 ******************************************************************************/

extern ssize_t
upath_normalize(const char *path,
                size_t      path_size,
                char       *norm,
                size_t      norm_size);

static inline char * __upath_nonull(1) __warn_result
upath_resolve(const char *path)
{
	upath_assert(path);

	char * res;

	res = realpath(path, NULL);
	if (!res) {
		upath_assert(errno != EINVAL);
		upath_assert(errno != ENAMETOOLONG);

		return NULL;
	}

	return res;
}

/******************************************************************************
 * Path related syscall helpers
 ******************************************************************************/

static inline int __upath_nonull(1, 2) __nothrow
upath_stat(const char * __restrict path, struct stat * __restrict st)
{
	upath_assert(upath_validate_path_name(path) > 0);
	upath_assert(st);

	if (!stat(path, st))
		return 0;

	upath_assert(errno != EFAULT);
	upath_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int __upath_nonull(1, 2) __nothrow
upath_lstat(const char * __restrict path, struct stat * __restrict st)
{
	upath_assert(upath_validate_path_name(path) > 0);
	upath_assert(st);

	if (!lstat(path, st))
		return 0;

	upath_assert(errno != EFAULT);
	upath_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int __upath_nonull(1) __nothrow __warn_result
upath_chdir(const char * path)
{
	upath_assert(upath_validate_path_name(path) > 0);

	if (!chdir(path))
		return 0;

	upath_assert(errno != EFAULT);
	upath_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int __upath_nonull(1) __nothrow __warn_result
upath_chown(const char * path, uid_t owner, gid_t group)
{
	upath_assert(upath_validate_path_name(path) > 0);

	if (!chown(path, owner, group))
		return 0;

	upath_assert(errno != EFAULT);
	upath_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int __upath_nonull(1) __nothrow __warn_result
upath_chmod(const char * path, mode_t mode)
{
	upath_assert(upath_validate_path_name(path) > 0);
	upath_assert(!(mode & ~ALLPERMS));

	if (!chmod(path, mode))
		return 0;

	upath_assert(errno != EFAULT);
	upath_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int __upath_nonull(1) __nothrow __warn_result
upath_truncate(const char * path, off_t length)
{
	upath_assert(upath_validate_path_name(path) > 0);
	upath_assert(length >= 0);

	if (!truncate(path, length))
		return 0;

	upath_assert(errno != EFAULT);
	upath_assert(errno != EINVAL);
	upath_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int __upath_nonull(1) __nothrow
upath_unlink(const char * path)
{
	upath_assert(upath_validate_path_name(path) > 0);

	if (!unlink(path))
		return 0;

	upath_assert(errno != EFAULT);
	upath_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int __upath_nonull(1) __nothrow
upath_mkdir(const char * path, mode_t mode)
{
	upath_assert(upath_validate_path_name(path) > 0);
	upath_assert(!(mode & ~ALLPERMS));

	if (!mkdir(path, mode))
		return 0;

	upath_assert(errno != EFAULT);
	upath_assert(errno != EINVAL);
	upath_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int __upath_nonull(1) __nothrow
upath_rmdir(const char * path)
{
	upath_assert(upath_validate_path_name(path) > 0);

	if (!rmdir(path))
		return 0;

	upath_assert(errno != EFAULT);
	upath_assert(errno != EINVAL);
	upath_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline int __upath_nonull(1, 2) __nothrow __warn_result
upath_symlink(const char * target, const char * path)
{
	upath_assert(upath_validate_path_name(target) > 0);
	upath_assert(upath_validate_path_name(path) > 0);
	upath_assert(strncmp(path, target, PATH_MAX));

	if (!symlink(target, path))
		return 0;

	upath_assert(errno != EFAULT);
	upath_assert(errno != ENAMETOOLONG);

	return -errno;
}

#endif /* _UTILS_PATH_H */
