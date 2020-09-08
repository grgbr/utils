#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <stdbool.h>

#define ustr_assert(...) assert(__VA_ARGS__)
#define upath_assert(...) assert(__VA_ARGS__)

struct upath_comp {
	const char *start;
	size_t      len;
};

static inline size_t
ustr_skip_char(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string;

	while ((str < (string + size)) && (*str == ch))
		str++;

	return str - string;
}

static inline size_t
ustr_rskip_char(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string + size - 1;

	while ((str >= string) && (*str == ch))
		str--;

	return size - (size_t)((str + 1) - string);
}

static inline size_t
ustr_skip_notchar(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string;

	while ((str < (string + size)) && *str && (*str != ch))
		str++;

	return str - string;
}

static inline size_t
ustr_rskip_notchar(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string + size - 1;

	if (!*str)
		return 0;

	while ((str >= string) && (*str != ch))
		str--;

	return size - (size_t)((str + 1) - string);
}

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
upath_prev_comp(struct upath_comp *comp, const char *path, size_t size)
{
	upath_assert(comp);
	upath_assert(path);
	upath_assert(size);

	size_t len;

	len = ustr_rskip_char(path, '/', size);

	if (size - len)
		comp->len = ustr_rskip_notchar(path, '/', size - len);
	else
		comp->len = 0;

	comp->start = path + size - (len + comp->len);

	upath_assert((comp->start + comp->len) <= (path + size));
	if (!comp->len)
		return -ENOENT;
	else if (comp->len >= NAME_MAX)
		return -ENAMETOOLONG;
	else
		return 0;
}

static bool
upath_comp_is_current(const struct upath_comp *comp)
{
	upath_assert(comp);

	return (comp->len == 1) && (*comp->start == '.');
}

static bool
upath_comp_is_parent(const struct upath_comp *comp)
{
	upath_assert(comp);

	return (comp->len == 2) &&
	       (*comp->start == '.') &&
	       (*(comp->start + 1) == '.');
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

		if (upath_comp_is_current(&comp))
			/* Skip '.' path component. */
			continue;

		if (upath_comp_is_parent(&comp) && (norm_ptr > norm)) {
			/* Get back up along the computed path. */
			struct upath_comp prev;

			err = upath_prev_comp(&prev, norm, norm_ptr - norm);
			upath_assert(!err || (err == -ENOENT));
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
			upath_assert(*norm != '/');
		}
#if 0
		if (upath_comp_is_parent(&comp)) {
			/* Get back up along the computed path. */
			struct upath_comp prev;

			err = upath_prev_comp_ptr(&prev, norm, norm_ptr);
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
#endif

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

	upath_assert(norm_ptr < norm_end);
	if (((norm_ptr - norm) > 1) && (*(norm_ptr - 1) == '/'))
		norm_ptr--;
	*norm_ptr = '\0';

	return norm_ptr - norm;
}

void
test_norm(const char *orig, size_t orig_len, size_t norm_size, const char *ref)
{
	ssize_t ret;
	char    norm[PATH_MAX];
	int     ok = 0;
	size_t  ref_len;

	memset(norm, 0xde, sizeof(norm));

	ret = upath_normalize(orig, orig_len, norm, norm_size);

	if (ret >= 0) {
		ok = !strcmp(norm, ref);
		if (ok)
			ok = (strlen(ref) == ret);
	}

	printf("%s: '%*.*s' --> '%s' (%zd/%zu)\n",
	       ok ? "OK" : "NOK",
	       (int)orig_len, (int)orig_len, orig,
	       (ret < 0) ? "ERROR" : norm, ret, norm_size);
}

#if 1
#define ORIG "///first/se.ond/./third/././fourth//"
int main(void)
{
	const char *orig = ORIG;
	const char *ref;

	ref = " ";
	test_norm(" ", 1, PATH_MAX, ref);

	ref = "/first/se.ond/third/fourth";
	test_norm(orig, sizeof(ORIG), PATH_MAX, ref);
	test_norm(orig, sizeof(ORIG) - 1, PATH_MAX, ref);
	test_norm(orig, sizeof(ORIG) - 2, PATH_MAX, ref);

	ref = "/";
	test_norm(orig, 1, PATH_MAX, ref);
	test_norm(orig, 2, PATH_MAX, ref);
	test_norm(orig, 3, PATH_MAX, ref);

	ref = "/first";
	test_norm(orig, 8, PATH_MAX, ref);
	test_norm(orig, 9, PATH_MAX, ref);

	ref = "/first/s";
	test_norm(orig, 10, PATH_MAX, ref);

	ref = "/first/se.ond";
	test_norm(orig, 17, PATH_MAX, ref);
	test_norm(orig, 18, PATH_MAX, ref);

	ref = "/first/se.ond/t";
	test_norm(orig, 19, PATH_MAX, ref);

	ref = "first/se.ond/third/fourth";
	test_norm(&orig[3], sizeof(ORIG) - 3, PATH_MAX, ref);
	test_norm(&orig[3], sizeof(ORIG) - 4, PATH_MAX, ref);
	test_norm(&orig[3], sizeof(ORIG) - 5, PATH_MAX, ref);

	ref = "f";
	test_norm(&orig[3], 1, PATH_MAX, ref);

	ref = "firs";
	test_norm(&orig[3], 4, PATH_MAX, ref);

	ref = "first";
	test_norm(&orig[3], 5, PATH_MAX, ref);
	test_norm(&orig[3], 6, PATH_MAX, ref);

	ref = "first/s";
	test_norm(&orig[3], 7, PATH_MAX, ref);

	ref = "first/se.ond";
	test_norm(&orig[3], 13, PATH_MAX, ref);
	test_norm(&orig[3], 14, PATH_MAX, ref);
	test_norm(&orig[3], 15, PATH_MAX, ref);

	ref = "first/se.ond/t";
	test_norm(&orig[3], 16, PATH_MAX, ref);

	ref = "/third/fourth";
	test_norm(&orig[15], sizeof(ORIG) - 15, PATH_MAX, ref);
	test_norm(&orig[15], sizeof(ORIG) - 16, PATH_MAX, ref);
	test_norm(&orig[15], sizeof(ORIG) - 17, PATH_MAX, ref);

	ref = "/";
	test_norm(&orig[15], 1, PATH_MAX, ref);
	test_norm(&orig[15], 2, PATH_MAX, ref);
	test_norm(&orig[15], 3, PATH_MAX, ref);

	ref = "/t";
	test_norm(&orig[15], 4, PATH_MAX, ref);

	ref = "third/fourth";
	test_norm(&orig[16], sizeof(ORIG) - 16, PATH_MAX, ref);
	test_norm(&orig[16], sizeof(ORIG) - 17, PATH_MAX, ref);
	test_norm(&orig[16], sizeof(ORIG) - 18, PATH_MAX, ref);

	ref = "";
	test_norm(&orig[16], 1, PATH_MAX, ref);
	test_norm(&orig[16], 2, PATH_MAX, ref);

	ref = "t";
	test_norm(&orig[16], 3, PATH_MAX, ref);

	ref = "th";
	test_norm(&orig[16], 4, PATH_MAX, ref);

	ref = "/fourth";
	test_norm(&orig[23], sizeof(ORIG) - 23, PATH_MAX, ref);
	test_norm(&orig[23], sizeof(ORIG) - 24, PATH_MAX, ref);
	test_norm(&orig[23], sizeof(ORIG) - 25, PATH_MAX, ref);

	ref = "fourth";
	test_norm(&orig[24], sizeof(ORIG) - 24, PATH_MAX, ref);
	test_norm(&orig[24], sizeof(ORIG) - 25, PATH_MAX, ref);
	test_norm(&orig[24], sizeof(ORIG) - 26, PATH_MAX, ref);

	orig = "/first/../..///../se.ond/third/..///";

	ref = "/";
	test_norm(&orig[0], 9, PATH_MAX, ref);
	test_norm(&orig[0], 10, PATH_MAX, ref);
	test_norm(&orig[0], 11, PATH_MAX, ref);
	test_norm(&orig[0], 12, PATH_MAX, ref);
	test_norm(&orig[0], 13, PATH_MAX, ref);
	test_norm(&orig[0], 15, PATH_MAX, ref);
	test_norm(&orig[0], 17, PATH_MAX, ref);

	ref = "/se.ond";
	test_norm(&orig[0], 24, PATH_MAX, ref);
	test_norm(&orig[0], 25, PATH_MAX, ref);
	test_norm(&orig[0], sizeof(ORIG), PATH_MAX, ref);

	ref = "";
	test_norm(&orig[1], 8, PATH_MAX, ref);
	test_norm(&orig[1], 9, PATH_MAX, ref);
	test_norm(&orig[1], 10, PATH_MAX, ref);

	ref = "..";
	test_norm(&orig[1], 11, PATH_MAX, ref);
	test_norm(&orig[1], 12, PATH_MAX, ref);
	test_norm(&orig[1], 13, PATH_MAX, ref);
	test_norm(&orig[1], 14, PATH_MAX, ref);
	test_norm(&orig[1], 15, PATH_MAX, ref);

	ref = "../..";
	test_norm(&orig[1], 16, PATH_MAX, ref);


	ref = "..";
	test_norm(&orig[7], 2, PATH_MAX, ref);
	test_norm(&orig[7], 3, PATH_MAX, ref);
	test_norm(&orig[7], 4, PATH_MAX, ref);

	ref = "../..";
	test_norm(&orig[7], 5, PATH_MAX, ref);
	test_norm(&orig[7], 6, PATH_MAX, ref);
	test_norm(&orig[7], 7, PATH_MAX, ref);
	test_norm(&orig[7], 8, PATH_MAX, ref);
	test_norm(&orig[7], 9, PATH_MAX, ref);

	ref = "../../..";
	test_norm(&orig[7], 10, PATH_MAX, ref);
	test_norm(&orig[7], 11, PATH_MAX, ref);

	ref = "../../../se.ond";
	test_norm(&orig[7], 17, PATH_MAX, ref);
	test_norm(&orig[7], 18, PATH_MAX, ref);
	test_norm(&orig[7], sizeof(ORIG) - 8, PATH_MAX, ref);

	ref = "../../../se.ond/third";
	test_norm(&orig[7], 23, PATH_MAX, ref);
	test_norm(&orig[7], 24, PATH_MAX, ref);

	ref = "/";
	test_norm(&orig[6], 3, PATH_MAX, ref);
	test_norm(&orig[6], 4, PATH_MAX, ref);
	test_norm(&orig[6], 12, PATH_MAX, ref);

	ref = "/se.ond";
	test_norm(&orig[6], 18, PATH_MAX, ref);
}
#else
int main(void)
{
	const char *orig = "///first/second/./third/././fourth//";
	const char *ref;

	ref = " ";
	test_norm(" ", 1, PATH_MAX, ref);

	ref = "/first/second/third/fourth";
	test_norm(orig, strlen(orig), PATH_MAX, ref);
	test_norm(orig, strlen(orig) - 1, PATH_MAX, ref);
	test_norm(orig, strlen(orig) - 2, PATH_MAX, ref);

	ref = "/";
	test_norm(orig, 1, PATH_MAX, ref);
	test_norm(orig, 2, PATH_MAX, ref);
	test_norm(orig, 3, PATH_MAX, ref);

	ref = "/first";
	test_norm(orig, 8, PATH_MAX, ref);
	test_norm(orig, 9, PATH_MAX, ref);

	ref = "/first/s";
	test_norm(orig, 10, PATH_MAX, ref);

	ref = "/first/second";
	test_norm(orig, 17, PATH_MAX, ref);
	test_norm(orig, 18, PATH_MAX, ref);

	ref = "/first/second/t";
	test_norm(orig, 19, PATH_MAX, ref);

	ref = "first/second/third/fourth";
	test_norm(&orig[3], strlen(orig) - 3, PATH_MAX, ref);
	test_norm(&orig[3], strlen(orig) - 4, PATH_MAX, ref);
	test_norm(&orig[3], strlen(orig) - 5, PATH_MAX, ref);

	ref = "f";
	test_norm(&orig[3], 1, PATH_MAX, ref);

	ref = "firs";
	test_norm(&orig[3], 4, PATH_MAX, ref);

	ref = "first";
	test_norm(&orig[3], 5, PATH_MAX, ref);
	test_norm(&orig[3], 6, PATH_MAX, ref);

	ref = "first/s";
	test_norm(&orig[3], 7, PATH_MAX, ref);

	ref = "first/second";
	test_norm(&orig[3], 13, PATH_MAX, ref);
	test_norm(&orig[3], 14, PATH_MAX, ref);
	test_norm(&orig[3], 15, PATH_MAX, ref);

	ref = "first/second/t";
	test_norm(&orig[3], 16, PATH_MAX, ref);

	ref = "/third/fourth";
	test_norm(&orig[15], strlen(orig) - 15, PATH_MAX, ref);
	test_norm(&orig[15], strlen(orig) - 16, PATH_MAX, ref);
	test_norm(&orig[15], strlen(orig) - 17, PATH_MAX, ref);

	ref = "/";
	test_norm(&orig[15], 1, PATH_MAX, ref);
	test_norm(&orig[15], 2, PATH_MAX, ref);
	test_norm(&orig[15], 3, PATH_MAX, ref);

	ref = "/t";
	test_norm(&orig[15], 4, PATH_MAX, ref);

	ref = "third/fourth";
	test_norm(&orig[16], strlen(orig) - 16, PATH_MAX, ref);
	test_norm(&orig[16], strlen(orig) - 17, PATH_MAX, ref);
	test_norm(&orig[16], strlen(orig) - 18, PATH_MAX, ref);

	ref = "";
	test_norm(&orig[16], 1, PATH_MAX, ref);
	test_norm(&orig[16], 2, PATH_MAX, ref);

	ref = "t";
	test_norm(&orig[16], 3, PATH_MAX, ref);

	ref = "th";
	test_norm(&orig[16], 4, PATH_MAX, ref);

	ref = "/fourth";
	test_norm(&orig[23], strlen(orig) - 23, PATH_MAX, ref);
	test_norm(&orig[23], strlen(orig) - 24, PATH_MAX, ref);
	test_norm(&orig[23], strlen(orig) - 25, PATH_MAX, ref);

	ref = "fourth";
	test_norm(&orig[24], strlen(orig) - 24, PATH_MAX, ref);
	test_norm(&orig[24], strlen(orig) - 25, PATH_MAX, ref);
	test_norm(&orig[24], strlen(orig) - 26, PATH_MAX, ref);

	orig = "/first/../..///../second/third/..///";

	ref = "/";
	test_norm(&orig[0], 9, PATH_MAX, ref);
	test_norm(&orig[0], 10, PATH_MAX, ref);
	test_norm(&orig[0], 11, PATH_MAX, ref);
	test_norm(&orig[0], 12, PATH_MAX, ref);
	test_norm(&orig[0], 13, PATH_MAX, ref);
	test_norm(&orig[0], 15, PATH_MAX, ref);
	test_norm(&orig[0], 17, PATH_MAX, ref);

	ref = "/second";
	test_norm(&orig[0], 24, PATH_MAX, ref);
	test_norm(&orig[0], 25, PATH_MAX, ref);
	test_norm(&orig[0], strlen(orig) + 1, PATH_MAX, ref);

	ref = "";
	test_norm(&orig[1], 8, PATH_MAX, ref);
	test_norm(&orig[1], 9, PATH_MAX, ref);
	test_norm(&orig[1], 10, PATH_MAX, ref);

	ref = "..";
	test_norm(&orig[1], 11, PATH_MAX, ref);
	test_norm(&orig[1], 12, PATH_MAX, ref);
	test_norm(&orig[1], 13, PATH_MAX, ref);
	test_norm(&orig[1], 14, PATH_MAX, ref);
	test_norm(&orig[1], 15, PATH_MAX, ref);

	ref = "../..";
	test_norm(&orig[1], 16, PATH_MAX, ref);


	ref = "..";
	test_norm(&orig[7], 2, PATH_MAX, ref);
	test_norm(&orig[7], 3, PATH_MAX, ref);
	test_norm(&orig[7], 4, PATH_MAX, ref);

	ref = "../..";
	test_norm(&orig[7], 5, PATH_MAX, ref);
	test_norm(&orig[7], 6, PATH_MAX, ref);
	test_norm(&orig[7], 7, PATH_MAX, ref);
	test_norm(&orig[7], 8, PATH_MAX, ref);
	test_norm(&orig[7], 9, PATH_MAX, ref);

	ref = "../../..";
	test_norm(&orig[7], 10, PATH_MAX, ref);
	test_norm(&orig[7], 11, PATH_MAX, ref);

	ref = "../../../second";
	test_norm(&orig[7], 17, PATH_MAX, ref);
	test_norm(&orig[7], 18, PATH_MAX, ref);
	test_norm(&orig[7], strlen(orig) - 8, PATH_MAX, ref);

	ref = "../../../second/third";
	test_norm(&orig[7], 23, PATH_MAX, ref);
	test_norm(&orig[7], 24, PATH_MAX, ref);

	ref = "/";
	test_norm(&orig[6], 3, PATH_MAX, ref);
	test_norm(&orig[6], 4, PATH_MAX, ref);
	test_norm(&orig[6], 12, PATH_MAX, ref);

	ref = "/second";
	test_norm(&orig[6], 18, PATH_MAX, ref);
}
#endif
