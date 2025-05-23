/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/insk.h"
#include <stdlib.h>

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define insk_assert_intern(_expr) \
	stroll_assert("utils:insk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define insk_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

/******************************************************************************
 * low-level UNIX socket wrappers
 ******************************************************************************/

int
insk_open(int type, int proto)
{
	/* See ip(7) for more informations about supported flags... */
	insk_assert_api((type == SOCK_DGRAM) ||
	                (type == SOCK_STREAM) ||
	                (type == SOCK_RAW));
	insk_assert_api((type != SOCK_DGRAM) ||
	                ((proto == IPPROTO_UDP) || (proto == IPPROTO_UDPLITE)));
	insk_assert_api((type != SOCK_STREAM) ||
	                ((proto == IPPROTO_TCP) || (proto == IPPROTO_SCTP)));

	int fd;

	fd = socket(AF_INET, type, proto);
	if (fd < 0) {
		insk_assert_api(errno != EAFNOSUPPORT);
		insk_assert_api(errno != EINVAL);
		insk_assert_api(errno != EPROTONOSUPPORT);

		return -errno;
	}

	return fd;
}

int
insk_bind(int fd, const struct sockaddr_in * __restrict addr)
{
	insk_assert_api(fd >= 0);
	insk_assert_api(addr);
	insk_assert_api(addr->sun_family == AF_INET);

	if (!bind(fd, (const struct sockaddr *)addr, sizeof(*addr)))
		return 0;

	insk_assert_api(errno != EBADF);
	insk_assert_api(errno != EINVAL);
	insk_assert_api(errno != ENOTSOCK);
	insk_assert_api(errno != EADDRNOTAVAIL);
	insk_assert_api(errno != EFAULT);
	insk_assert_api(errno != ENAMETOOLONG);

	return -errno;
}

int
insk_shutdown(int fd, int how)
{
	insk_assert_api(fd >= 0);
	insk_assert_api((how == SHUT_RD) ||
	                (how == SHUT_WR) ||
	                (how == SHUT_RDWR));

	if (!shutdown(fd, how))
		return 0;

	insk_assert_api(errno != EBADF);
	insk_assert_api(errno != EINVAL);
	insk_assert_api(errno != ENOTSOCK);

	return -errno;
}

int
insk_close(int fd)
{
	insk_assert_api(fd >= 0);

	int err;

	err = ufd_close(fd);

	insk_assert_api(err != -ENOSPC);
	insk_assert_api(err != -EDQUOT);

	return err;
}
