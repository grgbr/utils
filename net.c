/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/net.h"
#include "utils/path.h"

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define unet_assert_intern(_expr) \
	stroll_assert("utils:unet", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define unet_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

static __utils_nonull(1) __utils_pure __utils_nothrow
ssize_t
unet_iface_syspath_prefix_len(const char * __restrict path, size_t size)
{
	unet_assert_intern(path);

	size_t len;

	len = ustr_prefix_len(path,
	                      size,
	                      UNET_IFACE_SYSPATH_PREFIX,
	                      sizeof(UNET_IFACE_SYSPATH_PREFIX) - 1);
	unet_assert_intern(len <= size);

	if (len) {
		if (path[len] == '/')
			len++;

		if (len == size)
			return -ENOENT;

		return len;
	}

	if (path[0] == '/')
		return -ENOENT;

	return 0;
}

static __utils_nonull(1, 2) __utils_nothrow
int
unet_match_iface_syspath_suffix(struct upath_comp * __restrict comp,
                                const char * __restrict        path,
                                size_t                         size)
{
	unet_assert_intern(comp);
	unet_assert_intern(path);

	int ret;

	if (!size)
		return -ENOENT;

	ret = upath_prev_comp(comp, path, size);
	unet_assert_intern(!ret || (ret == -ENOENT));
	if (ret)
		return ret;

	if (comp->len != (sizeof("net") - 1))
		return 0;

	if (memcmp(comp->start, "net", sizeof("net") - 1))
		return 0;

	return 1;
}

static __utils_nonull(1) __utils_nothrow
ssize_t
unet_iface_syspath_suffix_len(const char * __restrict path, size_t size)
{
	unet_assert_intern(path);

	int               ret;
	struct upath_comp comp;
	size_t            len;

	ret = unet_match_iface_syspath_suffix(&comp, path, size);
	if (ret < 0)
		return ret;

	if (ret) {
		len = 1U + path + size - comp.start;
		if ((size <= len) || (path[size - len] != '/'))
			return -ENOENT;

		return len;
	}

	ret = unet_match_iface_syspath_suffix(&comp,
	                                      path,
	                                      comp.start - path);
	if (ret <= 0)
		return 0;

	len = 1U + path + size - comp.start;
	if ((size <= len) || (path[size - len] != '/'))
		return -ENOENT;

	return len;
}


static __utils_nonull(1) __utils_nothrow
ssize_t
unet_strip_iface_syspath(char * __restrict path, size_t size)
{
	unet_assert_intern(path);

	ssize_t   len;
	char    * start;
	ssize_t   suff_len;

	if (!size)
		return -ENOENT;

	/* Strip optional leading sysfs prefix. */
	len = unet_iface_syspath_prefix_len(path, size);
	if (len < 0)
		return len;

	/* Strip optional trailing sysfs net/... suffix. */
	start = path + len;
	len = size - len;
	suff_len = unet_iface_syspath_suffix_len(start, len);
	if (suff_len < 0)
		return suff_len;

	/* Check final length. */
	len -= suff_len;
	if ((size_t)len >= UNET_IFACE_SYSPATH_MAX)
		return -ENAMETOOLONG;

	if (start != path)
		memmove(path, start, len);

	path[len] = '\0';

	return len;
}

ssize_t
unet_normalize_iface_syspath(const char * __restrict orig,
                             char ** __restrict      norm)
{
	unet_assert_api(orig);
	unet_assert_api(norm);

	ssize_t ret;

	*norm = malloc(PATH_MAX);
	if (!*norm)
		return -errno;

	/* Check and normalize given syspath. */
	ret = upath_normalize(orig, PATH_MAX, *norm, PATH_MAX);
	if (ret < 0)
		goto free;

	/* Strip optional leading sysfs prefix and trailing suffix. */
	ret = unet_strip_iface_syspath(*norm, ret);
	if (ret < 0)
		goto free;

	return ret;

free:
	free(*norm);

	return ret;
}

ssize_t
unet_resolve_iface_syspath(const char * __restrict orig,
                           char ** __restrict      real)
{
	unet_assert_api(orig);
	unet_assert_api(real);

	ssize_t ret;

	/* Make given path an absolute path with all symlinks resolved. */
	*real = upath_resolve(orig);
	if (!*real)
		return -errno;

	/* Strip optional leading sysfs prefix and trailing suffix. */
	ret = unet_strip_iface_syspath(*real, strlen(*real));
	if (ret < 0)
		goto free;

	return ret;

free:
	free(*real);

	return ret;
}
