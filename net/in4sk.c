/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/in4sk.h"

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

#if defined(CONFIG_ETUX_NETDB)

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
	etux_in4sk_assert_api(host);
	etux_in4sk_assert_api(serv);
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

ssize_t
etux_in4sk_addr_name(
	const struct sockaddr_in * __restrict addr,
	const char * __restrict               proto,
	char                                  name[__restrict_arr ETUX_NETDB_NAME_MAX],
	int                                   flags)
{
	etux_in4sk_assert_api(addr);
	etux_in4sk_assert_api(addr->sin_family == AF_INET);
	etux_in4sk_assert_api(name);
	etux_in4sk_assert_api(!(flags & ~(NI_NAMEREQD |
	                                  NI_NOFQDN |
	                                  NI_NUMERICHOST |
	                                  NI_NUMERICSERV)));
	etux_in4sk_assert_api((flags & (NI_NAMEREQD | NI_NUMERICHOST)) !=
	                      (NI_NAMEREQD | NI_NUMERICHOST));

	ssize_t ret;

	ret = etux_in4sk_host_name(addr, name, flags & ~NI_NUMERICSERV);
	etux_in4sk_assert_intern(ret);
	etux_in4sk_assert_intern(ret < NI_MAXHOST);
	if (ret < 0)
		return ret;

	if (addr->sin_port) {
		ssize_t len = ret + 1;

		name[ret] = ':';

		ret = etux_in4sk_serv_name(addr,
		                           proto,
		                           &name[len],
		                           flags & NI_NUMERICSERV);
		etux_in4sk_assert_intern(ret);
		etux_in4sk_assert_intern(ret < NI_MAXSERV);
		if (ret < 0)
			return ret;

		ret += len;
	}

	return ret;
}

#endif /* defined(CONFIG_ETUX_NETDB) */
