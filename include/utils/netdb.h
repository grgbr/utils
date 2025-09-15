/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Network databases interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      25 May 2025
 * @copyright Copyright (C) 2017-2025 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _ETUX_NETDB_H
#define _ETUX_NETDB_H

#include <utils/cdefs.h>
#include <netinet/in.h>
#include <netdb.h>

#define ETUX_NETDB_NAME_MAX \
	(1U + (NI_MAXHOST - 1U) + 2U + NI_MAXSERV)

extern int
etux_netdb_validate_host(const char * __restrict string)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__warn_result
	__export_public;

extern int
etux_netdb_make_host(int                          family,
                     const char * __restrict      host,
                     struct sockaddr * __restrict addr,
                     socklen_t                    size,
                     int                          flags)
	__utils_nonull(2, 3) __warn_result __export_public;

extern ssize_t
etux_netdb_host_name(
	const struct sockaddr * __restrict addr,
	socklen_t                          size,
	char                               host[__restrict_arr NI_MAXHOST],
	int                                flags)
	__utils_nonull(1, 3) __warn_result __export_public;

extern int
etux_netdb_validate_proto(const char * __restrict string)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__warn_result
	__export_public;

extern int
etux_netdb_parse_proto(const char * __restrict string, int * __restrict proto)
	__utils_nonull(1, 2) __warn_result __export_public;

extern int
etux_netdb_validate_serv(const char * __restrict string)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__warn_result
	__export_public;

extern int
etux_netdb_parse_serv(const char * __restrict serv,
                      const char * __restrict proto,
                      in_port_t * __restrict  port,
                      int                     flags)
	__utils_nonull(1, 3) __warn_result __export_public;

extern ssize_t
etux_netdb_serv_name(in_port_t               port,
                     const char * __restrict proto,
                     char                    serv[__restrict_arr NI_MAXSERV],
                     int                     flags)
	__utils_nonull(3) __warn_result __export_public;

#endif /* _ETUX_NETDB_H */
