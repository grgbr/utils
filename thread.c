#include <utils/thread.h>
#include <utils/time.h>
#include <stdlib.h>
#include <errno.h>

int
uthr_timed_wait_cond_msec(struct uthr_cond  *restrict cond,
                          struct uthr_mutex *restrict mutex,
                          unsigned long      tmout_msec)
{
	uthr_assert(cond);
	uthr_assert(mutex);

	if (tmout_msec) {
		struct timespec tmout;

		uthr_cond_now(cond, &tmout);
		utime_tspec_add_msec(&tmout, tmout_msec);

		return uthr_timed_wait_cond(cond, mutex, &tmout);
	}

	return -ETIMEDOUT;
}

int
uthr_init_cond(struct uthr_cond *cond, clockid_t clock)
{
	int                ret;
	pthread_condattr_t attr;

	ret = pthread_condattr_init(&attr);
	uthr_assert(ret != EINVAL);
	if (ret)
		return -ret;

	ret = pthread_condattr_setclock(&attr, clock);
	if (ret)
		goto destroy;

	ret = pthread_cond_init(&cond->pthread, &attr);
	uthr_assert(ret != EBUSY);
	uthr_assert(ret != EINVAL);

	cond->clock = clock;

destroy:
	pthread_condattr_destroy(&attr);

	return -ret;
}
