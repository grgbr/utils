/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/mqueue.h"
#include "utils/path.h"

ssize_t
umq_validate_name(const char * __restrict name)
{
	umq_assert_api(name);

	if (name[0] == '/') {
		ssize_t len;

		len = upath_validate_file_name(&name[1]);
		if (len < 0)
			return len;

		if (memchr(&name[1], '/', (size_t)len))
			return -EISDIR;

		return len + 1;
	}
	else if (name[0] == '\0')
		return -ENODATA;
	else
		return -EINVAL;
}
