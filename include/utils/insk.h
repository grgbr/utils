/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * IPv4/IPv6 socket interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      25 May 2025
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _ETUX_INSK_H
#define _ETUX_INSK_H

#include <utils/cdefs.h>
#include <netinet/in.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_insk_assert_api(_expr) \
	stroll_assert("etux:insk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_insk_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_insk_assert_intern(_expr) \
	stroll_assert("etux:insk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_insk_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

#if defined(CONFIG_ETUX_NETDB)

#include <utils/netdb.h>

extern int
etux_insk_make_host(__SOCKADDR_ARG               addr,
                    socklen_t                    size,
                    const char * __restrict      string,
                    int                          flags)
	__utils_nonull(3) __warn_result __export_public;

extern ssize_t
etux_insk_host_name(
	__CONST_SOCKADDR_ARG               addr,
	char                               host[__restrict_arr NI_MAXHOST],
	int                                flags)
	__utils_nonull(2) __warn_result __export_public;

extern int
etux_insk_make_serv(__SOCKADDR_ARG               addr,
                    const char * __restrict      serv,
                    const char * __restrict      proto,
                    int                          flags)
	__utils_nonull(2) __warn_result __export_public;

extern ssize_t
etux_insk_serv_name(
	__CONST_SOCKADDR_ARG               addr,
	const char * __restrict            proto,
	char                               serv[__restrict_arr NI_MAXSERV],
	int                                flags)
	__utils_nonull(3) __warn_result __export_public;

#endif /* defined(CONFIG_ETUX_NETDB) */

#endif /* _UTILS_INSK_H */
