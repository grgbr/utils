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

#include <utils/sock.h>
#include <netinet/in.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_in4sk_assert_api(_expr) \
	stroll_assert("etux:in4sk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_in4sk_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_in4sk_assert_intern(_expr) \
	stroll_assert("etux:in4sk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_in4sk_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

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
#define ETUX_IN4SK_ADDR(_net, _xport) \
	((struct sockaddr_in) { \
		.sin_family      = AF_INET, \
		.sin_port        = htons(_xport), \
		.sin_addr.s_addr = htonl(_net) \
	 })

#define ETUX_IN4SK_QUAD(_net0, _net1, _net2, _net3) \
	(((_net0) << 24) | ((_net1) << 16) | ((_net2) << 8) | (_net3))

extern void
etux_in4sk_setup_addr(struct sockaddr_in * __restrict addr,
                      in_addr_t                       host,
                      in_port_t                       serv)
	__utils_nonull(1) __utils_nothrow __leaf __export_public;

#if defined(CONFIG_ETUX_NETDB)

#include <utils/netdb.h>

static inline __utils_nonull(1, 2) __warn_result
int
etux_in4sk_make_host(struct sockaddr_in * __restrict addr,
                     const char * __restrict         string,
                     int                             flags)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(string);
	etux_in4sk_assert_api(!(flags & ~(AI_NUMERICHOST |
	                                  AI_PASSIVE |
	                                  AI_ADDRCONFIG |
	                                  AI_IDN)));

	int err;

	err = etux_netdb_make_host(AF_INET,
	                           string,
	                           (struct sockaddr *)addr,
	                           sizeof(*addr),
	                           flags);
	if (!err) {
		etux_in4sk_assert_intern(addr->sin_family == AF_INET);
		return 0;
	}

	return err;
}

static inline __utils_nonull(1, 2) __warn_result
ssize_t
etux_in4sk_host_name(
	const struct sockaddr_in * __restrict addr,
	char                                  host[__restrict_arr NI_MAXHOST],
	int                                   flags)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(addr->sin_family == AF_INET);
	etux_in4sk_assert_api(host);
	etux_in4sk_assert_api(!(flags & ~(NI_NAMEREQD |
	                                  NI_NOFQDN |
	                                  NI_NUMERICHOST)));
	etux_in4sk_assert_api((flags & (NI_NAMEREQD | NI_NUMERICHOST)) !=
	                      (NI_NAMEREQD | NI_NUMERICHOST));

	return etux_netdb_host_name((const struct sockaddr *)addr,
	                            sizeof(*addr),
	                            host,
	                            flags);
}

static inline __utils_nonull(1, 2) __warn_result
int
etux_in4sk_make_serv(struct sockaddr_in * __restrict addr,
                     const char * __restrict         serv,
                     const char * __restrict         proto,
                     int                             flags)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(serv);
	etux_in4sk_assert_api(!(flags & ~AI_NUMERICSERV));

	return etux_netdb_parse_serv(serv, proto, &addr->sin_port, flags);
}

static inline __utils_nonull(1, 3) __warn_result
ssize_t
etux_in4sk_serv_name(
	const struct sockaddr_in * __restrict addr,
	const char * __restrict               proto,
	char                                  serv[__restrict_arr NI_MAXSERV],
	int                                   flags)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(addr->sin_family == AF_INET);
	etux_in4sk_assert_api(serv);
	etux_in4sk_assert_api(!(flags & ~NI_NUMERICSERV));

	return etux_netdb_serv_name(addr->sin_port, proto, serv, flags);
}

extern int
etux_in4sk_make_addr(struct sockaddr_in * __restrict addr,
                     const char * __restrict         host,
                     const char * __restrict         serv,
                     const char * __restrict         proto,
                     int                             flags)
	__utils_nonull(1, 2, 3) __warn_result __export_public;

extern ssize_t
etux_in4sk_addr_name(
	const struct sockaddr_in * __restrict addr,
	const char * __restrict               proto,
	char                                  name[__restrict_arr ETUX_NETDB_NAME_MAX],
	int                                   flags)
	__utils_nonull(1, 3) __warn_result __export_public;

#endif /* defined(CONFIG_ETUX_NETDB) */

static inline __utils_nonull(2) __warn_result
int
etux_in4sk_connect(int fd, const struct sockaddr_in * __restrict peer)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api(peer);
	etux_in4sk_assert_api(peer->sin_family == AF_INET);

	return etux_sock_connect(fd,
	                         (const struct sockaddr *)peer,
	                         sizeof(*peer));
}

static inline __warn_result
int
etux_in4sk_accept(int fd, struct sockaddr_in * __restrict peer, int flags)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	socklen_t sz = sizeof(*peer);

	return etux_sock_accept(fd, (struct sockaddr *)peer, &sz, flags);
}

static inline __utils_nothrow __warn_result
int
etux_in4sk_listen(int fd, int backlog)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api(backlog >= 0);

	return etux_sock_listen(fd, backlog);
}

#if defined(CONFIG_ETUX_NETIF)

static inline __utils_nonull(2) __utils_nothrow __warn_result
int
etux_in4sk_bind_netif(int fd, const char * __restrict iface, size_t len)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api(iface);
	etux_in4sk_assert_api(len);
	etux_in4sk_assert_api(len < IFNAMSIZ);
	etux_in4sk_assert_api(strnlen(iface, IFNAMSIZ) == len);

	return etux_sock_bind_netif(fd, iface, len);
}

#endif /* defined(CONFIG_ETUX_NETIF) */

static inline __utils_nonull(2) __utils_nothrow __warn_result
int
etux_in4sk_bind(int fd, const struct sockaddr_in * __restrict local)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api(local);
	etux_in4sk_assert_api(local->sin_family == AF_INET);

	return etux_sock_bind(fd,
	                      (const struct sockaddr *)local,
	                      sizeof(*local));
}

static inline __utils_nothrow __warn_result
int
etux_in4sk_open(int type, int proto, int flags)
{
	etux_in4sk_assert_api((type == SOCK_DGRAM) ||
	                      (type == SOCK_STREAM) ||
	                      (type == SOCK_RAW));
	etux_in4sk_assert_api(proto >= IPPROTO_IP);
	etux_in4sk_assert_api(proto < IPPROTO_MAX);
	etux_in4sk_assert_api((type == SOCK_RAW) ^ !!proto);
	etux_in4sk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	return etux_sock_open(AF_INET, type, proto, flags);
}

static inline __utils_nothrow
void
etux_in4sk_shutdown(int fd, int how)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api((how == SHUT_RD) ||
	                      (how == SHUT_WR) ||
	                      (how == SHUT_RDWR));

	etux_sock_shutdown(fd, how);
}

static inline
int
etux_in4sk_close(int fd)
{
	etux_in4sk_assert_api(fd >= 0);

	return etux_sock_close(fd);
}

#endif /* _UTILS_IN4SK_H */
