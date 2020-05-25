#ifndef _UTILS_SIGNAL_H
#define _UTILS_SIGNAL_H

#include <utils/cdefs.h>
#include <signal.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __usig_nonull(_arg_index, ...)

#define usig_assert(_expr) \
	uassert("usignal", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __usig_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define usig_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __nothrow
usig_action(int signum, const struct sigaction *act, struct sigaction *oldact)
{
	usig_assert(act || oldact);

	usig_assert(!sigaction(signum, act, oldact));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __nothrow
usig_action(int signum, const struct sigaction *act, struct sigaction *oldact)
{
	sigaction(signum, act, oldact);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline int __usig_nonull(1) __nothrow
usig_isemptyset(sigset_t *set)
{
	usig_assert(set);

	return !!sigisemptyset(set);
}

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __usig_nonull(1) __nothrow
usig_emptyset(sigset_t *set)
{
	usig_assert(set);

	usig_assert(!sigemptyset(set));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __usig_nonull(1) __nothrow
usig_emptyset(sigset_t *set)
{
	sigemptyset(set);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __usig_nonull(1) __nothrow
usig_fillset(sigset_t *set)
{
	usig_assert(set);

	usig_assert(!sigfillset(set));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __usig_nonull(1) __nothrow
usig_fillset(sigset_t *set)
{
	sigfillset(set);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __usig_nonull(1) __nothrow
usig_addset(sigset_t *set, int signum)
{
	usig_assert(set);

	usig_assert(!sigaddset(set, signum));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __usig_nonull(1) __nothrow
usig_addset(sigset_t *set, int signum)
{
	sigaddset(set, signum);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __usig_nonull(1) __nothrow
usig_delset(sigset_t *set, int signum)
{
	usig_assert(set);

	usig_assert(!sigdelset(set, signum));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __usig_nonull(1) __nothrow
usig_delset(sigset_t *set, int signum)
{
	sigdelset(set, signum);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#endif /* _UTILS_SIGNAL_H */
