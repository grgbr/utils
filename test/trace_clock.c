/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "trace_clock.h"
#include <lttng/ust-clock.h>
#include <time.h>
#include <string.h>

extern int __clock_gettime(clockid_t, struct timespec *);

static
uint64_t
etuxpt_timer_lttng_clock_read64(void)
{
	struct timespec tspec;

	if (__clock_gettime(CLOCK_MONOTONIC, &tspec)) {
		tspec.tv_sec = 0;
		tspec.tv_nsec = 0;
	}

	return ((uint64_t)tspec.tv_sec * UINT64_C(1000000000)) +
	       (uint64_t)tspec.tv_nsec;
}

static
uint64_t
etuxpt_timer_lttng_clock_freq(void)
{
	return UINT64_C(1000000000);
}

static
int
etuxpt_timer_lttng_clock_uuid(char *uuid)
{
	/* "etuxpt_timer_clock" as stringified ascii code sequence... */
	const char id[] = "6574757870745f74696d65725f636c6f636b";

	memcpy(uuid, id, LTTNG_UST_UUID_STR_LEN);

	return 0;
}

static
const char *
etuxpt_timer_lttng_clock_name(void)
{
	return "etuxpt_timer_clock";
}

static
const char *
etuxpt_timer_lttng_clock_desc(void)
{
	return "eTux timer performance monotonic clock";
}

/*
 * Make sure that LTTng uses the system monotonic clock instead of our own timer
 * performance clock installed by overriding clock_gettime() above.
 */
int
etuxpt_timer_setup_trace_clock(void)
{
	int err;

	err = lttng_ust_trace_clock_set_read64_cb(
		etuxpt_timer_lttng_clock_read64);
	if (err)
		return err;

	err = lttng_ust_trace_clock_set_freq_cb(
		etuxpt_timer_lttng_clock_freq);
	if (err)
		return err;

	err = lttng_ust_trace_clock_set_uuid_cb(
		etuxpt_timer_lttng_clock_uuid);
	if (err)
		return err;

	err = lttng_ust_trace_clock_set_name_cb(
		etuxpt_timer_lttng_clock_name);
	if (err)
		return err;

	err = lttng_ust_trace_clock_set_description_cb(
		etuxpt_timer_lttng_clock_desc);
	if (err)
		return err;

	err = lttng_ust_enable_trace_clock_override();
	if (err)
		return err;

	return 0;
}
