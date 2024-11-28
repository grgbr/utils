#include "utils/timer.h"
#include <stdio.h>

enum etuxpt_timer_op {
	ETUXPT_TIMER_ARM_TSPEC_TYPE,
	ETUXPT_TIMER_ARM_MSEC_TYPE,
	ETUXPT_TIMER_CANCEL_TYPE,
	ETUXPT_TIMER_RUN_TYPE,
	ETUXPT_TIMER_TYPE_NR,
};

struct etuxpt_timer_cmd {
	enum etuxpt_timer_op          op;
	unsigned long                 id;
	union {
		const struct timespec arm_tspec;
		int                   arm_msec;
	}
};

static const struct etuxpt_timer_cmd etuxpt_timer_cmds[] = {

};

static const struct etuxpt_timer_cmd etuxpt_timer_cmd_nr;

static struct etux_timer *           etuxpt_timers;
static unsigned int                  etuxpt_timer_nr;

int
main(int argc, char * const argv[])
{
        printf("test\n");
}
