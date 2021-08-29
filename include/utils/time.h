#ifndef _UTILS_TIME_H
#define _UTILS_TIME_H

#include <utils/cdefs.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __utime_nonull(_arg_index, ...)

#define utime_assert(_expr) \
	uassert("utime", _expr)

static inline void __utime_nonull(1) __nothrow
utime_realtime_now(struct timespec * now)
{
	utime_assert(now);
	
	utime_assert(!clock_gettime(CLOCK_REALTIME, now));
}

static inline void __utime_nonull(1) __nothrow
utime_monotonic_now(struct timespec * now)
{
	utime_assert(now);

	utime_assert(!clock_gettime(CLOCK_MONOTONIC, now));
}

static inline void __utime_nonull(1) __nothrow
utime_coarse_now(struct timespec * now)
{
	utime_assert(now);
	
	utime_assert(!clock_gettime(CLOCK_MONOTONIC_COARSE, now));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __utime_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define utime_assert(_expr)

static inline void __nonull(1) __nothrow
utime_realtime_now(struct timespec * now)
{
	clock_gettime(CLOCK_REALTIME, now);
}

static inline void __nonull(1) __nothrow
utime_monotonic_now(struct timespec * now)
{
	clock_gettime(CLOCK_MONOTONIC, now);
}

static inline void __nonull(1) __nothrow
utime_coarse_now(struct timespec * now)
{
	clock_gettime(CLOCK_MONOTONIC_COARSE, now);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

extern int
utime_tspec_cmp(const struct timespec * __restrict fst,
                const struct timespec * __restrict snd)
	__utime_nonull(1, 2) __pure __nothrow __leaf;

static inline bool __utime_nonull(1, 2) __pure __nothrow
utime_tspec_after(const struct timespec * __restrict fst,
                  const struct timespec * __restrict snd)
{
	return (utime_tspec_cmp(fst, snd) > 0);
}

static inline bool __utime_nonull(1, 2) __pure __nothrow
utime_tspec_after_eq(const struct timespec * __restrict fst,
                     const struct timespec * __restrict snd)
{
	return (utime_tspec_cmp(fst, snd) >= 0);
}

static inline bool __utime_nonull(1, 2) __pure __nothrow
utime_tspec_before(const struct timespec * __restrict fst,
                   const struct timespec * __restrict snd)
{
	return (utime_tspec_cmp(fst, snd) < 0);
}

static inline bool __utime_nonull(1, 2) __pure __nothrow
utime_tspec_before_eq(const struct timespec * __restrict fst,
                      const struct timespec * __restrict snd)
{
	return (utime_tspec_cmp(fst, snd) <= 0);
}

static inline struct timespec __const __nothrow
utime_tspec_from_msec(unsigned long msec)
{
	ldiv_t          res;
	struct timespec tspec;

	res = ldiv(msec, 1000UL);

	tspec.tv_sec = (time_t)res.quot;
	tspec.tv_nsec = (long)res.rem;

	return tspec;
}

static inline long __utime_nonull(1) __pure __nothrow
utime_msec_from_tspec(const struct timespec * tspec)
{
	return (tspec->tv_sec * 1000L) + (tspec->tv_nsec / 1000000L);
}

static inline void __utime_nonull(1) __nothrow
utime_tspec_add_sec(struct timespec * result, unsigned long sec)
{
	utime_assert(result);
	utime_assert(sec);

	result->tv_sec += (time_t)sec;
}

extern void
utime_tspec_add(struct timespec       * __restrict result,
                const struct timespec * __restrict amount)
	__utime_nonull(1, 2) __nothrow __leaf;

extern void
utime_tspec_add_msec(struct timespec * result, unsigned long msec)
	__utime_nonull(1) __nothrow;

extern void
utime_tspec_sub(struct timespec       * __restrict result,
                const struct timespec * __restrict amount)
	__utime_nonull(1, 2) __nothrow __leaf;

extern void
utime_tspec_sub_msec(struct timespec * result, unsigned long msec)
	__utime_nonull(1) __nothrow;

static inline void __utime_nonull(1) __nothrow
utime_tspec_sub_sec(struct timespec * result, unsigned long sec)
{
	utime_assert(result);
	utime_assert(sec);

	result->tv_sec -= (time_t)sec;
}

extern long
utime_tspec_diff_msec(const struct timespec * __restrict fst,
                      const struct timespec * __restrict snd)
	__utime_nonull(1, 2) __pure __nothrow;

#endif /* _UTILS_TIME_H */
