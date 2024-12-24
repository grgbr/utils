#ifndef _ETUX_TEST_TIMER_CLOCK_H
#define _ETUX_TEST_TIMER_CLOCK_H

#include "utils/cdefs.h"

struct timespec;

extern void
etuxpt_timer_clock_expect(const struct timespec * __restrict expected)
	__nothrow __leaf __export_intern;

#endif /* _ETUX_TEST_TIMER_CLOCK_H */
