#include <utils/path.h>
#include <utils/string.h>

struct upath_comp {
	const char *start;
	size_t      len;
};

static int
upath_next_comp(struct upath_comp *comp, const char *path, size_t size)
{
	upath_assert(comp);
	upath_assert(path);
	upath_assert(size);

	size_t len;

	len = ustr_skip_char(path, '/', size);
	comp->start = path + len;

	if (size - len)
		comp->len = ustr_skip_notchar(comp->start, '/', size - len);
	else
		comp->len = 0;

	upath_assert((comp->start + comp->len) <= (path + size));
	if (!comp->len)
		return -ENOENT;
	else if (comp->len >= NAME_MAX)
		return -ENAMETOOLONG;
	else
		return 0;
}

static int
upath_prev_comp(struct upath_comp *comp, const char *path, const char *end)
{
	upath_assert(comp);
	upath_assert(path);
	upath_assert(end);
	upath_assert(end >= path);

	if ((end - path) > 1) {
		upath_assert(*(end - 1) == '/');

		const char *prev = end;

		do {
			prev--;
		} while ((prev > path) &&
		         (*(prev - 1) != '/') &&
		         (*(prev - 1) != '.'));

		if (prev < (end - 1)) {
			comp->start = prev;
			comp->len = (end - 1) - prev;
			upath_assert(comp->len);
			return 0;
		}

		upath_assert(*path != '/');
		return -ENOENT;
	}

	return  (*path == '/') ? -EPERM : -ENOENT;
}

ssize_t
upath_normalize(const char *path,
                size_t      path_size,
                char       *norm,
                size_t      norm_size)
{
	upath_assert(path);
	upath_assert(path_size);
	upath_assert(path_size <= PATH_MAX);
	upath_assert(norm);
	upath_assert(norm_size);
	upath_assert(norm_size <= PATH_MAX);

	const char *path_ptr = path;
	const char *path_end = path + path_size;
	char       *norm_ptr = norm;
	const char *norm_end = norm + norm_size - 1;

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

		upath_assert(comp.start < path_end);
		path_ptr = comp.start + comp.len;

		if ((comp.len == 1) && (*comp.start == '.'))
			/* Skip '.' path component. */
			continue;

		if ((comp.len == 2) &&
		    (*comp.start == '.') && (*(comp.start + 1) == '.')) {
			/* Get back up along the computed path. */
			struct upath_comp prev;

			err = upath_prev_comp(&prev, norm, norm_ptr);
			if (!err) {
				/*
				 * An upper path component exists: make current
				 * normalized path point to it and go processing
				 * next path component.
				 */
				upath_assert(prev.len == (size_t)
				                         ((norm_ptr - 1) -
				                          prev.start));
				norm_ptr = (char *)prev.start;
				continue;
			}
			else if (err == -EPERM) {
				/*
				 * We are already at the root of an absolute
				 * path: we cannot go any further up.  Just
				 * ignore the '..' entry to stay compliant with
				 * realpath(3) and readlink(3) behavior and go
				 * processing next path component.
				 */
				continue;
			}

			/*
			 * We are computing a relative path and we are already
			 * at the uppermost directory, i.e. all previous
			 * components are made of a succession of '../' pattern.
			 * Hence, we need to append an additional '../' entry to
			 * the normalized path.
			 */
			upath_assert(err == -ENOENT);
		}

		if ((norm_ptr + comp.len) >= norm_end)
			/* No more room to store curr path component. */
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

	upath_assert(norm_ptr < norm_end);
	if (((norm_ptr - norm) > 1) && (*(norm_ptr - 1) == '/'))
		norm_ptr--;
	*norm_ptr = '\0';

	return norm_ptr - norm;
}
