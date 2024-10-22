/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/path.h"
#include "utils/string.h"

int
upath_parse_mode(const char * __restrict string, mode_t * mode)
{
	upath_assert_api(string);
	upath_assert_api(mode);

	int           ret;
	unsigned long val;

	ret = ustr_parse_base_ulong(string, &val, 8);
	if (ret)
		return ret;

	if (val & ~ALLPERMS)
		return -ERANGE;

	*mode = (mode_t)val;

	return 0;
}

ssize_t
upath_validate_path(const char * __restrict path, size_t max_size)
{
	upath_assert_api(path);
	upath_assert_api(max_size);

	size_t len;

	len = strnlen(path, max_size);
	if (!len)
		return -ENODATA;

	if (len < max_size)
		return len;

	return -ENAMETOOLONG;
}

/******************************************************************************
 * Path component handling
 ******************************************************************************/

int
upath_next_comp(struct upath_comp * __restrict comp,
                const char * __restrict        path,
                size_t                         size)
{
	upath_assert_api(comp);
	upath_assert_api(path);
	upath_assert_api(size);

	size_t len;

	len = ustr_skip_char(path, '/', size);
	comp->start = path + len;

	if (size - len)
		comp->len = ustr_skip_notchar(comp->start, '/', size - len);
	else
		comp->len = 0;

	upath_assert_intern((comp->start + comp->len) <= (path + size));
	if (!comp->len)
		return -ENOENT;
	else if (comp->len >= NAME_MAX)
		return -ENAMETOOLONG;
	else
		return 0;
}

int
upath_prev_comp(struct upath_comp * __restrict comp,
                const char * __restrict        path,
                size_t                         size)
{
	upath_assert_api(comp);
	upath_assert_api(path);
	upath_assert_api(size);

	size_t len;

	len = ustr_rskip_char(path, '/', size);

	if (size - len)
		comp->len = ustr_rskip_notchar(path, '/', size - len);
	else
		comp->len = 0;

	comp->start = path + size - (len + comp->len);

	upath_assert_intern((comp->start + comp->len) <= (path + size));
	if (!comp->len)
		return -ENOENT;
	else if (comp->len >= NAME_MAX)
		return -ENAMETOOLONG;
	else
		return 0;
}

/******************************************************************************
 * Path components iterator
 ******************************************************************************/

const struct upath_comp *
upath_comp_iter_next(struct upath_comp_iter * __restrict iter)
{
	upath_assert_api(iter);
	upath_assert_api(iter->stop);
	upath_assert_api((iter->curr.start + iter->curr.len) <= iter->stop);

	int         err;
	const char *next = iter->curr.start + iter->curr.len;
	size_t      sz = iter->stop - next;

	if (!sz) {
		errno = ENOENT;
		return NULL;
	}

	err = upath_next_comp(&iter->curr, next, iter->stop - next);
	if (err) {
		errno = -err;
		return NULL;
	}

	return &iter->curr;
}

const struct upath_comp *
upath_comp_iter_first(struct upath_comp_iter * __restrict iter,
                      const char * __restrict             path,
                      size_t                              size)
{
	upath_assert_api(iter);
	upath_assert_api(path);

	iter->curr.start = path;
	iter->curr.len = 0;
	iter->stop = path + size;

	return upath_comp_iter_next(iter);
}

const struct upath_comp *
upath_comp_iter_prev(struct upath_comp_iter * __restrict iter)
{
	upath_assert_api(iter);
	upath_assert_api(iter->stop);
	upath_assert_api(iter->curr.start >= iter->stop);

	int    err;
	size_t sz = iter->curr.start - iter->stop;

	if (!sz) {
		errno = ENOENT;
		return NULL;
	}

	err = upath_prev_comp(&iter->curr, iter->stop, sz);
	if (err) {
		errno = -err;
		return NULL;
	}

	return &iter->curr;
}

const struct upath_comp *
upath_comp_iter_last(struct upath_comp_iter * __restrict iter,
                     const char * __restrict             path,
                     size_t                              size)
{
	upath_assert_api(iter);
	upath_assert_api(path);

	iter->curr.start = path + size;
	iter->curr.len = 0;
	iter->stop = path;

	return upath_comp_iter_prev(iter);
}

/******************************************************************************
 * Path normalization
 ******************************************************************************/

ssize_t
upath_normalize(const char * __restrict path,
                size_t                  path_size,
                char * __restrict       norm,
                size_t                  norm_size)
{
	upath_assert_api(path);
	upath_assert_api(path_size);
	upath_assert_api(path_size <= PATH_MAX);
	upath_assert_api(norm);
	upath_assert_api(norm_size);
	upath_assert_api(norm_size <= PATH_MAX);

	const char * path_ptr = path;
	const char * path_end = path + path_size;
	char       * norm_ptr = norm;
	const char * norm_end = norm + norm_size;

	if (*path_ptr == '/')
		*norm_ptr++ = '/';

	do {
		struct upath_comp comp;
		int               err;

		/* Probe next path component. */
		err = upath_next_comp(&comp, path_ptr, path_end - path_ptr);
		if (err) {
			if (err == -ENOENT)
				break;
			return err;
		}

		upath_assert_intern(comp.start < path_end);
		path_ptr = comp.start + comp.len;

		if (upath_comp_is_current(&comp))
			/* Skip '.' path component. */
			continue;

		if (upath_comp_is_parent(&comp) && (norm_ptr > norm)) {
			/* Get back up along the computed path. */
			struct upath_comp prev;

			err = upath_prev_comp(&prev, norm, norm_ptr - norm);
			upath_assert_intern(!err || (err == -ENOENT));
			if (!err) {
				/* An upper path component exists... */
				if (!upath_comp_is_parent(&prev)) {
					/*
					 * ...and it is not a parent directory.
					 * Make current normalized path point to
					 * it and go processing next path
					 * component.
					 */
					norm_ptr = (char *)prev.start;
					continue;
				}
			}
			else if (*norm == '/')
				/*
				 * We are already at the root of an absolute
				 * path: we cannot go any further up.  Just
				 * ignore the '..' entry to stay compliant with
				 * realpath(3) and readlink(3) behavior and go
				 * processing next path component.
				 */
				continue;

			/*
			 * We are computing a relative path and we are already
			 * at the uppermost directory, i.e. all previous
			 * components are made of a succession of '../' pattern
			 * (or no upper component exists).
			 * Hence, we need to append an additional '../' entry to
			 * the normalized path.
			 */
			upath_assert_intern(*norm != '/');
		}

		if ((norm_ptr + comp.len) >= norm_end)
			/* No more room to store current path component. */
			return -ENAMETOOLONG;

		/* Copy current path component to normalized path. */
		memcpy(norm_ptr, comp.start, comp.len);
		/* Append path delimiter to normalized path... */
		*(norm_ptr + comp.len) = '/';

		/*
		 * ... and point to next unprocessed component, skipping path
		 * delimiter.
		 */
		norm_ptr += comp.len + 1;
	} while (path_ptr < path_end);

	upath_assert_intern(norm_ptr <= norm_end);
	if (((norm_ptr - norm) > 1) && (*(norm_ptr - 1) == '/'))
		norm_ptr--;

	upath_assert_intern(norm_ptr < norm_end);
	*norm_ptr = '\0';

	return norm_ptr - norm;
}
