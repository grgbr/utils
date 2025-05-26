/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _ETUX_SYSSK_H
#define _ETUX_SYSSK_H

#include "utils/fd.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_syssk_assert_api(_expr) \
	stroll_assert("etux:syssk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_syssk_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_syssk_assert_intern(_expr) \
	stroll_assert("etux:syssk", _expr)

#else  /* !defined(CONFIG_ETUX_ASSERT_INTERN) */

#define etux_syssk_assert_intern(_expr)

#endif /* defined(CONFIG_ETUX_ASSERT_INTERN) */

extern int
etux_syssk_getnameinfo(
	const struct sockaddr * __restrict addr,
	socklen_t                          size,
	char                               host[__restrict_arr NI_MAXHOST],
	char                               serv[__restrict_arr NI_MAXSERV],
	int                                flags)
	__utils_nonull(1) __warn_result;

static inline __utils_nonull(1) __utils_nothrow
void
etux_syssk_freeaddrinfo(struct addrinfo * __restrict infos)
{
	etux_syssk_assert_intern(infos);

	freeaddrinfo(infos);
}

static inline __utils_nonull(2) __warn_result
int
etux_syssk_connect(int                                fd,
                   const struct sockaddr * __restrict peer,
                   socklen_t                          size)
{
	etux_syssk_assert_api(fd >= 0);
	etux_syssk_assert_api(peer);
	etux_syssk_assert_api(peer->sa_family != AF_UNSPEC);
	etux_syssk_assert_api(size);

	if (!connect(fd, peer, size))
		return 0;

	etux_syssk_assert_api(errno != EAFNOSUPPORT);
	etux_syssk_assert_api(errno != EBADF);
	etux_syssk_assert_api(errno != EFAULT);
	etux_syssk_assert_api(errno != EISCONN);
	etux_syssk_assert_api(errno != ENOTSOCK);

	return -errno;
}

static inline __warn_result
int
etux_syssk_accept(int                          fd,
                  struct sockaddr * __restrict peer,
                  socklen_t * __restrict       size,
                  int                          flags)
{
	etux_syssk_assert_api(fd >= 0);
	etux_syssk_assert_api(size);
	etux_syssk_assert_api(*size);
	etux_syssk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	int nevv;

	nevv = accept4(fd, peer, size, flags);
	if (nevv >= 0)
		return nevv;

	etux_syssk_assert_api(errno != EBADF);
	etux_syssk_assert_api(errno != EFAULT);
	etux_syssk_assert_api(errno != EINVAL);
	etux_syssk_assert_api(errno != ENOTSOCK);
	etux_syssk_assert_api(errno != EOPNOTSUPP);

	return -errno;
}

static inline __utils_nonull(2) __utils_nothrow __warn_result
int
etux_syssk_bind(int                                fd,
                const struct sockaddr * __restrict local,
                socklen_t                          size)
{
	etux_syssk_assert_api(fd >= 0);
	etux_syssk_assert_api(local);
	etux_syssk_assert_api(local->sa_family != AF_UNSPEC);
	etux_syssk_assert_api(size);

	if (!bind(fd, local, size))
		return 0;

	etux_syssk_assert_api(errno != EBADF);
	etux_syssk_assert_api(errno != EINVAL);
	etux_syssk_assert_api(errno != ENOTSOCK);

	return -errno;
}

static inline __utils_nonull(2) __utils_nothrow __warn_result
int
etux_syssk_bind_netif(int fd, const char * __restrict iface, size_t len)
{
	etux_syssk_assert_api(fd >= 0);
	etux_syssk_assert_api(iface);
	etux_syssk_assert_api(len);
	etux_syssk_assert_api(len < IFNAMSIZ);
	etux_syssk_assert_api(strnlen(iface, IFNAMSIZ) == len);

	if (!setsockopt(fd,
	                SOL_SOCKET,
	                SO_BINDTODEVICE,
	                iface,
	                (socklen_t)len + 1))
		return 0;

	etux_syssk_assert_api(errno != EBADF);
	etux_syssk_assert_api(errno != EFAULT);
	etux_syssk_assert_api(errno != EINVAL);
	etux_syssk_assert_api(errno != ENOPROTOOPT);
	etux_syssk_assert_api(errno != ENOTSOCK);

	return -errno;
}

static inline __utils_nothrow __warn_result
int
etux_syssk_open(int domain, int type, int proto, int flags)
{
	etux_syssk_assert_api(domain > AF_UNSPEC);
	etux_syssk_assert_api(type > 0);
	etux_syssk_assert_api(proto >= 0);
	etux_syssk_assert_api(proto < IPPROTO_MAX);
	etux_syssk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	int fd;

	fd = socket(domain, type | flags, proto);
	if (fd < 0) {
		etux_syssk_assert_api(errno != EINVAL);
		etux_syssk_assert_api(errno != EPROTONOSUPPORT);

		return -errno;
	}

	return fd;
}

static inline __utils_nothrow __warn_result
int
etux_syssk_shutdown(int fd, int how)
{
	etux_syssk_assert_api(fd >= 0);
	etux_syssk_assert_api((how == SHUT_RD) ||
	                     (how == SHUT_WR) ||
	                     (how == SHUT_RDWR));

	if (!shutdown(fd, how))
		return 0;

	etux_syssk_assert_api(errno != EBADF);
	etux_syssk_assert_api(errno != EINVAL);
	etux_syssk_assert_api(errno != ENOTSOCK);

	return -errno;
}

static inline
int
etux_syssk_close(int fd)
{
	etux_syssk_assert_api(fd >= 0);

	int err;

	err = ufd_close(fd);

	etux_syssk_assert_api(err != -ENOSPC);
	etux_syssk_assert_api(err != -EDQUOT);

	return err;
}

#endif /* _UTILS_SYSSK_H */
