/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/dir.h"

int
udir_nointr_open(const char * __restrict path, int flags)
{
	udir_assert_api(upath_validate_path_name(path) > 0);

	int fd;

	do {
		fd = udir_open(path, flags);
	} while (fd == -EINTR);

	return fd;
}
