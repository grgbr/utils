#ifndef _UTILS_BITOPS_H
#define _UTILS_BITOPS_H

#include <utils/cdefs.h>
#include <stdint.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define bops_assert(_expr) \
	uassert("bops", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define bops_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline unsigned int __const __nothrow
bops_fls32(uint32_t mask)
{
	return (sizeof(mask) * CHAR_BIT) - __builtin_clz(mask);
}

static inline unsigned int __const __nothrow
bops_ffs32(uint32_t mask)
{
	return __builtin_ffs(mask);
}

#if __WORDSIZE == 64

static inline unsigned int __const __nothrow
bops_fls64(uint64_t mask)
{
	return (sizeof(mask) * CHAR_BIT) - __builtin_clzl(mask);
}

static inline unsigned int __const __nothrow
bops_ffs64(uint64_t mask)
{
	return __builtin_ffsl(mask);
}

#elif __WORDSIZE == 32

static inline unsigned int __const __nothrow
bops_fls64(uint64_t mask)
{
	return (sizeof(mask) * CHAR_BIT) - __builtin_clzll(mask);
}

static inline unsigned int __const __nothrow
bops_ffs64(uint64_t mask)
{
	return __builtin_ffsll(mask);
}

#else
#error "Unsupported machine word size !"
#endif

#define bops_fls(_value) \
	choose_sized_expr(_value, 32U, bops_fls32(_value), \
	choose_sized_expr(_value, 64U, bops_fls64(_value), \
	(void)0))


#define bops_ffs(_value) \
	choose_sized_expr(_value, 32U, bops_ffs32(_value), \
	choose_sized_expr(_value, 64U, bops_ffs64(_value), \
	(void)0))

#endif /* _UTILS_BITOPS_H */
