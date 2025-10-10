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

#include <utils/sock.h>
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
 * The following may be used with ETUX_IN6SK_ADDR _net argument:
 * - IN6ADDR_ANY_INIT or the in6addr_any variable
 * - IN6ADDR_LOOPBACK_INIT or the in6addr_loopback
 * Expected in network byte order.
 * See <netinet/in.h> for more infos...
 *
 * The following may be used with ETUX_IN6SK_ADDR _flow argument:
 * - IPV6_FLOWINFO_FLOWLABEL
 * - IPV6_FLOWINFO_PRIORITY
 * - IPV6_PRIORITY_UNCHARACTERIZED
 * - IPV6_PRIORITY_FILLER
 * - IPV6_PRIORITY_UNATTENDED
 * - IPV6_PRIORITY_RESERVED1
 * - IPV6_PRIORITY_BULK
 * - IPV6_PRIORITY_RESERVED2
 * - IPV6_PRIORITY_INTERACTIVE
 * - IPV6_PRIORITY_CONTROL
 * - IPV6_PRIORITY_8
 * - IPV6_PRIORITY_9
 * - IPV6_PRIORITY_10
 * - IPV6_PRIORITY_11
 * - IPV6_PRIORITY_12
 * - IPV6_PRIORITY_13
 * - IPV6_PRIORITY_14
 * - IPV6_PRIORITY_15
 * Expected in host byte order (to be able to use the above macros).
 * See <linux/in6.h> for further infos...
 */
#define ETUX_IN6SK_ADDR(_net, _flow, _scope, _xport) \
	((struct sockaddr_in6) { \
		.sin6_family   = AF_INET6, \
		.sin6_port     = htons(_xport), \
		.sin6_flowinfo = htonl(_flow), \
		.sin6_addr     = _net, \
		.sin6_scope_id = _scope \
	 })

#define ETUX_IN6SK_HEXTETS(_net0, _net1, _net2, _net3, \
                           _net4, _net5, _net6, _net7) \
	{ \
		.s6_addr16 = { \
			htons(_net0), htons(_net1), htons(_net2), htons(_net3), \
			htons(_net4), htons(_net5), htons(_net6), htons(_net7), \
		} \
	}

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
	                                  AI_V4MAPPED |
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

extern ssize_t
etux_in6sk_addr_name(
	const struct sockaddr_in6 * __restrict addr,
	const char * __restrict                proto,
	char                                   name[__restrict_arr ETUX_NETDB_NAME_MAX],
	int                                    flags)
	__utils_nonull(1, 3) __warn_result __export_public;

#endif /* defined(CONFIG_ETUX_NETDB) */

static inline __utils_nonull(2) __warn_result
int
etux_in6sk_connect(int fd, const struct sockaddr_in6 * __restrict peer)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api(peer);
	etux_in6sk_assert_api(peer->sin6_family == AF_INET6);

	return etux_sock_connect(fd,
	                         (const struct sockaddr *)peer,
	                         sizeof(*peer));
}

static inline __warn_result
int
etux_in6sk_accept(int fd, struct sockaddr_in6 * __restrict peer, int flags)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	socklen_t sz = sizeof(*peer);

	return etux_sock_accept(fd, (struct sockaddr *)peer, &sz, flags);
}

static inline __utils_nothrow __warn_result
int
etux_in6sk_listen(int fd, int backlog)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api(backlog >= 0);

	return etux_sock_listen(fd, backlog);
}

#if defined(CONFIG_ETUX_NETIF)

static inline __utils_nonull(2) __utils_nothrow __warn_result
int
etux_in6sk_bind_netif(int fd, const char * __restrict iface, size_t len)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api(iface);
	etux_in6sk_assert_api(len);
	etux_in6sk_assert_api(len < IFNAMSIZ);
	etux_in6sk_assert_api(strnlen(iface, IFNAMSIZ) == len);

	return etux_sock_bind_netif(fd, iface, len);
}

#endif /* defined(CONFIG_ETUX_NETIF) */

static inline __utils_nonull(2) __utils_nothrow __warn_result
int
etux_in6sk_bind(int fd, const struct sockaddr_in6 * __restrict local)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api(local);
	etux_in6sk_assert_api(local->sin6_family == AF_INET6);

	return etux_sock_bind(fd,
	                      (const struct sockaddr *)local,
	                      sizeof(*local));
}

static inline __utils_nothrow __warn_result
int
etux_in6sk_open(int type, int proto, int flags)
{
	etux_in6sk_assert_api((type == SOCK_DGRAM) ||
	                      (type == SOCK_STREAM) ||
	                      (type == SOCK_RAW));
	etux_in6sk_assert_api(proto >= IPPROTO_IP);
	etux_in6sk_assert_api(proto < IPPROTO_MAX);
	etux_in6sk_assert_api((type == SOCK_RAW) ^ !!proto);
	etux_in6sk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	return etux_sock_open(AF_INET6, type, proto, flags);
}

static inline __utils_nothrow
void
etux_in6sk_shutdown(int fd, int how)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api((how == SHUT_RD) ||
	                      (how == SHUT_WR) ||
	                      (how == SHUT_RDWR));

	etux_sock_shutdown(fd, how);
}

static inline
int
etux_in6sk_close(int fd)
{
	etux_in6sk_assert_api(fd >= 0);

	return etux_sock_close(fd);
}

#endif /* _UTILS_IN6SK_H */
