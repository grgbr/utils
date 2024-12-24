#include "timer_clock.h"
#include <time.h>
#include <stdbool.h>
#include <string.h>

/*
 * For some reason, when built with Link-Time Optimization (LTO), some
 * `etuxpt_timer_clock_on' variable accesses get optimized out...
 * Use volatile keyword to enforce GCC to generate load/store instructions...
 */
static volatile bool   etuxpt_timer_clock_on;
static struct timespec etuxpt_now;

extern int __clock_gettime(clockid_t, struct timespec *);

/*
 * Override glibc's clock_gettime() with our own implementation while still
 * keeping the ability to call the original clock_gettime() syscall when needed.
 */
int
clock_gettime(clockid_t id, struct timespec * tspec)
{
	if (etuxpt_timer_clock_on) {
		/*
		 * Set struct timespec content using values given by
		 * etuxpt_timer_clock_expect().
		 */
		*tspec = etuxpt_now;

		return 0;
	}

	/* Mocking is off: use normal clock_gettime() syscall... */
	return __clock_gettime(id, tspec);
}

void
etuxpt_timer_clock_expect(const struct timespec * __restrict expected)
{
	if (expected) {
		/*
		 * Setup timespec value to return into timespec structure at
		 * clock_gettime() calling time.
		 */
		etuxpt_now.tv_sec = expected->tv_sec;
		etuxpt_now.tv_nsec = expected->tv_nsec;

		/* Tell clock_gettime() that mocking is on... */
		etuxpt_timer_clock_on = true;
	}
	else
		etuxpt_timer_clock_on = false;
}
