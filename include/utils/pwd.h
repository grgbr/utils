/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * System password / group database interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      13 Jan 2022
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_PWD_H
#define _UTILS_PWD_H

#include <utils/string.h>
#include <pwd.h>
#include <grp.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define upwd_assert_api(_expr) \
	stroll_assert("utils:upwd", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define upwd_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

extern int
upwd_parse_uid(const char * __restrict string, uid_t * __restrict uid)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
ssize_t
upwd_validate_user_name(const char * __restrict name)
{
	/*
	 * TODO: validate string content.
	 * Should match something like following regexp: [a-z][-a-z0-9]*
	 */
	return ustr_parse(name, LOGIN_NAME_MAX);
}

extern const struct passwd *
upwd_get_user_byid(uid_t uid) __warn_result;

extern const struct passwd *
upwd_get_user_byname(const char * __restrict name)
	__utils_nonull(1) __warn_result;

extern int
upwd_get_uid_byname(const char * __restrict name, uid_t * __restrict uid)
	__utils_nonull(1, 2) __warn_result;

extern int
upwd_parse_gid(const char * __restrict string, gid_t * __restrict gid)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
ssize_t
upwd_validate_group_name(const char * __restrict name)
{
	/*
	 * TODO: validate string content.
	 * Should match something like following regexp: [a-z][-a-z0-9]*
	 */
	return ustr_parse(name, LOGIN_NAME_MAX);
}

extern const struct group *
upwd_get_group_byid(gid_t gid) __warn_result;

extern const struct group *
upwd_get_group_byname(const char * __restrict name)
	__utils_nonull(1) __warn_result;

extern int
upwd_get_gid_byname(const char * __restrict name, gid_t * __restrict gid)
	__utils_nonull(1, 2) __warn_result;

#endif /* _UTILS_PWD_H */
