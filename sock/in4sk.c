/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/in4sk.h"
#include "syssk.h"

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

int
etux_in4sk_parse_addr(struct sockaddr_in * __restrict addr,
                      const char * __restrict         string)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(string);

	int err;

	err = etux_syssk_parse_addr(&addr->sin_addr, AF_INET, string);
	if (err)
		return err;

	addr->sin_family = AF_INET;

	return 0;
}

ssize_t
etux_in4sk_validate_host(const char * __restrict string)
{
	etux_in4sk_assert_api(string);

	struct in_addr addr;

	if (!etux_syssk_parse_addr(&addr, AF_INET, string))
		return (ssize_t)strlen(string);

	return etux_syssk_validate_host_name(string);
}

int
etux_in4sk_resolv_host(struct sockaddr_in * __restrict addr,
                       const char * __restrict         string,
                       int                             flags)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(string);
	etux_in4sk_assert_api(!(flags & ~(AI_NUMERICHOST | AI_NUMERICSERV |
	                                  AI_PASSIVE |
	                                  AI_ADDRCONFIG |
	                                  AI_IDN)));


	const struct addrinfo hints = {
		.ai_flags     = flags | AI_ADDRCONFIG,
		.ai_family    = AF_INET,
		.ai_socktype  = 0,
		.ai_protocol  = 0,
		.ai_addrlen   = 0,
		.ai_addr      = NULL,
		.ai_canonname = NULL,
		.ai_next      = NULL
	};
	struct addrinfo *     infos;
	int                   err;

	err = etux_syssk_getaddrinfo(string, NULL, &hints, &infos);
	if (err)
		return err;

	etux_in4sk_assert_api(infos->ai_addrlen == sizeof(*addr));

	addr->sin_family = AF_INET;
	memcpy(&addr->sin_addr,
	       &((const struct sockaddr_in *)infos->ai_addr)->sin_addr,
	       sizeof(*addr));

	etux_syssk_freeaddrinfo(infos);

	return 0;
}

int
etux_in4sk_parse_port(struct sockaddr_in * __restrict addr,
                      const char * __restrict         string)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(string);

	return etux_syssk_parse_port(&addr->sin_port, string);
}

int
etux_in4sk_resolv_serv(struct sockaddr_in * __restrict addr,
                       const char * __restrict         string)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(string);

	return etux_syssk_resolv_serv(&addr->sin_port, string);
}

void
etux_in4sk_make_addr(struct sockaddr_in * __restrict addr,
                     in_addr_t                       net,
                     in_port_t                       xport)
{
	etux_in4sk_assert_api(addr);

	addr->sin_family = AF_INET;
	addr->sin_port = htons(xport);
	addr->sin_addr.s_addr = htonl(net);
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
etux_in4sk_open(int type, int proto, int flags)
{
	etux_in4sk_assert_api((type == SOCK_DGRAM) ||
	                      (type == SOCK_STREAM) ||
	                      (type == SOCK_RAW));
	etux_in4sk_assert_api((type == SOCK_RAW) || proto);
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
