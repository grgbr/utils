/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _UTILS_UTEST_H
#define _UTILS_UTEST_H

#include "utils/config.h"
#include <cute/cute.h>
#include <cute/check.h>
#include <cute/expect.h>
#include <time.h>

#define UTILSUT_NOASSERT_TEST(_test) \
	CUTE_TEST(_test) { cute_skip("assertion not supported"); }

extern void
utilsut_expect_monotonic_now(const struct timespec * expected);

#endif /* _UTILS_UTEST_H */
