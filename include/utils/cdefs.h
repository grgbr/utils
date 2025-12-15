/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Common preprocessing definitions
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2017
 * @copyright Copyright (C) 2017-2023 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_CDEFS_H
#define _UTILS_CDEFS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <utils/config.h>
#include <stroll/cdefs.h>

#if defined(CONFIG_UTILS_ASSERT_API) || defined(CONFIG_UTILS_ASSERT_INTERN)

#define __utils_nonull(_arg_index, ...)
#define __utils_const
#define __utils_pure
#define __utils_nothrow

#else  /* !(defined(CONFIG_UTILS_ASSERT_API) || \
            defined(CONFIG_UTILS_ASSERT_INTERN)) */

#define __utils_nonull(_arg_index, ...)  __nonull(_arg_index, ## __VA_ARGS__)
#define __utils_const                    __const
#define __utils_pure                     __pure
#define __utils_nothrow                  __nothrow

#endif /* defined(CONFIG_UTILS_ASSERT_API) || \
          defined(CONFIG_UTILS_ASSERT_INTERN) */

#endif /* _UTILS_CDEFS_H */
