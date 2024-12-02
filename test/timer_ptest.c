#include "utils/timer.h"
#include <stdio.h>

enum etuxpt_timer_op {
	ETUXPT_TIMER_ARM_TSPEC_OP,
	ETUXPT_TIMER_ARM_MSEC_OP,
	ETUXPT_TIMER_CANCEL_OP,
	ETUXPT_TIMER_RUN_OP,
	ETUXPT_TIMER_OP_NR,
};

struct etuxpt_timer_cmd;
typedef void (etuxpt_timer_op_fn)(const struct etuxpt_timer_cmd * __restrict);

struct etuxpt_timer_cmd {
	etuxpt_timer_op_fn *          op;
	unsigned long                 tmr_id;
	union {
		const struct timespec tspec;
		int                   msec;
		int                   sec;
	};
};

struct etuxpt_timer {
	struct etux_timer * base;
	unsigned long       id;
};

static unsigned int          etuxpt_timer_cnt;
static unsigned int          etuxpt_timer_nr;
static struct etuxpt_timer * etuxpt_timers;

static
struct etuxpt_timer *
etuxpt_timer_get(unsigned long id)
{
	unsigned int t;

	for (t = 0; t < etuxpt_timer_cnt; t++)
		if (etuxpt_timers[t].id == id)
			return &etuxpt_timers[t];

	return NULL;
}

static
void
etuxpt_timer_arm_tspec_op(const struct etuxpt_timer_cmd * __restrict command)
{
	struct etuxpt_timer * tmr = etuxpt_timer_get(command->tmr_id);

	assert(tmr);

	etux_timer_arm_tspec(tmr->base, &command->tspec);
}

static
void
etuxpt_timer_arm_msec_op(const struct etuxpt_timer_cmd * __restrict command)
{
	struct etuxpt_timer * tmr = etuxpt_timer_get(command->tmr_id);

	assert(tmr);

	etux_timer_arm_msec(tmr->base, command->msec);
}

static
void
etuxpt_timer_arm_sec_op(const struct etuxpt_timer_cmd * __restrict command)
{
	struct etuxpt_timer * tmr = etuxpt_timer_get(command->tmr_id);

	assert(tmr);

	etux_timer_arm_sec(tmr->base, command->sec);
}

static
void
etuxpt_timer_cancel_op(const struct etuxpt_timer_cmd * __restrict command)
{
	struct etuxpt_timer * tmr = etuxpt_timer_get(command->tmr_id);

	assert(tmr);

	etux_timer_cancel(tmr->base);
}

static
void
etuxpt_timer_run_op(const struct etuxpt_timer_cmd * __restrict command)
{
	struct etuxpt_timer * tmr = etuxpt_timer_get(command->tmr_id);

	assert(tmr);

	etux_timer_run();
}

static unsigned int                  etuxpt_timer_cmd_nr;
static const struct etuxpt_timer_cmd etuxpt_timer_cmds[] = {
	{
		.op     = etuxpt_timer_arm_tspec_op,
		.tmr_id = 0,
		.tspec  = { .tv_sec  = 0, .tv_nsec = 0 }
	},
	{
		.op     = etuxpt_timer_arm_msec_op,
		.tmr_id = 0,
		.msec   = 100
	},
	{
		.op     = etuxpt_timer_arm_sec_op,
		.tmr_id = 0,
		.msec   = 2
	},
	{
		.op     = etuxpt_timer_run_op,
		.tmr_id = 0
	},
	{
		.op     = etuxpt_timer_cancel_op,
		.tmr_id = 0
	},
	{
		.op     = etuxpt_timer_arm_msec_op,
		.tmr_id = 0,
		.msec   = 50
	},
	{
		.op     = etuxpt_timer_run_op,
		.tmr_id = 0
	}
};

static
void
etuxpt_timer_expire(struct etux_timer * timer __unused)
{
}

static
struct etuxpt_timer *
etuxpt_timer_build(unsigned long id)
{
	struct etuxpt_timer * tmr;

	tmr = etuxpt_timer_get(id);
	if (!tmr) {
		struct etux_timer * base;

		if (etuxpt_timer_cnt >= etuxpt_timer_nr) {
			unsigned int nr = stroll_max(etuxpt_timer_nr * 2, 1U);

			tmr = realloc(etuxpt_timers,
			              nr * sizeof(etuxpt_timers[0]));
			if (!tmr)
				return NULL;

			etuxpt_timers = tmr;
			etuxpt_timer_nr = nr;
		}

		base = malloc(sizeof(*base));
		if (!base)
			return NULL;
		etux_timer_init(base, etuxpt_timer_expire);

		etuxpt_timers[etuxpt_timer_cnt].base = base;
		etuxpt_timers[etuxpt_timer_cnt].id = id;

		return &etuxpt_timers[etuxpt_timer_cnt++];
	}

	return tmr;
}

static
void
etuxpt_timer_load_cmds(void)
{
	unsigned int c;

	for (c = 0; c < stroll_array_nr(etuxpt_timer_cmds); c++)
		etuxpt_timer_build(etuxpt_timer_cmds[c].tmr_id);

	etuxpt_timer_cmd_nr = c;
}

static
void
etuxpt_timer_run_cmds(void)
{
	unsigned int c;

	for (c = 0; c < etuxpt_timer_cmd_nr; c++)
		etuxpt_timer_cmds[c].op(&etuxpt_timer_cmds[c]);
}

static
void
etuxpt_timer_unload_cmds(void)
{
	unsigned int t;

	for (t = 0; t < etuxpt_timer_cnt; t++)
		free(etuxpt_timers[t].base);

	free(etuxpt_timers);
}

int
main(int argc, char * const argv[])
{
	etuxpt_timer_load_cmds();
	etuxpt_timer_run_cmds();
	etuxpt_timer_unload_cmds();
}
