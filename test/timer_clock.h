#ifndef _ETUX_TIMER_CLOCK_H
#define _ETUX_TIMER_CLOCK_H

#include "utils/cdefs.h"

struct timespec;

extern void
etuxpt_timer_clock_expect(const struct timespec * __restrict expected)
	__nothrow __leaf __export_intern;

#if defined(CONFIG_ETUX_TRACE)

extern int
etuxpt_timer_setup_lttng_clock(void) __export_intern;

#endif /* defined(CONFIG_ETUX_TRACE) */

#endif /* _ETUX_TIMER_CLOCK_H */
