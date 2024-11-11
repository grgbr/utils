/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _UTILS_TIMER_INTERN_H
#define _UTILS_TIMER_INTERN_H

#include "utils/timer.h"

/*
 * Timer ticks sub second precision bits.
 *
 * Configure the tick period to 1/(2^UTIMER_TICK_SUBSEC_BITS) second.
 *
 * The table below gives tick sub second precision according to allowed
 * UTIMER_TICK_SUBSEC_BITS values:
 *
 *     UTIMER_TICK_SUBSEC_BITS     Tick period  Tick frequency
 *                              (milliseconds)         (Hertz)
 *                           0    1000.000000               1
 *                           1     500.000000               2
 *                           2     250.000000               4
 *                           3     125.000000               8
 *                           4      62.500000              16
 *                           5      31.250000              32
 *                           6      15.625000              64
 *                           7       7.812500             128
 *                           8       3.906250             256
 *                           9       1.953125             512
 *
 * Watch out!
 * The tick period MUST be a divisor of 1000000000 nanoseconds so that we can
 * perform power of 2 arithmetics (see utimer_tick_from_tspec_lower(),
 * utimer_tick_from_tspec_upper() and utimer_tspec_from_tick()).
 * This is the reason why UTIMER_TICK_SUBSEC_BITS MUST be < 10.
 */
#define UTIMER_TICK_SUBSEC_BITS \
	STROLL_CONCAT(CONFIG_UTILS_TIMER_SUBSEC_BITS, U)
#if (UTIMER_TICK_SUBSEC_BITS < 0) || (UTIMER_TICK_SUBSEC_BITS > 9)
#error Invalid tick sub second precision bits.
#endif

#define UTIMER_TICK_SUBSEC_MASK \
	((INT64_C(1) << UTIMER_TICK_SUBSEC_BITS) - 1)

/* Period of a tick in nanoseconds */
#define UTIMER_TICK_NSEC \
	(INT64_C(1000000000) >> UTIMER_TICK_SUBSEC_BITS)

/* Number of ticks per second. */
#define UTIMER_TICKS_PER_SEC \
	(1UL << UTIMER_TICK_SUBSEC_BITS)

/*
 * Maximum tick value that can be encoded.
 *
 * Allows to prevent overflow while converting a tick's second part to the
 * tv_sec field of a struct timespec (tv_sec is a time_t, i.e., either a signed
 * 64 or 32 bits word).
 */
#if UTIME_TIMET_BITS == 64

#define UTIMER_TICK_MAX \
	(INT64_MAX)

#elif UTIME_TIMET_BITS == 32

#define UTIMER_TICK_MAX \
	(((int64_t)(INT32_MAX) << UTIMER_TICK_SUBSEC_BITS) | \
	 UTIMER_TICK_SUBSEC_MASK)

#else
#error Unexpected time_t bit width value (can only be 32 or 64-bit) !
#endif

/*
 * Maximum value of a struct timespec's tv_sec field that can be converted to a
 * tick.
 */
#define UTIMER_TVSEC_MAX \
	((time_t)(UTIMER_TICK_MAX >> UTIMER_TICK_SUBSEC_BITS))

#endif /* _UTILS_TIMER_INTERN_H */
