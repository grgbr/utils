#ifndef _ETUX_TIMER_CLOCK_H
#define _ETUX_TIMER_CLOCK_H

#include <stroll/cdefs.h>

struct timespec;

extern void
etuxpt_timer_clock_expect(const struct timespec * __restrict expected)
	__nothrow __leaf __export_intern;

extern int
etuxpt_timer_setup_lttng_clock(void) __export_intern;

#endif /* _ETUX_TIMER_CLOCK_H */
