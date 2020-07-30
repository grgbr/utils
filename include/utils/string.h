#ifndef _UTILS_STRING_H
#define _UTILS_STRING_H

#include <utils/cdefs.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define ustr_assert(_expr) \
	uassert("ustr", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define ustr_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline ssize_t
ustr_parse(const char *str, size_t max_size)
{
	ustr_assert(str);
	ustr_assert(max_size);

	size_t len;

	len = strnlen(str, max_size);

	return (len != max_size) ? (ssize_t)len : -ENAMETOOLONG;
}

extern char *
ustr_clone(const char *orig, size_t len);

extern char *
ustr_sized_clone(const char *orig, size_t max_size);

#endif /* _UTILS_STRING_H */
