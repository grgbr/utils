#include "ptest.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>

int
etuxpt_parse_sched_prio(const char * __restrict arg,
                        int * __restrict        priority)
{
	assert(arg);
	assert(priority);

	char * str;
	int    prio;
	int    err = 0;

	prio = (int)strtoul(arg, &str, 0);
	if (*str)
		err = EINVAL;
	else if ((prio < sched_get_priority_min(SCHED_FIFO)) ||
	         (prio > sched_get_priority_max(SCHED_FIFO)))
		err = ERANGE;

	if (err) {
		etuxpt_err("invalid scheduling priority '%s' specified: "
		           "%s (%d).\n",
		           arg,
		           strerror(err),
		           err);
		return EXIT_FAILURE;
	}

	*priority = prio;

	return EXIT_SUCCESS;
}

int
etuxpt_setup_sched_prio(int priority)
{
	if (priority) {
		assert(priority >= sched_get_priority_min(SCHED_FIFO));
		assert(priority <= sched_get_priority_max(SCHED_FIFO));

		const struct sched_param parm = { .sched_priority = priority };

		if (sched_setscheduler(getpid(), SCHED_FIFO, &parm)) {
			etuxpt_err("failed set scheduling policy: "
			           "%s (%d).\n",
			           strerror(errno),
			           errno);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
