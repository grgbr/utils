/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2026 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Missing syscall wrapping support
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      09 Feb 2026
 * @copyright Copyright (C) 2017-2026 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _ETUX_SYSCALL_H
#define _ETUX_SYSCALL_H

#include <utils/cdefs.h>
#include <unistd.h>
#include <sys/syscall.h>

/* Public definition of gettid(2) comes with Glibc versions >= 2.30. */
#if defined(_GNU_SOURCE) && !__GLIBC_PREREQ(2, 30)

#include <sys/types.h>

static inline
pid_t
gettid(void)
{
	return syscall(SYS_gettid);
}

#endif /* defined(_GNU_SOURCE) && !__GLIBC_PREREQ(2, 30) */

#endif /* _ETUX_SYSCALL_H */
