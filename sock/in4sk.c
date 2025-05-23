/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/in4sk.h"
#include <string.h>
#include <errno.h>

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_in4sk_assert_intern(_expr) \
	stroll_assert("etux:in4sk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_in4sk_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

ssize_t
etux_in4sk_make_addr(struct sockaddr_in * __restrict addr,
                     const char * __restrict         string)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(string);

	size_t len;
	int    ret __unused;

	len = strnlen(string, ETUX_IN4SK_ADDR_MAX);
	if (!len)
		return -ENODATA;
	else if (len >= ETUX_IN4SK_ADDR_MAX)
		return -ENAMETOOLONG;

	ret = inet_pton(AF_INET, string, &addr->sin_addr);
	if (ret == 1) {
		addr->sin_family = AF_INET;
		return sizeof(*addr);
	}

	etux_in4sk_assert_intern(!ret);

	return -EINVAL;
}
