/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/netdb.h"
#include "utils/string.h"
#include <stdio.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_netdb_assert_api(_expr) \
	stroll_assert("etux:netdb", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_netdb_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_netdb_assert_intern(_expr) \
	stroll_assert("etux:netdb", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_netdb_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

#define ETUX_NETDB_PROTO_MAX (1024U)

static __utils_nonull(1) __utils_pure __utils_nothrow
int
etux_netdb_validate_string(const char * __restrict string, size_t size)
{
	etux_netdb_assert_intern(string);
	etux_netdb_assert_intern(size);

	size_t len;

	len = strnlen(string, size);
	etux_netdb_assert_intern(len <= size);
	if (!len)
		return -ENODATA;
	else if (len == size)
		return -ENAMETOOLONG;

	return 0;
}

int
etux_netdb_validate_host(const char * __restrict string)
{
	etux_netdb_assert_api(string);

	return etux_netdb_validate_string(string, NI_MAXHOST);
}

int
etux_netdb_make_host(int                          family,
                     const char * __restrict      host,
                     struct sockaddr * __restrict addr,
                     size_t                       size __unused,
                     int                          flags)
{
	etux_netdb_assert_api(family >= AF_UNSPEC);
	etux_netdb_assert_api(host);
	etux_netdb_assert_api(addr);
	etux_netdb_assert_api(size >= sizeof(*addr));
	etux_netdb_assert_api(size < UINT_MAX);
	etux_netdb_assert_api(!(flags & ~(AI_NUMERICHOST |
	                                  AI_PASSIVE |
	                                  AI_V4MAPPED |
	                                  AI_ADDRCONFIG |
	                                  AI_IDN)));

	const struct addrinfo hints = {
		.ai_flags     = flags | AI_ADDRCONFIG,
		.ai_family    = family,
		.ai_socktype  = 0,
		.ai_protocol  = 0,
		.ai_addrlen   = 0,
		.ai_addr      = NULL,
		.ai_canonname = NULL,
		.ai_next      = NULL
	};
	struct addrinfo *     infos;
	int                   err;

	err = getaddrinfo(host, NULL, &hints, &infos);
	if (err) {
		etux_netdb_assert_api(err != EAI_BADFLAGS);
		etux_netdb_assert_api(err != EAI_SOCKTYPE);
		etux_netdb_assert_api(err != EAI_SERVICE);

		switch (err) {
		case EAI_ADDRFAMILY:
			return -EADDRNOTAVAIL;
		case EAI_AGAIN:
			return -EAGAIN;
		case EAI_FAIL:
			return -ENOTRECOVERABLE;
		case EAI_FAMILY:
			return -EAFNOSUPPORT;
		case EAI_MEMORY:
			return -ENOMEM;
		case EAI_NODATA:
			return -ENODATA;
		case EAI_NONAME:
			return -ENOENT;
		case EAI_SYSTEM:
			break;
		default:
			etux_netdb_assert_intern(0);
		}

		return -errno;
	}

	etux_netdb_assert_intern(infos);
	etux_netdb_assert_intern(infos->ai_family);
	etux_netdb_assert_intern(!hints.ai_family ||
	                         (infos->ai_family == hints.ai_family));
	etux_netdb_assert_intern(infos->ai_socktype);
	etux_netdb_assert_intern(infos->ai_addr);
	etux_netdb_assert_intern(infos->ai_addrlen <= size);

	memcpy(addr, infos->ai_addr, infos->ai_addrlen);

	freeaddrinfo(infos);

	return 0;
}

ssize_t
etux_netdb_host_name(
	const struct sockaddr * __restrict addr,
	size_t                             size,
	char                               host[__restrict_arr NI_MAXHOST],
	int                                flags)
{
	etux_netdb_assert_api(addr);
	etux_netdb_assert_api(addr->sa_family != AF_UNSPEC);
	etux_netdb_assert_api(size >= sizeof(*addr));
	etux_netdb_assert_api(size < UINT_MAX);
	etux_netdb_assert_api(host);
	etux_netdb_assert_api(!(flags & ~(NI_NAMEREQD |
	                                  NI_NOFQDN |
	                                  NI_NUMERICHOST)));
	etux_netdb_assert_api((flags & (NI_NAMEREQD | NI_NUMERICHOST)) !=
	                      (NI_NAMEREQD | NI_NUMERICHOST));

	int err;

	err = getnameinfo(addr,
	                  (socklen_t)size,
	                  host,
	                  NI_MAXHOST,
	                  NULL,
	                  0,
	                  flags);
	if (!err) {
		size_t len;

		len = strlen(host);
		etux_netdb_assert_intern(len);
		etux_netdb_assert_intern(len < NI_MAXHOST);

		return (ssize_t)len;
	}

	etux_netdb_assert_api(err != EAI_BADFLAGS);
	etux_netdb_assert_api(err != EAI_FAMILY);
	etux_netdb_assert_api(err != EAI_OVERFLOW);

	switch (err) {
	case EAI_AGAIN:
		return -EAGAIN;
	case EAI_FAIL:
		return -ENOTRECOVERABLE;
	case EAI_MEMORY:
		return -ENOMEM;
	case EAI_NONAME:
		etux_netdb_assert_api(flags & NI_NAMEREQD);
		return -ENOENT;
	case EAI_SYSTEM:
		break;
	default:
		etux_netdb_assert_intern(0);
	}

	return -errno;
}

int
etux_netdb_validate_proto(const char * __restrict string)
{
	etux_netdb_assert_api(string);

	return etux_netdb_validate_string(string, ETUX_NETDB_PROTO_MAX);
}

int
etux_netdb_parse_proto(const char * __restrict string, int * __restrict proto)
{
	etux_netdb_assert_api(string);
	etux_netdb_assert_api(proto);

	if (strcmp(string, "unspec")) {
		const struct protoent * ent;

		ent = getprotobyname(string);
		if (ent) {
			if ((ent->p_proto < 0) || (ent->p_proto >= IPPROTO_MAX))
				return -EPROTONOSUPPORT;

			*proto = ent->p_proto;

			return 0;
		}

		return -ENOENT;
	}

	*proto = 0;

	return 0;
}

int
etux_netdb_validate_serv(const char * __restrict string)
{
	etux_netdb_assert_api(string);

	return etux_netdb_validate_string(string, NI_MAXSERV);
}

/*
 * Do not use getaddrinfo() as service port resolution is not properly handled
 * (unpredictable port number returned when passing out-of-range numeric service
 * string).
 *
 * Proto is optional and may be passed as NULL (see getservbyname(3)).
 */
int
etux_netdb_parse_serv(const char * __restrict serv,
                      const char * __restrict proto,
                      in_port_t * __restrict  port,
                      int                     flags)
{
	etux_netdb_assert_api(serv);
	etux_netdb_assert_api(port);
	etux_netdb_assert_api(!(flags & ~AI_NUMERICSERV));

	unsigned long prt;
	int           err;

	err = ustr_parse_base_ulong(serv, &prt, 10);
	if (err) {
		if (!(flags & AI_NUMERICSERV)) {
			const struct servent * ent;

			ent = getservbyname(serv, proto);
			if (!ent)
				return -ENOENT;

			/* Port number is given in network byte order. */
			prt = ntohl((unsigned int)ent->s_port);
		}
		else
			return err;
	}

	if (prt > (unsigned long)USHRT_MAX)
		return -ERANGE;

	*port = htons((unsigned short)prt);

	return 0;
}

ssize_t
etux_netdb_serv_name(in_port_t               port,
                     const char * __restrict proto,
                     char                    serv[__restrict_arr NI_MAXSERV],
                     int                     flags)
{
	etux_netdb_assert_api(serv);
	etux_netdb_assert_api(!(flags & ~NI_NUMERICSERV));

	size_t len;

	if (!(flags & NI_NUMERICSERV)) {
		const struct servent * ent;

		ent = getservbyport((int)port, proto);
		if (!ent)
			return -ENOENT;

		len = strnlen(ent->s_name, NI_MAXSERV);
		etux_netdb_assert_intern(len <= NI_MAXSERV);
		if (!len)
			return -ENODATA;
		else if (len == NI_MAXSERV)
			return -ENAMETOOLONG;

		memcpy(serv, ent->s_name, len);
		serv[len] = '\0';
	}
	else {
		len = (size_t)sprintf(serv, "%hu", ntohs(port));
		etux_netdb_assert_intern(len <= 5);
	}

	return (ssize_t)len;
}
