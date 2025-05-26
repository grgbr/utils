/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Netif interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      25 May 2025
 * @copyright Copyright (C) 2017-2025 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _ETUX_NETIF_H
#define _ETUX_NETIF_H

#include <utils/cdefs.h>
#include <net/if.h>
#include <errno.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_netif_assert_api(_expr) \
	stroll_assert("etux:netif", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_netif_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

extern int
etux_netif_validate(const char * __restrict string)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__warn_result
	__export_public;

static inline __utils_nonull(1) __utils_nothrow __warn_result
int
etux_netif_parse(const char * __restrict string)
{
	etux_netif_assert_api(string);

	return if_nametoindex(string) ? 0 : -errno;
}

#endif /* _ETUX_NETIF_H */
