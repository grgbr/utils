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
#include <netinet/in.h>

/*
 * The following may be used with ETUX_IN4SK_ADDR_FROM_HOST:
 * - INADDR_ANY
 * - INADDR_BROADCAST
 * - INADDR_NONE
 * - INADDR_DUMMY
 * - INADDR_LOOPBACK
 * - INADDR_UNSPEC_GROUP
 * - INADDR_ALLHOSTS_GROUP
 * - INADDR_ALLRTRS_GROUP
 * - INADDR_ALLSNOOPERS_GROUP
 * - INADDR_MAX_LOCAL_GROUP
 * See <netinet/in.h> for more infos...
 */
#define ETUX_IN4SK_MAKE_ADDR(_net, _xport) \
	((struct sockaddr_in) { \
		.sin_family      = AF_INET, \
		.sin_port        = htons(_xport), \
		.sin_addr.s_addr = htonl(_net) \
	 })

#define ETUX_IN4SK_MAKE_ADDR_FROM_QUAD(_net0, _net1, _net2, _net3, _xport) \
	ETUX_IN4SK_MAKE_ADDR(((_net0) << 24) | \
	                     ((_net1) << 16) | \
	                     ((_net2) << 8) | \
	                     (_net3), \
	                     _xport)

extern int
etux_in4sk_parse_addr(struct sockaddr_in * __restrict addr,
                      const char * __restrict         string)
	__utils_nonull(1, 2)
	__utils_nothrow
	__warn_result
	__leaf
	__export_public;

extern ssize_t
etux_in4sk_validate_host(const char * __restrict string)
	__utils_nonull(1) __warn_result __export_public;

extern int
etux_in4sk_resolv_host(struct sockaddr_in * __restrict addr,
                       const char * __restrict         string,
                       int                             flags)
	__utils_nonull(1, 2) __warn_result __export_public;

extern int
etux_in4sk_parse_port(struct sockaddr_in * __restrict addr,
                      const char * __restrict         string)
	__utils_nonull(1, 2)
	__utils_nothrow
	__leaf
	__warn_result
	__export_public;

extern int
etux_in4sk_resolv_serv(struct sockaddr_in * __restrict addr,
                       const char * __restrict         string)
	__utils_nonull(1, 2) __warn_result __export_public;

extern void
etux_in4sk_make_addr(struct sockaddr_in * __restrict addr,
                     in_addr_t                       net,
                     in_port_t                       xport)
	__utils_nonull(1) __utils_nothrow __leaf __export_public;

extern int
etux_in4sk_connect(int fd, const struct sockaddr_in * __restrict peer)
	__utils_nonull(2) __warn_result __export_public;

extern int
etux_in4sk_accept(int fd, struct sockaddr_in * __restrict peer, int flags)
	__warn_result __export_public;

extern int
etux_in4sk_bind(int fd, const struct sockaddr_in * __restrict local)
	__utils_nonull(2) __utils_nothrow __leaf __warn_result __export_public;

extern int
etux_in4sk_open(int type, int proto, int flags)
	__utils_nothrow __leaf __warn_result __export_public;

extern int
etux_in4sk_shutdown(int fd, int how)
	__utils_nothrow __warn_result __leaf __export_public;

extern int
etux_in4sk_close(int fd)
	__export_public;

#endif /* _UTILS_IN4SK_H */
