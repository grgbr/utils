/**
 * @defgroup common Common
 * Common definitions
 *
 * @file
 * Common preprocessing definitions
 *
 * @ingroup      common
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
#ifndef _UTILS_H
#define _UTILS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <utils/config.h>
#include <stroll/cdefs.h>
#include <values.h>
#include <sys/cdefs.h>

#define unreachable() \
	__builtin_unreachable()

#define sizeof_member(_type, _member) \
	(sizeof(((_type *)0)->_member))

#define containerof(_ptr, _type, _member) \
	({ \
		((_type *)((char *)(_ptr) - offsetof(_type, _member))); \
	 })

#define choose_sized_expr(_const_expr, _size, _true_expr, _false_expr) \
	__builtin_choose_expr((sizeof(_const_expr) * CHAR_BIT) == (_size), \
	                      _true_expr, \
	                      _false_expr)

#define uabs(_a) \
	({ \
		typeof(_a) __a = _a; \
		(__a >= 0) ? __a : -__a; \
	 })

#define umin(_a, _b) \
	({ \
		typeof(_a) __a = _a; \
		typeof(_b) __b = _b; \
		(__a < __b) ? __a : __b; \
	 })

#define umax(_a, _b) \
	({ \
		typeof(_a) __a = _a; \
		typeof(_b) __b = _b; \
		(__a > __b) ? __a : __b; \
	 })

#define __ualign_mask(_value, _align) \
	((typeof(_value))(_align) - 1)

#define ualign_mask(_align) \
	((_align) - 1)

#define ualign_lower(_value, _align) \
	((_value) & ~__ualign_mask(_value, _align))

#define ualign_upper(_value, _align) \
	ualign_lower((_value) + __ualign_mask(_value, _align), _align)

#define uround_upper(_value, _align) \
	({ \
		typeof(_value) __value = _value; \
		typeof(_value) __align = _align; \
		\
		__value = (__value + __align - 1) / __align; \
		__value * __align; \
	 })

#define uround_lower(_value, _align) \
	({ \
		typeof(_value) __value = _value; \
		typeof(_value) __align = _align; \
		\
		__value = __value / __align; \
		__value * __align; \
	 })

#if __WORDSIZE == 64
#define __UINTPTR_C(c) c ## UL
#define UWORD_SHIFT    (6)
#elif __WORDSIZE == 32
#define __UINTPTR_C(c) c ## U
#define UWORD_SHIFT    (5)
#else
#error "Unsupported machine word size !"
#endif

#define _UCONCAT(_a, _b) \
	(_a ## _b)

#define UCONCAT(_a, _b) \
	_UCONCAT(_a, _b)

#define _USTRINGIFY(_expr) \
	# _expr

#define USTRINGIFY(_expr) \
	_USTRINGIFY(_expr) \

#define __VA_ARGS_COUNT__( _c0,  _c1,  _c2,  _c3,  _c4,  _c5,  _c6,  _c7, \
                           _c8,  _c9, _c10, _c11, _c12, _c13, _c14, _c15, \
                          _c16, _c17, _c18, _c19, _c20, _c21, _c22, _c23, \
                          _c24, _c25, _c26, _c27, _c28, _c29, _c30, _c31, \
                          _c32, _c33, _c34, _c35, _c36, _c37, _c38, _C39, \
                          _c40, _c41, _c42, _c43, _c44, _c45, _c46, _c47, \
                          _c48, _c49, _c50, _c51, _c52, _c53, _c54, _c55, \
                          _c56, _c57, _c58, _c59, _c60, _c61, _c62, _c63, \
                          _c64, _cnt, ...) \
	_cnt

#define VA_ARGS_COUNT(...) \
	__VA_ARGS_COUNT__(_dummy, \
	                  ## __VA_ARGS__, \
	                                              64, \
	                  63, 62, 61, 60, 59, 58, 57, 56, \
	                  55, 54, 53, 52, 51, 50, 49, 48, \
	                  47, 46, 45, 44, 43, 42, 41, 40, \
	                  39, 38, 37, 36, 35, 34, 33, 32, \
	                  31, 30, 29, 28, 27, 26, 25, 24, \
	                  23, 22, 21, 20, 19, 18, 17, 16, \
	                  15, 14, 13, 12, 11, 10,  9,  8, \
	                   7,  6,  5,  4,  3,  2,  1,  0)


static inline unsigned int __pure __nothrow
usys_page_size(void)
{
	extern unsigned int usys_pgsz;

	return usys_pgsz;
}

#endif /* _UTILS_H */
