#ifndef _UTILS_H
#define _UTILS_H

#include <utils/config.h>
#include <stddef.h>
#include <values.h>
#include <sys/cdefs.h>

#define __unused \
	__attribute__((unused))

#define __warn_result \
	__attribute__((warn_unused_result))

#define __noreturn \
	__attribute__((noreturn))

#define __returns_nonull \
	__attribute__((returns_nonnull))

#define __nonull(_arg_index, ...) \
	__attribute__((nonnull(_arg_index, ## __VA_ARGS__)))

#define __printf(_format_index, _arg_index) \
	__attribute__((format(printf, _format_index, _arg_index)))

#define __nothrow \
	__attribute__((nothrow))

#define __leaf \
	__attribute__((leaf))

#define __pure \
	__attribute__((pure))

#define __const \
	__attribute__((const))

#define __align(_size) \
	__attribute__((aligned(_size)))

#define likely(_expr) \
	__builtin_expect(!!(_expr), 1)

#define unlikely(_expr) \
	__builtin_expect(!!(_expr), 0)

#define PREFETCH_ACCESS_RO     (0)
#define PREFETCH_ACCESS_RW     (1)
#define PREFETCH_LOCALITY_TMP  (0)
#define PREFETCH_LOCALITY_LOW  (1)
#define PREFETCH_LOCALITY_FAIR (2)
#define PREFETCH_LOCALITY_HIGH (3)

#define prefetch(_address, ...) \
	__builtin_prefetch(_address, ## __VA_ARGS__)

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

#define array_nr(_array) \
	(sizeof(_array) / sizeof(_array[0]))

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

#if __WORDSIZE == 64
#define __UINTPTR_C(c) c ## UL
#define UWORD_SHIFT    (6)
#elif __WORDSIZE == 32
#define __UINTPTR_C(c) c ## U
#define UWORD_SHIFT    (5)
#else
#error "Unsupported machine word size !"
#endif

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

#endif /* _UTILS_H */
