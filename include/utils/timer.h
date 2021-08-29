#ifndef _UTILS_TIMER_H
#define _UTILS_TIMER_H

#include <utils/time.h>
#include <utils/dlist.h>

struct utime_timer;

typedef void (utime_expire_fn)(struct utime_timer * timer);

struct utime_timer {
	struct dlist_node node;
	struct timespec   date;
	utime_expire_fn * expire;
};

#define UTIME_TIMER_INIT(_timer, _expire) \
	{ \
		.node   = DLIST_INIT((_timer).node), \
		.expire = _expire \
	}

static inline struct timespec * __utime_nonull(1) __const  __nothrow
utime_timer_expiry_date(struct utime_timer * timer)
{
	utime_assert(timer);
	utime_assert(timer->expire);

	return &timer->date;
}

static inline void __utime_nonull(1) __nothrow
utime_timer_cancel(struct utime_timer * timer)
{
	utime_assert(timer);
	utime_assert(timer->expire);

	dlist_remove_init(&timer->node);
}

static inline void __utime_nonull(1, 2) __nothrow
utime_timer_init(struct utime_timer * __restrict timer,
                 utime_expire_fn *               expire)
{
	dlist_init(&timer->node);
	timer->expire = expire;
}

extern void
utime_timer_arm(struct utime_timer * timer) __utime_nonull(1) __nothrow;

extern void
utime_timer_arm_msec(struct utime_timer * timer, unsigned long msec)
	__utime_nonull(1) __nothrow;

extern void
utime_timer_arm_sec(struct utime_timer * timer, unsigned long sec)
	__utime_nonull(1) __nothrow;

extern void utime_timer_run(void) __nothrow;

#endif /* _UTILS_TIMER_H */
