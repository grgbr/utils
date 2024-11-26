/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utest.h"

#if defined(CONFIG_UTILS_TIME)
extern CUTE_SUITE_DECL(utilsut_time_suite);
#endif

CUTE_GROUP(utilsut_group) = {
#if defined(CONFIG_UTILS_TIME)
	CUTE_REF(utilsut_time_suite),
#endif
};

CUTE_SUITE(utilsut_suite, utilsut_group);

CUTE_MAIN(utilsut_suite, "Utils", UTILS_VERSION_STRING)
