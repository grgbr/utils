/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * IPv4 socket interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 May 2025
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _ETUX_IN4SK_H
#define _ETUX_IN4SK_H

#include <utils/cdefs.h>
#include <arpa/inet.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_in4sk_assert_api(_expr) \
	stroll_assert("etux:in4sk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_in4sk_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#define ETUX_IN4SK_ADDR_MAX \
	(16U)

extern ssize_t
etux_in4sk_make_addr(struct sockaddr_in * __restrict addr,
                     const char * __restrict         string)
	__utils_nonull(1, 2)
	__utils_nothrow
	__warn_result
	__leaf
	__export_public;

#endif /* _UTILS_IN4SK_H */
