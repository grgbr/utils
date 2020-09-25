#ifndef _UTILS_POW2_H
#define _UTILS_POW2_H

#include <utils/bitops.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#define pow2_assert(_expr) \
	uassert("pow2", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define pow2_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline unsigned int __const __nothrow
pow2_lower32(uint32_t value)
{
	pow2_assert(value);

	return bops_fls32(value) - 1;
}

static inline unsigned int __const __nothrow
pow2_lower64(uint64_t value)
{
	pow2_assert(value);

	return bops_fls64(value) - 1;
}

#define pow2_lower(_value) \
	choose_sized_expr(_value, 32U, pow2_lower32(_value), \
	choose_sized_expr(_value, 64U, pow2_lower64(_value), \
	(void)0))

static inline unsigned int __const __nothrow
pow2_upper32(uint32_t value)
{
	/* Would overflow otherwise... */
	pow2_assert(value <= (UINT32_C(1) << ((sizeof(value) * CHAR_BIT) - 1)));
	pow2_assert(value);

	return pow2_lower32(value + (UINT32_C(1) << pow2_lower32(value)) - 1);
}

static inline unsigned int __const __nothrow
pow2_upper64(uint64_t value)
{
	/* Would overflow otherwise... */
	pow2_assert(value <= (UINT64_C(1) << ((sizeof(value) * CHAR_BIT) - 1)));
	pow2_assert(value);

	return pow2_lower64(value + (UINT64_C(1) << pow2_lower64(value)) - 1);
}

#define pow2_upper(_value) \
	choose_sized_expr(_value, 32U, pow2_upper32(_value), \
	choose_sized_expr(_value, 64U, pow2_upper64(_value), \
	(void)0))

#endif /* _UTILS_BITOPS_H */
