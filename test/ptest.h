/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of eTux.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _ETUX_PTEST_H
#define _ETUX_PTEST_H

#include "utils/cdefs.h"
#include <stdio.h>

#define etuxpt_err(_format, ...) \
	fprintf(stderr, \
	        "%s: " _format, \
	        program_invocation_short_name, \
	        ## __VA_ARGS__)

extern int
etuxpt_parse_sched_prio(const char * __restrict arg,
                        int * __restrict        priority);

extern int etuxpt_setup_sched_prio(int priority);

#endif /* _ETUX_PTEST_H */
