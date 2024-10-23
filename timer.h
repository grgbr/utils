/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _UTILS_TIMER_INTERN_H
#define _UTILS_TIMER_INTERN_H

#include "utils/timer.h"

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define utimer_assert_intern(_expr) \
	stroll_assert("utils:utimer", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define utimer_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

extern int
utimer_tick_cmp(const struct stroll_dlist_node * __restrict first,
                const struct stroll_dlist_node * __restrict second,
                void *                                      data __unused)
	__utils_nonull(1, 2) __utils_pure __utils_nothrow __export_intern;

#endif /* _UTILS_TIMER_INTERN_H */
