/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "syssk.h"
#include "utils/string.h"

int
etux_syssk_getnameinfo(
	const struct sockaddr * __restrict addr,
	socklen_t                          size,
	char                               host[__restrict_arr NI_MAXHOST],
	char                               serv[__restrict_arr NI_MAXSERV],
	int                                flags)
{
	etux_syssk_assert_api(addr);
	etux_syssk_assert_api(host || serv);
	etux_syssk_assert_api(!(flags & ~(NI_NAMEREQD |
	                                  NI_NOFQDN |
	                                  NI_NUMERICHOST |
	                                  NI_NUMERICSERV)));
	etux_syssk_assert_api(!((flags & NI_NAMEREQD) &&
	                        (flags & NI_NUMERICHOST)));

	int err;

	err = getnameinfo(addr,
	                  size,
	                  host,
	                  NI_MAXHOST,
	                  serv,
	                  NI_MAXSERV,
	                  flags);
	if (!err)
		return 0;

	etux_syssk_assert_api(err != EAI_BADFLAGS);
	etux_syssk_assert_api(err != EAI_FAMILY);
	etux_syssk_assert_api(err != EAI_OVERFLOW);

	switch (err) {
	case EAI_AGAIN:
		return -EAGAIN;
	case EAI_FAIL:
		return -ENOTRECOVERABLE;
	case EAI_MEMORY:
		return -ENOMEM;
	case EAI_NONAME:
		etux_syssk_assert_api(flags & NI_NAMEREQD);
		return -ENOENT;
	case EAI_SYSTEM:
		break;
	default:
		etux_syssk_assert_intern(0);
	}

	return -errno;
}
