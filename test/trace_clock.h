/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _ETUX_TEST_TRACE_CLOCK_H
#define _ETUX_TEST_TRACE_CLOCK_H

#include <utils/config.h>
#include <stroll/cdefs.h>

#if defined(CONFIG_ETUX_TRACE)

extern int
etuxpt_timer_setup_trace_clock(void) __export_intern;

#else /* !defined(CONFIG_ETUX_TRACE) */

static inline
int
etuxpt_timer_setup_trace_clock(void)
{
	return 0;
}

#endif /* defined(CONFIG_ETUX_TRACE) */

#endif /* _ETUX_TEST_TRACE_CLOCK_H */
