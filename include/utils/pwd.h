#ifndef _UTILS_PWD_H
#define _UTILS_PWD_H

#include <utils/cdefs.h>
#include <utils/string.h>
#include <pwd.h>
#include <grp.h>

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

extern int
upwd_parse_uid(const char * __restrict string, uid_t * __restrict uid)
	__upwd_nonull(1, 2) __nothrow __leaf;

extern int
upwd_parse_gid(const char * __restrict string, gid_t * __restrict gid)
	__upwd_nonull(1, 2) __nothrow __leaf;

static inline ssize_t __upwd_nonull(1) __pure __nothrow
upwd_validate_user_name(const char * __restrict name)
{
	/*
	 * TODO: validate string content.
	 * Should match something like following regexp: [a-z][-a-z0-9]*
	 */
	return ustr_parse(name, LOGIN_NAME_MAX);
}

extern const struct passwd *
upwd_get_user_byid(uid_t uid);

extern const struct passwd *
upwd_get_user_byname(const char * __restrict name) __upwd_nonull(1);

static inline int __upwd_nonull(1, 2)
upwd_get_uid_byname(const char * __restrict name, uid_t * __restrict uid)
{
	upwd_assert(upwd_validate_user_name(name) > 0);
	upwd_assert(uid);

	const struct passwd * pwd;

	pwd = upwd_get_user_byname(name);
	if (!pwd)
		return -errno;

	*uid = pwd->pw_uid;

	return 0;
}

static inline ssize_t __upwd_nonull(1) __pure __nothrow
upwd_validate_group_name(const char * __restrict name)
{
	/*
	 * TODO: validate string content.
	 * Should match something like following regexp: [a-z][-a-z0-9]*
	 */
	return ustr_parse(name, LOGIN_NAME_MAX);
}

extern const struct group *
upwd_get_group_byid(gid_t gid);

extern const struct group *
upwd_get_group_byname(const char * __restrict name) __upwd_nonull(1);

static inline int __upwd_nonull(1, 2)
upwd_get_gid_byname(const char * __restrict name, gid_t * __restrict gid)
{
	upwd_assert(upwd_validate_group_name(name) > 0);
	upwd_assert(gid);

	const struct group * grp;

	grp = upwd_get_group_byname(name);
	if (!grp)
		return -errno;

	*gid = grp->gr_gid;

	return 0;
}

#endif /* _UTILS_PWD_H */
