/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * File system path interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      04 Oct 2021
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_PATH_H
#define _UTILS_PATH_H

#include <utils/cdefs.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

/*
 *  Maximum value for an off_t variable.
 *
 * As stated into the glibc manual section "Feature Test Macros", the
 * _FILE_OFFSET_BITS macro controls the bit size of off_t, and therefore the bit
 * size of all off_t-derived types and the prototypes of all related functions.
 *
 * See
 * https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
 * for more infos.
 */
#define OFF_MAX \
	compile_choose(__builtin_types_compatible_p(off_t, long long), \
	               LLONG_MAX, \
	               LONG_MAX)

/*
 *  Minimum value for an off_t variable.
 *
 * As stated into the glibc manual section "Feature Test Macros", the
 * _FILE_OFFSET_BITS macro controls the bit size of off_t, and therefore the bit
 * size of all off_t-derived types and the prototypes of all related functions.
 *
 * See
 * https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
 * for more infos.
 */
#define OFF_MIN \
	compile_choose(__builtin_types_compatible_p(off_t, long long), \
	               LLONG_MIN, \
	               LONG_MIN)

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define upath_assert_api(_expr) \
	stroll_assert("utils:upath", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define upath_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#define upath_assert_intern(_expr) \
	stroll_assert("utils:upath", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define upath_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

extern int
upath_parse_mode(const char * __restrict string, mode_t * __restrict mode)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

/******************************************************************************
 * Path name checkers
 ******************************************************************************/

extern ssize_t
upath_validate_path(const char * __restrict path, size_t max_size)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
ssize_t
upath_validate_path_name(const char * __restrict path)
{
	return upath_validate_path(path, PATH_MAX);
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
upath_is_path_name(const char * __restrict path, size_t len)
{
	upath_assert_api(upath_validate_path_name(path) == (ssize_t)len);

	return !!memchr(path, '/', len);
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
ssize_t
upath_validate_file_name(const char * __restrict path)
{
	return upath_validate_path(path, NAME_MAX);
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
upath_is_file_name(const char * __restrict path, size_t len)
{
	upath_assert_api(upath_validate_path_name(path) == (ssize_t)len);

	return !memchr(path, '/', len);
}

/******************************************************************************
 * Path components iterator
 ******************************************************************************/

struct upath_comp {
	const char * start;
	size_t       len;
};

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
upath_comp_is_current(const struct upath_comp * __restrict comp)
{
	upath_assert_api(comp);

	return (comp->len == 1) && (*comp->start == '.');
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
upath_comp_is_parent(const struct upath_comp * __restrict comp)
{
	upath_assert_api(comp);

	return (comp->len == 2) &&
	       (*comp->start == '.') &&
	       (*(comp->start + 1) == '.');
}

extern int
upath_next_comp(struct upath_comp * __restrict comp,
                const char * __restrict        path,
                size_t                         size)
	__utils_nonull(1, 2) __utils_nothrow __leaf;

extern int
upath_prev_comp(struct upath_comp * __restrict comp,
                const char * __restrict        path,
                size_t                         size)
	__utils_nonull(1, 2) __utils_nothrow __leaf;

struct upath_comp_iter {
	struct upath_comp curr;
	const char *      stop;
};

extern const struct upath_comp *
upath_comp_iter_next(struct upath_comp_iter * __restrict iter)
	__utils_nonull(1) __utils_nothrow;

extern const struct upath_comp *
upath_comp_iter_first(struct upath_comp_iter * __restrict iter,
                      const char * __restrict             path,
                      size_t                              size)
	__utils_nonull(1, 2) __utils_nothrow;

#define upath_foreach_comp_forward(_iter, _comp, _path, _size) \
	for (_comp = upath_comp_iter_first(_iter, _path, _size); \
	     _comp; \
	     _comp = upath_comp_iter_next(_iter))

extern const struct upath_comp *
upath_comp_iter_prev(struct upath_comp_iter * __restrict iter)
	__utils_nonull(1) __utils_nothrow;

extern const struct upath_comp *
upath_comp_iter_last(struct upath_comp_iter * __restrict iter,
                     const char * __restrict             path,
                     size_t                              size)
	__utils_nonull(1, 2) __utils_nothrow;

#define upath_foreach_comp_backward(_iter, _comp, _path, _size) \
	for (_comp = upath_comp_iter_last(_iter, _path, _size); \
	     _comp; \
	     _comp = upath_comp_iter_prev(_iter))

/******************************************************************************
 * Path normalization
 ******************************************************************************/

extern ssize_t
upath_normalize(const char * __restrict path,
                size_t                  path_size,
                char * __restrict       norm,
                size_t                  norm_size)
	__utils_nonull(1, 3) __utils_nothrow;

static inline __utils_nonull(1) __warn_result
char *
upath_resolve(const char * __restrict path)
{
	upath_assert_api(path);

	char * res;

	res = realpath(path, NULL);
	if (!res) {
		upath_assert_api(errno != EINVAL);
		upath_assert_api(errno != ENAMETOOLONG);

		return NULL;
	}

	return res;
}

/******************************************************************************
 * Path related syscall helpers
 ******************************************************************************/

static inline __utils_nonull(1, 2) __utils_nothrow
int
upath_stat(const char * __restrict path, struct stat * __restrict st)
{
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(st);

	if (!stat(path, st))
		return 0;

	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1, 2) __utils_nothrow
int
upath_lstat(const char * __restrict path, struct stat * __restrict st)
{
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(st);

	if (!lstat(path, st))
		return 0;

	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow __warn_result
int
upath_chdir(const char * __restrict path)
{
	upath_assert_api(upath_validate_path_name(path) > 0);

	if (!chdir(path))
		return 0;

	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow __warn_result
int
upath_chown(const char * __restrict path, uid_t owner, gid_t group)
{
	upath_assert_api(upath_validate_path_name(path) > 0);

	if (!chown(path, owner, group))
		return 0;

	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow __warn_result
int
upath_chmod(const char * __restrict path, mode_t mode)
{
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(!(mode & ~((mode_t)ALLPERMS)));

	if (!chmod(path, mode))
		return 0;

	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow __warn_result
int
upath_truncate(const char * __restrict path, off_t length)
{
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(length >= 0);

	if (!truncate(path, length))
		return 0;

	upath_assert_api(errno != EINVAL);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
upath_symlink(const char * __restrict target, const char * __restrict path)
{
	upath_assert_api(upath_validate_path_name(target) > 0);
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(strncmp(path, target, PATH_MAX));

	if (!symlink(target, path))
		return 0;

	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1, 3) __utils_nothrow __warn_result
int
upath_symlink_at(const char * __restrict target,
                 int                     path_dir,
                 const char * __restrict path)
{
	upath_assert_api(upath_validate_path_name(target) > 0);
	upath_assert_api((path_dir >= 0) || (path_dir == AT_FDCWD));
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(strncmp(path, target, PATH_MAX));

	if (!symlinkat(target, path_dir, path))
		return 0;

	upath_assert_api(errno != EBADF);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1, 2) __utils_nothrow
int
upath_rename(const char * __restrict old_path, const char * __restrict new_path)
{
	upath_assert_api(upath_validate_path_name(old_path) > 0);
	upath_assert_api(upath_validate_path_name(new_path) > 0);

	if (!rename(old_path, new_path))
		return 0;

	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(2, 4) __utils_nothrow
int
upath_rename_at(int                     old_dir,
                const char * __restrict old_path,
                int                     new_dir,
                const char * __restrict new_path,
                unsigned int            flags)
{
	upath_assert_api((old_dir >= 0) || (old_dir == AT_FDCWD));
	upath_assert_api(upath_validate_path_name(old_path) > 0);
	upath_assert_api((new_dir >= 0) || (new_dir == AT_FDCWD));
	upath_assert_api(upath_validate_path_name(new_path) > 0);
	upath_assert_api(!(flags & ~((unsigned int)(RENAME_EXCHANGE |
	                                            RENAME_NOREPLACE |
	                                            RENAME_WHITEOUT))));

	if (!renameat2(old_dir, old_path, new_dir, new_path, flags))
		return 0;

	upath_assert_api(errno != EBADF);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow
int
upath_unlink(const char * __restrict path)
{
	upath_assert_api(upath_validate_path_name(path) > 0);

	if (!unlink(path))
		return 0;

	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(2) __utils_nothrow
int
upath_unlink_at(int dir, const char * __restrict path)
{
	upath_assert_api(dir >= 0);
	upath_assert_api(upath_validate_path_name(path) > 0);

	if (!unlinkat(dir, path, 0))
		return 0;

	upath_assert_api(errno != EBADF);
	upath_assert_api(errno != EINVAL);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow
int
upath_mkdir(const char * __restrict path, mode_t mode)
{
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(!(mode & ~((mode_t)ALLPERMS)));

	if (!mkdir(path, mode))
		return 0;

	upath_assert_api(errno != EINVAL);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(2) __utils_nothrow
int
upath_mkdir_at(int dir, const char * __restrict path, mode_t mode)
{
	upath_assert_api((dir >= 0) || (dir == AT_FDCWD));
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(!(mode & ~((mode_t)ALLPERMS)));

	if (!mkdirat(dir, path, mode))
		return 0;

	upath_assert_api(errno != EBADF);
	upath_assert_api(errno != EINVAL);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow
int
upath_rmdir(const char * __restrict path)
{
	upath_assert_api(upath_validate_path_name(path) > 0);

	if (!rmdir(path))
		return 0;

	upath_assert_api(errno != EINVAL);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(2) __utils_nothrow
int
upath_rmdir_at(int dir, const char * __restrict path)
{
	upath_assert_api(dir >= 0);
	upath_assert_api(upath_validate_path_name(path) > 0);

	if (!unlinkat(dir, path, AT_REMOVEDIR))
		return 0;

	upath_assert_api(errno != EBADF);
	upath_assert_api(errno != EINVAL);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1) __nothrow
int
upath_mknod(const char * __restrict path, mode_t mode, dev_t dev)
{
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(!(mode & ~((mode_t)S_IFMT | ACCESSPERMS)));
	upath_assert_api(S_ISREG(mode) ||
	                 S_ISCHR(mode) ||
	                 S_ISBLK(mode) ||
	                 S_ISFIFO(mode) ||
	                 S_ISSOCK(mode));
	upath_assert_api((major(dev) > 0) || !(mode & (S_IFCHR | S_IFBLK)));

	if (!mknod(path, mode, dev))
		return 0;

	upath_assert_api(errno != EINVAL);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(2) __nothrow
int
upath_mknod_at(int dir, const char * __restrict path, mode_t mode, dev_t dev)
{
	upath_assert_api((dir >= 0) || (dir == AT_FDCWD));
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(!(mode & ~((mode_t)S_IFMT | ACCESSPERMS)));
	upath_assert_api(S_ISREG(mode) ||
	                 S_ISCHR(mode) ||
	                 S_ISBLK(mode) ||
	                 S_ISFIFO(mode) ||
	                 S_ISSOCK(mode));
	upath_assert_api((major(dev) > 0) || !(mode & (S_IFCHR | S_IFBLK)));

	if (!mknodat(dir, path, mode, dev))
		return 0;

	upath_assert_api(errno != EBADF);
	upath_assert_api(errno != EINVAL);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow
int
upath_mkfifo(const char * __restrict path, mode_t mode)
{
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(!(mode & ~((mode_t)ACCESSPERMS)));

	if (!mkfifo(path, mode))
		return 0;

	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

static inline __utils_nonull(2) __utils_nothrow
int
upath_mkfifo_at(int dir, const char * __restrict path, mode_t mode)
{
	upath_assert_api((dir >= 0) || (dir == AT_FDCWD));
	upath_assert_api(upath_validate_path_name(path) > 0);
	upath_assert_api(!(mode & ~((mode_t)ACCESSPERMS)));

	if (!mkfifoat(dir, path, mode))
		return 0;

	upath_assert_api(errno != EBADF);
	upath_assert_intern(errno != EFAULT);
	upath_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

#endif /* _UTILS_PATH_H */
