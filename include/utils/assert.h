/**
 * @defgroup assert Assertion
 * Assertion handling
 *
 * @file
 * Assertion interface
 *
 * @ingroup      assert
 * @author       Grégor Boirie <gregor.boirie@free.fr>
 * @date         29 Aug 2017
 * @copyright    Copyright (C) 2017-2021 Grégor Boirie.
 * @licensestart GNU Lesser General Public License (LGPL) v3
 *
 * This file is part of libutils
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, If not, see <http://www.gnu.org/licenses/>.
 * @licenseend
 */
#ifndef _UTILS_ASSERT_H
#define _UTILS_ASSERT_H

#include <utils/cdefs.h>

/**
 * Output a failed assertion message.
 *
 * @param[in] prefix message prefix
 * @param[in] expr   expression that failed
 * @param[in] file   filename containing expr definition
 * @param[in] line   line number where expr is defined
 * @param[in] func   function that uses expr
 *
 * @ingroup assert
 */
extern void
uassert_fail(const char * __restrict prefix,
             const char * __restrict expr,
             const char * __restrict file,
             unsigned int            line,
             const char * __restrict func) __nonull(1, 2, 3, 5)
                                           __noreturn
                                           __leaf;

/**
 * Check an assertion.
 *
 * @param[in] _prefix prefix prepended to message generated when assertion fails
 * @param[in] _expr   expression to check assertion for
 *
 * @ingroup assert
 * @hideinitializer
 */
#define uassert(_prefix, _expr) \
	((_expr) ? \
	 (void)(0) : \
	 uassert_fail(_prefix, \
	              __STRING(_expr), \
	              __FILE__, \
	              __LINE__, \
	              __FUNCTION__))

#endif /* _UTILS_ASSERT_H */
