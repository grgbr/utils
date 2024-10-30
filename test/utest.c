/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utest.h"
#include <stroll/assert.h>
#include <stdio.h>
#include <stdlib.h>

static char utilsut_assert_msg[LINE_MAX];

/*
 * Override libstroll's stroll_assert_fail() and use cute_mock_assert() to
 * validate assertions.
 */
void
stroll_assert_fail(const char * __restrict prefix,
                   const char * __restrict expr,
                   const char * __restrict file,
                   unsigned int            line,
                   const char * __restrict func)
{
	int    ret;
	size_t sz = sizeof(utilsut_assert_msg) - 1;

	/*
	 * cute_mock_assert() does not really "return". It uses a nonlocal goto
	 * logic to restore program / stack state that existed before the code
	 * under test called us. This is the way CUTe allows checking for
	 * assertions.
	 * This means that the code below will never reach the abort() call
	 * below (which is just there to prevent GCC from warning us since
	 * stroll_assert_fail() is declared as a function that cannot return).
	 *
	 * Since cute_mock_assert() does not give control back to us, we MUST
	 * use a statically allocated buffer to store assertion messages. We
	 * would not have the opportunity to free(3) a previously allocated
	 * buffer otherwise.
	 * In other words, Valgrind memory leak checker should be happy with
	 * this...
	 */
	ret = snprintf(utilsut_assert_msg,
	               sz,
	               "{utest assert} %s:%s:%u:%s:\'%s\'",
	               prefix,
	               file,
	               line,
	               func,
	               expr);
	if (ret > 0) {
		if ((size_t)ret >= sz)
			utilsut_assert_msg[sz - 1] = '\0';

		cute_mock_assert(utilsut_assert_msg, file, line, func);
	}
	else
		cute_mock_assert("{utest assert} ??", file, line, func);

	/* Not reached (see comment above)... */
	abort();
}

#if defined(CONFIG_UTILS_TIME)
extern CUTE_SUITE_DECL(utilsut_time_suite);
#endif
#if defined(CONFIG_UTILS_TIMER)
extern CUTE_SUITE_DECL(utilsut_timer_suite);
#endif


CUTE_GROUP(utilsut_group) = {
#if defined(CONFIG_UTILS_TIME)
	CUTE_REF(utilsut_time_suite),
#endif
#if defined(CONFIG_UTILS_TIMER)
	CUTE_REF(utilsut_timer_suite),
#endif
};

CUTE_SUITE(utilsut_suite, utilsut_group);

CUTE_MAIN(utilsut_suite, "Utils", UTILS_VERSION_STRING)
