/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/netif.h"
#include <string.h>

int
etux_netif_validate(const char * __restrict string)
{
	etux_netif_assert_api(string);

	size_t len;

	len = strnlen(string, IFNAMSIZ);
	if (!len)
		return -ENODATA;
	else if (len >= IFNAMSIZ)
		return -ENAMETOOLONG;

	return 0;
}
