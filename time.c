#include <utils/time.h>

#define utime_assert_tspec(_tspec) \
	utime_assert(_tspec); \
	utime_assert((_tspec)->tv_nsec >= 0); \
	utime_assert((_tspec)->tv_nsec < 1000000000LL)

int
utime_tspec_cmp(const struct timespec *__restrict fst,
                const struct timespec *__restrict snd)
{
	utime_assert_tspec(fst);
	utime_assert_tspec(snd);

	if (fst->tv_sec > snd->tv_sec)
		return 1;
	else if (fst->tv_sec < snd->tv_sec)
		return -1;
	else {
		if (fst->tv_nsec > snd->tv_nsec)
			return 1;
		if (fst->tv_nsec < snd->tv_nsec)
			return -1;
		else
			return 0;
	}
}

void
utime_tspec_add(struct timespec       *__restrict result,
                const struct timespec *__restrict amount)
{
	utime_assert_tspec(result);
	utime_assert_tspec(amount);
	utime_assert(amount->tv_sec >= 0);

	long nsec = result->tv_nsec + amount->tv_nsec;

	if (nsec >= 1000000000L) {
		result->tv_sec += amount->tv_sec + 1;
		result->tv_nsec = nsec - 1000000000L;
	}
	else {
		result->tv_sec += amount->tv_sec;
		result->tv_nsec = nsec;
	}
}

void
utime_tspec_add_msec(struct timespec *result, unsigned long msec)
{
	struct timespec tspec;

	tspec = utime_tspec_from_msec(msec);
	utime_tspec_add(result, &tspec);
}

void
utime_tspec_sub(struct timespec       *__restrict result,
                const struct timespec *__restrict amount)
{
	utime_assert_tspec(result);
	utime_assert_tspec(amount);
	utime_assert(amount->tv_sec >= 0);

	long nsec = result->tv_nsec - amount->tv_nsec;

	if (nsec < 0) {
		result->tv_sec -= amount->tv_sec + 1;
		result->tv_nsec = nsec + 1000000000L;
	}
	else {
		result->tv_sec -= amount->tv_sec;
		result->tv_nsec = nsec;
	}
}

void
utime_tspec_sub_msec(struct timespec *result, unsigned long msec)
{
	struct timespec tspec;

	tspec = utime_tspec_from_msec(msec);
	utime_tspec_sub(result, &tspec);
}

long
utime_tspec_diff_msec(const struct timespec *__restrict fst,
                      const struct timespec *__restrict snd)
{
	utime_assert(fst);

	struct timespec tmp = *fst;

	utime_tspec_sub(&tmp, snd);

	return utime_msec_from_tspec(&tmp);
}
