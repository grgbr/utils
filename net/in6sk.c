/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/in6sk.h"

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
	addr->sin6_flowinfo = htonl(flow),
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

	if (addr->sin6_port) {
		char *  host;
		ssize_t len;

		host = malloc(NI_MAXHOST);
		if (!host)
			return -ENOMEM;

		ret = etux_in6sk_host_name(addr,
		                           host,
		                           flags & ~NI_NUMERICSERV);
		etux_in6sk_assert_intern(ret);
		etux_in6sk_assert_intern(ret < NI_MAXHOST);
		if (ret < 0)
			goto free;

		len = ret;
		if (memchr(host, ':', (size_t)ret)) {
			name[0] = '[';
			memcpy(&name[1], host, (size_t)len++);
			name[len++] = ']';
		}
		else
			memcpy(name, host, (size_t)len);

		name[len++] = ':';
		ret = etux_in6sk_serv_name(addr,
		                           proto,
		                           &name[len],
		                           flags & NI_NUMERICSERV);
		etux_in6sk_assert_intern(ret);
		etux_in6sk_assert_intern(ret < NI_MAXSERV);
		if (ret < 0)
			goto free;

		ret += len;
		etux_in6sk_assert_intern((size_t)ret < ETUX_NETDB_NAME_MAX);

free:
		free(host);
	}
	else {
		ret = etux_in6sk_host_name(addr, name, flags & ~NI_NUMERICSERV);
		etux_in6sk_assert_intern(ret);
		etux_in6sk_assert_intern(ret < NI_MAXHOST);
	}

	return ret;
}

#endif /* defined(CONFIG_ETUX_NETDB) */
