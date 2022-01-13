#ifndef _UTILS_PWD_H
#define _UTILS_PWD_H

#include <utils/cdefs.h>
#include <utils/string.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __upwd_nonull(_arg_index, ...)

#define __upwd_pure

#define upwd_assert(_expr) \
	uassert("upwd", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __upwd_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define __upwd_pure \
	__pure

#define upwd_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline ssize_t __upwd_nonull(1) __pure __nothrow
upwd_validate_user(const char * __restrict user)
{
	/*
	 * TODO: validate string content.
	 * Should match something like following regexp: [a-z][-a-z0-9]*
	 */
	return ustr_parse(user, LOGIN_NAME_MAX);
}

extern int __upwd_nonull(1)
upwd_get_uid(const char * __restrict user, uid_t * __restrict uid);

static inline ssize_t __upwd_nonull(1) __pure __nothrow
upwd_validate_group(const char * __restrict user)
{
	/*
	 * TODO: validate string content.
	 * Should match something like following regexp: [a-z][-a-z0-9]*
	 */
	return ustr_parse(user, LOGIN_NAME_MAX);
}

extern int __upwd_nonull(1)
upwd_get_gid(const char * __restrict group, gid_t * __restrict gid);

#endif /* _UTILS_PWD_H */
