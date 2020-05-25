#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <values.h>
#include <assert.h>
#include <sys/cdefs.h>

#define __unused \
	__attribute__((unused))

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

#define sizeof_member(_type, _member) \
	(sizeof(((_type *)0)->_member))

#define array_count(_array) \
	(sizeof(_array) / sizeof(_array[0]))

#define min(_a, _b) \
	({ \
		typeof(_a) __a = _a; \
		typeof(_b) __b = _b; \
		(__a < __b) ? __a : __b; \
	 })

#define max(_a, _b) \
	({ \
		typeof(_a) __a = _a; \
		typeof(_b) __b = _b; \
		(__a > __b) ? __a : __b; \
	 })

#define containerof(_ptr, _type, _member) \
	({ \
		((_type *)((char *)(_ptr) - offsetof(_type, _member))); \
	 })

extern void
utils_assert_fail(const char   *prefix,
                  const char   *expr,
                  const char   *file,
                  unsigned int  line,
                  const char   *func) __nonull(1, 2, 3, 5) \
                                      __noreturn \
                                      __leaf;

#define utils_assert(_prefix, _expr) \
	((_expr) ? \
	 (void)(0) : \
	 utils_assert_fail(_prefix, \
	                   __STRING(_expr), \
	                   __FILE__, \
	                   __LINE__, \
	                   __FUNCTION__))

static inline unsigned int __const __nothrow
utils_fls64(uint64_t mask)
{
	assert(mask);

	return (sizeof(mask) * CHAR_BIT) - 1 - __builtin_clzll(mask);
}

static inline unsigned int __const __nothrow
utils_ffs64(uint64_t mask)
{
	assert(mask);

	return __builtin_ffsll(mask) - 1;
}

#endif /* _UTILS_H */
