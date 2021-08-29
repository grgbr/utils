#include "utils/timer.h"

static struct dlist_node utime_timers = DLIST_INIT(utime_timers);

void __utime_nonull(1) __nothrow
utime_timer_arm(struct utime_timer * timer)
{
	utime_assert(timer);
	utime_assert(timer->expire);

	struct dlist_node *     node;
        const struct timespec * date = &timer->date;

	dlist_remove_init(&timer->node);

	for (node = dlist_prev(&utime_timers);
	     node != &utime_timers;
	     node = dlist_prev(node)) {
		const struct utime_timer * t = dlist_entry(node,
		                                           struct utime_timer,
		                                           node);
		if (utime_tspec_after_eq(date, &t->date))
			break;
	}

	dlist_append(node, &timer->node);
}

void __utime_nonull(1) __nothrow
utime_timer_arm_msec(struct utime_timer * timer, unsigned long msec)
{
	utime_assert(timer);
	utime_assert(timer->expire);

	utime_coarse_now(&timer->date);
	utime_tspec_add_msec(&timer->date, msec);
	utime_timer_arm(timer);
}

void __utime_nonull(1) __nothrow
utime_timer_arm_sec(struct utime_timer * timer, unsigned long sec)
{
	utime_assert(timer);
	utime_assert(timer->expire);

	utime_coarse_now(&timer->date);
	utime_tspec_add_sec(&timer->date, sec);
	utime_timer_arm(timer);
}

void __nothrow
utime_timer_run(void)
{
	struct timespec now;

	utime_coarse_now(&now);

	while (!dlist_empty(&utime_timers)) {
		struct utime_timer * timer;

		timer = dlist_entry(dlist_next(&utime_timers),
		                    struct utime_timer,
		                    node);

		if (utime_tspec_after_eq(&timer->date, &now))
			break;

		dlist_remove_init(&timer->node);
		timer->expire(timer);
	}
}
