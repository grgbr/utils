#ifndef _UTILS_ASSERT_H
#define _UTILS_ASSERT_H

#include <utils/cdefs.h>

extern void
uassert_fail(const char   *restrict prefix,
             const char   *restrict expr,
             const char   *restrict file,
             unsigned int  line,
             const char   *restrict func) __nonull(1, 2, 3, 5) \
                                          __noreturn \
                                          __leaf;

#define uassert(_prefix, _expr) \
	((_expr) ? \
	 (void)(0) : \
	 uassert_fail(_prefix, \
	              __STRING(_expr), \
	              __FILE__, \
	              __LINE__, \
	              __FUNCTION__))

#endif /* _UTILS_ASSERT_H */
