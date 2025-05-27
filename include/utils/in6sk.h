/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * IPv6 socket interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      27 May 2025
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _ETUX_IN6SK_H
#define _ETUX_IN6SK_H

#include <utils/cdefs.h>
#include <netinet/in.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_in6sk_assert_api(_expr) \
	stroll_assert("etux:in6sk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_in6sk_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_in6sk_assert_intern(_expr) \
	stroll_assert("etux:in6sk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_in6sk_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

/*
 * The following may be used with ETUX_IN6SK_ADDR_FROM_HOST:
 * - IN6ADDR_ANY_INIT
 * - IN6ADDR_LOOPBACK_INIT
 * See <netinet/in.h> for more infos...
 */
#define ETUX_IN6SK_ADDR(_net, _flow, _scope, _xport) \
	((struct sockaddr_in6) { \
		.sin6_family   = AF_INET6, \
		.sin6_port     = htons(_xport), \
		.flowinfo      = _flow, \
		.sin6_scope_id = _scope \
	 })

#define ETUX_IN6SK_ADDR_FROM_HEXTETS(_net, _flow, _scope, _xport) \
	ETUX_IN6SK_MAKE_ADDR({ .s6_addr16 = _net }, _flow, _scope, _xport)

#define ETUX_IN6SK_ADDR_FROM_OCTETS(_net, _flow, _scope, _xport) \
	ETUX_IN6SK_MAKE_ADDR({ .s6_addr = _net }, _flow, _scope, _xport)

extern void
etux_in6sk_setup_addr(struct sockaddr_in6 * __restrict   addr,
                      const struct in6_addr * __restrict host,
                      uint32_t                           flow,
                      uint32_t                           scope,
                      in_port_t                          serv)
	__utils_nonull(1, 2) __utils_nothrow __leaf __export_public;

#if defined(CONFIG_ETUX_NETDB)

#include <utils/netdb.h>

static inline __utils_nonull(1, 2) __warn_result
int
etux_in6sk_make_host(struct sockaddr_in6 * __restrict addr,
                     const char * __restrict          string,
                     int                              flags)
{
	etux_in6sk_assert_api(addr);
	etux_in6sk_assert_api(string);
	etux_in6sk_assert_api(!(flags & ~(AI_NUMERICHOST |
	                                  AI_PASSIVE |
	                                  AI_ADDRCONFIG |
	                                  AI_IDN)));

	int err;

	err = etux_netdb_make_host(AF_INET6,
	                           string,
	                           (struct sockaddr *)addr,
	                           sizeof(*addr),
	                           flags);
	if (!err) {
		etux_in6sk_assert_intern(addr->sin6_family == AF_INET6);
		return 0;
	}

	return err;
}

static inline __utils_nonull(1, 2) __warn_result
ssize_t
etux_in6sk_host_name(
	const struct sockaddr_in6 * __restrict addr,
	char                                   host[__restrict_arr NI_MAXHOST],
	int                                    flags)
{
	etux_in6sk_assert_api(addr);
	etux_in6sk_assert_api(addr->sin6_family == AF_INET6);
	etux_in6sk_assert_api(host);
	etux_in6sk_assert_api(!(flags & ~(NI_NAMEREQD |
	                                  NI_NOFQDN |
	                                  NI_NUMERICHOST)));
	etux_in6sk_assert_api((flags & (NI_NAMEREQD | NI_NUMERICHOST)) !=
	                      (NI_NAMEREQD | NI_NUMERICHOST));

	return etux_netdb_host_name((const struct sockaddr *)addr,
	                            sizeof(*addr),
	                            host,
	                            flags);
}

static inline __utils_nonull(1, 2) __warn_result
int
etux_in6sk_make_serv(struct sockaddr_in6 * __restrict addr,
                     const char * __restrict          serv,
                     const char * __restrict          proto,
                     int                              flags)
{
	etux_in6sk_assert_api(addr);
	etux_in6sk_assert_api(serv);
	etux_in6sk_assert_api(!(flags & ~AI_NUMERICSERV));

	return etux_netdb_parse_serv(serv, proto, &addr->sin6_port, flags);
}

static inline __utils_nonull(1, 3) __warn_result
ssize_t
etux_in6sk_serv_name(
	const struct sockaddr_in6 * __restrict addr,
	const char * __restrict                proto,
	char                                   serv[__restrict_arr NI_MAXSERV],
	int                                    flags)
{
	etux_in6sk_assert_api(addr);
	etux_in6sk_assert_api(addr->sin6_family == AF_INET6);
	etux_in6sk_assert_api(serv);
	etux_in6sk_assert_api(!(flags & ~NI_NUMERICSERV));

	return etux_netdb_serv_name(addr->sin6_port, proto, serv, flags);
}

extern int
etux_in6sk_make_addr(struct sockaddr_in6 * __restrict addr,
                     const char * __restrict          host,
                     const char * __restrict          serv,
                     const char * __restrict          proto,
                     int                              flags)
	__utils_nonull(1, 2, 3) __warn_result __export_public;

#define ETUX_NETDB_NAME_MAX \
	(1U + (NI_MAXHOST - 1U) + 2U + NI_MAXSERV)

extern ssize_t
etux_in6sk_addr_name(
	const struct sockaddr_in6 * __restrict addr,
	const char * __restrict                proto,
	char                                   name[__restrict_arr ETUX_NETDB_NAME_MAX],
	int                                    flags)
	__utils_nonull(1, 3) __warn_result __export_public;

#endif /* defined(CONFIG_ETUX_NETDB) */

extern int
etux_in6sk_connect(int fd, const struct sockaddr_in6 * __restrict peer)
	__utils_nonull(2) __warn_result __export_public;

extern int
etux_in6sk_accept(int fd, struct sockaddr_in6 * __restrict peer, int flags)
	__warn_result __export_public;

extern int
etux_in6sk_bind(int fd, const struct sockaddr_in6 * __restrict local)
	__utils_nonull(2) __utils_nothrow __leaf __warn_result __export_public;

#if defined(CONFIG_ETUX_NETIF)

extern int
etux_in6sk_bind_netif(int fd, const char * __restrict iface, size_t len)
	__utils_nonull(2) __utils_nothrow __leaf __warn_result __export_public;

#endif /* defined(CONFIG_ETUX_NETIF) */

extern int
etux_in6sk_open(int type, int proto, int flags)
	__utils_nothrow __leaf __warn_result __export_public;

extern int
etux_in6sk_shutdown(int fd, int how)
	__utils_nothrow __warn_result __leaf __export_public;

extern int
etux_in6sk_close(int fd)
	__export_public;

#endif /* _UTILS_IN6SK_H */
