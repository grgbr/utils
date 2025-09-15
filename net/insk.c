/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/insk.h"
#if defined(CONFIG_ETUX_IN4SK)
#include "utils/in4sk.h"
#endif /* defined(CONFIG_ETUX_IN4SK) */
#if defined(CONFIG_ETUX_IN6SK)
#include "utils/in6sk.h"
#endif /* defined(CONFIG_ETUX_IN6SK) */

#if defined(CONFIG_ETUX_NETDB)

#include <utils/netdb.h>

int
etux_insk_make_host(struct sockaddr * __restrict addr,
                    socklen_t                    size,
                    const char * __restrict      string,
                    int                          flags)
{
	etux_insk_assert_api(addr);
	etux_insk_assert_api((size_t)size >= sizeof(*addr));
	etux_insk_assert_api(string);
	etux_insk_assert_api(!(flags & ~(AI_NUMERICHOST |
	                                 AI_PASSIVE |
	                                 AI_V4MAPPED |
	                                 AI_ADDRCONFIG |
	                                 AI_IDN)));

	int err;

	err = etux_netdb_make_host(AF_UNSPEC,
	                           string,
	                           (struct sockaddr *)addr,
	                           size,
	                           flags);
	if (!err) {
		etux_insk_assert_intern(addr->sa_family == AF_INET ||
		                        addr->sa_family == AF_INET6);
		return 0;
	}

	return err;
}

ssize_t
etux_insk_host_name(
	const struct sockaddr * __restrict addr,
	char                               host[__restrict_arr NI_MAXHOST],
	int                                flags)
{
	etux_insk_assert_api(addr);
	etux_insk_assert_intern(addr->sa_family == AF_INET ||
	                        addr->sa_family == AF_INET6);
	etux_insk_assert_api(host);
	etux_insk_assert_api(!(flags & ~(NI_NAMEREQD |
	                                 NI_NOFQDN |
	                                 NI_NUMERICHOST)));
	etux_insk_assert_api((flags & (NI_NAMEREQD | NI_NUMERICHOST)) !=
	                     (NI_NAMEREQD | NI_NUMERICHOST));

	switch (addr->sa_family) {
	case AF_INET:
		return etux_in4sk_host_name((const struct sockaddr_in *)addr,
		                            host,
		                            flags);
	case AF_INET6:
		return etux_in6sk_host_name((const struct sockaddr_in6 *)addr,
		                            host,
		                            flags);
	default:
		etux_insk_assert_api(0);
	}

	unreachable();
}

int
etux_insk_make_serv(struct sockaddr * __restrict addr,
                    const char * __restrict      serv,
                    const char * __restrict      proto,
                    int                          flags)
{
	etux_insk_assert_api(addr);
	etux_insk_assert_api(addr->sa_family == AF_INET ||
	                     addr->sa_family == AF_INET6);
	etux_insk_assert_api(serv);
	etux_insk_assert_api(!(flags & ~AI_NUMERICSERV));

	switch (addr->sa_family) {
	case AF_INET:
		return etux_in4sk_make_serv((struct sockaddr_in *)addr,
		                            serv,
		                            proto,
		                            flags);
	case AF_INET6:
		return etux_in6sk_make_serv((struct sockaddr_in6 *)addr,
		                            serv,
		                            proto,
		                            flags);
	default:
		etux_insk_assert_api(0);
	}

	unreachable();
}

ssize_t
etux_insk_serv_name(
	const struct sockaddr * __restrict addr,
	const char * __restrict            proto,
	char                               serv[__restrict_arr NI_MAXSERV],
	int                                flags)
{
	etux_insk_assert_api(addr);
	etux_insk_assert_api(addr->sa_family == AF_INET ||
	                     addr->sa_family == AF_INET6);
	etux_insk_assert_api(serv);
	etux_insk_assert_api(!(flags & ~NI_NUMERICSERV));

	switch (addr->sa_family) {
	case AF_INET:
		return etux_in4sk_serv_name(
			(const struct sockaddr_in *)addr,
			proto,
			serv,
			flags);
	case AF_INET6:
		return etux_in6sk_serv_name(
			(const struct sockaddr_in6 *)addr,
			proto,
			serv,
			flags);
	default:
		etux_insk_assert_api(0);
	}

	unreachable();
}

#endif /* defined(CONFIG_ETUX_NETDB) */
