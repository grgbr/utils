/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2026 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Spawn interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      09 Feb 2026
 * @copyright Copyright (C) 2017-2026 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _ETUX_SPAWN_H
#define _ETUX_SPAWN_H

#include <utils/cdefs.h>
#include <stdio.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define spawn_assert_api(_expr) \
	stroll_assert("utils:spawn", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define spawn_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

extern __utils_nonull(1)
FILE *
etux_spawn_popen(const char *path,
		 char *const argv[],
		 char *const envp[],
		 int flags);

extern __utils_nonull(1)
int
etux_spawn_pclose(FILE *stream);

#endif /* _ETUX_SPAWN_H */

