#include "timer_clock.h"
#include <lttng/ust-clock.h>
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

#if defined(CONFIG_ETUX_TRACE)

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
etuxpt_timer_setup_lttng_clock(void)
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

#endif /* defined(CONFIG_ETUX_TRACE) */
