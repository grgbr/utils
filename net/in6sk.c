/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/in6sk.h"
#include "syssk.h"

void
etux_in6sk_setup_addr(struct sockaddr_in6 * __restrict   addr,
                      const struct in6_addr * __restrict host,
                      uint32_t                           flow,
                      uint32_t                           scope,
                      in_port_t                          serv)
{
	etux_in6sk_assert_api(addr);
	etux_in6sk_assert_api(host);

	addr->sin6_family = AF_INET6;
	addr->sin6_port = htons(serv);
	addr->sin6_flowinfo = flow,
	addr->sin6_addr = *host;
	addr->sin6_scope_id = scope;
}

#if defined(CONFIG_ETUX_NETDB)

/*
 * Do not use getaddrinfo() as service port resolution is not properly handled
 * (unpredictable port number returned when passing out-of-range numeric service
 * string).
 *
 * Proto is optional and may be passed as NULL (see getservbyname(3)).
 */
int
etux_in6sk_make_addr(struct sockaddr_in6 * __restrict addr,
                     const char * __restrict          host,
                     const char * __restrict          serv,
                     const char * __restrict          proto,
                     int                              flags)
{
	etux_in6sk_assert_api(addr);
	etux_in6sk_assert_api(host);
	etux_in6sk_assert_api(serv);
	etux_in6sk_assert_api(!(flags & ~(AI_NUMERICHOST |
	                                  AI_NUMERICSERV |
	                                  AI_PASSIVE |
	                                  AI_V4MAPPED |
	                                  AI_ADDRCONFIG |
	                                  AI_IDN)));

	int err;

	err = etux_in6sk_make_host(addr, host, flags & (AI_NUMERICHOST |
	                                                AI_PASSIVE |
	                                                AI_V4MAPPED |
	                                                AI_ADDRCONFIG |
	                                                AI_IDN));
	if (err)
		return err;

	return etux_in6sk_make_serv(addr, serv, proto, flags & AI_NUMERICSERV);
}

ssize_t
etux_in6sk_addr_name(
	const struct sockaddr_in6 * __restrict addr,
	const char * __restrict                proto,
	char                                   name[__restrict_arr ETUX_NETDB_NAME_MAX],
	int                                    flags)
{
	etux_in6sk_assert_api(addr);
	etux_in6sk_assert_api(addr->sin6_family == AF_INET6);
	etux_in6sk_assert_api(name);
	etux_in6sk_assert_api(!(flags & ~(NI_NAMEREQD |
	                                  NI_NOFQDN |
	                                  NI_NUMERICHOST |
	                                  NI_NUMERICSERV)));
	etux_in6sk_assert_api((flags & (NI_NAMEREQD | NI_NUMERICHOST)) !=
	                      (NI_NAMEREQD | NI_NUMERICHOST));

	ssize_t ret;

	ret = etux_in6sk_host_name(addr, name, flags & ~NI_NUMERICSERV);
	etux_in6sk_assert_intern(ret);
	etux_in6sk_assert_intern(ret < NI_MAXHOST);
	if (ret < 0)
		return ret;

	if (addr->sin6_port) {
		ssize_t len = ret + 1;

		name[ret] = ':';

#warning fixme
		ret = etux_in6sk_serv_name(addr,
		                           proto,
		                           &name[len],
		                           flags & NI_NUMERICSERV);
		etux_in6sk_assert_intern(ret);
		etux_in6sk_assert_intern(ret < NI_MAXSERV);
		if (ret < 0)
			return ret;

		ret += len;
	}

	return ret;
}

#endif /* defined(CONFIG_ETUX_NETDB) */

int
etux_in6sk_connect(int fd, const struct sockaddr_in6 * __restrict peer)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api(peer);
	etux_in6sk_assert_api(peer->sin6_family == AF_INET6);

	return etux_syssk_connect(fd,
	                          (const struct sockaddr *)peer,
	                          sizeof(*peer));
}

int
etux_in6sk_accept(int fd, struct sockaddr_in6 * __restrict peer, int flags)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	socklen_t sz = sizeof(*peer);
	int       nevv;

	nevv = etux_syssk_accept(fd, (struct sockaddr *)peer, &sz, flags);
	if (nevv >= 0)
		etux_in6sk_assert_api(sz == sizeof(*peer));

	return nevv;
}

int
etux_in6sk_bind(int fd, const struct sockaddr_in6 * __restrict local)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api(local);
	etux_in6sk_assert_api(local->sin6_family == AF_INET6);

	return etux_syssk_bind(fd,
	                       (const struct sockaddr *)local,
	                       sizeof(*local));
}

#if defined(CONFIG_ETUX_NETIF)

int
etux_in6sk_bind_netif(int fd, const char * __restrict iface, size_t len)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api(iface);
	etux_in6sk_assert_api(len);
	etux_in6sk_assert_api(len < IFNAMSIZ);
	etux_in6sk_assert_api(strnlen(iface, IFNAMSIZ) == len);

	return etux_syssk_bind_netif(fd, iface, len);
}

#endif /* defined(CONFIG_ETUX_NETIF) */

int
etux_in6sk_open(int type, int proto, int flags)
{
	etux_in6sk_assert_api((type == SOCK_DGRAM) ||
	                      (type == SOCK_STREAM) ||
	                      (type == SOCK_RAW));
	etux_in6sk_assert_api(proto >= IPPROTO_MAX);
	etux_in6sk_assert_api(proto < IPPROTO_MAX);
	etux_in6sk_assert_api((type == SOCK_RAW) ^ !!proto);
	etux_in6sk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	return etux_syssk_open(AF_INET6, type, proto, flags);
}

int
etux_in6sk_shutdown(int fd, int how)
{
	etux_in6sk_assert_api(fd >= 0);
	etux_in6sk_assert_api((how == SHUT_RD) ||
	                      (how == SHUT_WR) ||
	                      (how == SHUT_RDWR));

	return etux_syssk_shutdown(fd, how);
}

int
etux_in6sk_close(int fd)
{
	etux_in6sk_assert_api(fd >= 0);

	return etux_syssk_close(fd);
}
