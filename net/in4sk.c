/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/in4sk.h"
#include "syssk.h"

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_in4sk_assert_intern(_expr) \
	stroll_assert("etux:in4sk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_in4sk_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

MAKE ME STATIC INLINE
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
	                           flags);
	if (!err) {
		etux_in4sk_assert_intern(addr->sin_family == AF_INET);
		return 0;
	}

	return err;
}
FINISHE ME!!
int
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
	etux_in4sk_assert_api(!((flags & NI_NAMEREQD) &&
	                        (flags & NI_NUMERICHOST)));

	return etux_syssk_getnameinfo((const struct sockaddr *)addr,
	                              sizeof(*addr),
	                              host,
	                              NULL,
	                              flags);
}

void
etux_in4sk_setup_addr(struct sockaddr_in * __restrict addr,
                      in_addr_t                       host,
                      in_port_t                       serv)
{
	etux_in4sk_assert_api(addr);

	addr->sin_family = AF_INET;
	addr->sin_port = htons(serv);
	addr->sin_addr.s_addr = htonl(host);
}

/*
 * Do not use getaddrinfo() as service port resolution is not properly handled
 * (unpredictable port number returned when passing out-of-range numeric service
 * string).
 *
 * Proto is optional and may be passed as NULL (see getservbyname(3)).
 */
int
etux_in4sk_make_addr(struct sockaddr_in * __restrict addr,
                     const char * __restrict         host,
                     const char * __restrict         serv,
                     const char * __restrict         proto,
                     int                             flags)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(host || serv);
	etux_in4sk_assert_api(!(flags & ~(AI_NUMERICHOST |
	                                  AI_NUMERICSERV |
	                                  AI_PASSIVE |
	                                  AI_ADDRCONFIG |
	                                  AI_IDN)));

	int err;

	err = etux_in4sk_make_host(addr, host, flags & (AI_NUMERICHOST |
	                                                AI_PASSIVE |
	                                                AI_ADDRCONFIG |
	                                                AI_IDN));
	if (err)
		return err;

	return etux_in4sk_make_serv(addr, serv, proto, flags & AI_NUMERICSERV);
}

int
etux_in4sk_addr_name(
	const struct sockaddr_in * __restrict addr,
	char                                  host[__restrict_arr NI_MAXHOST],
	char                                  serv[__restrict_arr NI_MAXSERV],
	int                                   flags)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(addr->sin_family == AF_INET);
	etux_in4sk_assert_api(host);
	etux_in4sk_assert_api(serv);
	etux_in4sk_assert_api(!(flags & ~(NI_NAMEREQD |
	                                  NI_NOFQDN |
	                                  NI_NUMERICHOST |
	                                  NI_NUMERICSERV)));
	etux_in4sk_assert_api(!((flags & NI_NAMEREQD) &&
	                        (flags & NI_NUMERICHOST)));

	return etux_syssk_getnameinfo((const struct sockaddr *)addr,
	                              sizeof(*addr),
	                              host,
	                              serv,
	                              flags);
}

int
etux_in4sk_connect(int fd, const struct sockaddr_in * __restrict peer)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api(peer);
	etux_in4sk_assert_api(peer->sin_family == AF_INET);

	return etux_syssk_connect(fd,
	                          (const struct sockaddr *)peer,
	                          sizeof(*peer));
}

int
etux_in4sk_accept(int fd, struct sockaddr_in * __restrict peer, int flags)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	socklen_t sz = sizeof(*peer);
	int       nevv;

	nevv = etux_syssk_accept(fd, (struct sockaddr *)peer, &sz, flags);
	if (nevv >= 0)
		etux_in4sk_assert_api(sz == sizeof(*peer));

	return nevv;
}

int
etux_in4sk_bind(int fd, const struct sockaddr_in * __restrict local)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api(local);
	etux_in4sk_assert_api(local->sin_family == AF_INET);

	return etux_syssk_bind(fd,
	                       (const struct sockaddr *)local,
	                       sizeof(*local));
}

int
etux_in4sk_bind_netif(int fd, const char * __restrict iface, size_t len)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api(iface);
	etux_in4sk_assert_api(len);
	etux_in4sk_assert_api(len < IFNAMSIZ);
	etux_in4sk_assert_api(strnlen(iface, IFNAMSIZ) == len);

	return etux_syssk_bind_netif(fd, iface, len);
}

int
etux_in4sk_open(int type, int proto, int flags)
{
	etux_in4sk_assert_api((type == SOCK_DGRAM) ||
	                      (type == SOCK_STREAM) ||
	                      (type == SOCK_RAW));
	etux_in4sk_assert_api(proto >= IPPROTO_MAX);
	etux_in4sk_assert_api(proto < IPPROTO_MAX);
	etux_in4sk_assert_api((type == SOCK_RAW) ^ !!proto);
	etux_in4sk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	return etux_syssk_open(AF_INET, type, proto, flags);
}

int
etux_in4sk_shutdown(int fd, int how)
{
	etux_in4sk_assert_api(fd >= 0);
	etux_in4sk_assert_api((how == SHUT_RD) ||
	                      (how == SHUT_WR) ||
	                      (how == SHUT_RDWR));

	return etux_syssk_shutdown(fd, how);
}

int
etux_in4sk_close(int fd)
{
	etux_in4sk_assert_api(fd >= 0);

	return etux_syssk_close(fd);
}
