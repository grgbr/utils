#include "utils/timer.h"
#include "timer_clock.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define etuxpt_err(_format, ...) \
	fprintf(stderr, \
	        "%s: " _format, \
	        program_invocation_short_name, \
	        ## __VA_ARGS__)

enum etuxpt_timer_kind {
	ETUXPT_TIMER_ARM_TSPEC_KIND = 0,
	ETUXPT_TIMER_ARM_MSEC_KIND  = 1,
	ETUXPT_TIMER_ARM_SEC_KIND   = 2,
	ETUXPT_TIMER_CANCEL_KIND    = 3,
	ETUXPT_TIMER_ISSUE_TSPEC    = 4,
	ETUXPT_TIMER_ISSUE_MSEC     = 5,
	ETUXPT_TIMER_RUN_KIND       = 6,
	ETUXPT_TIMER_EXPIRE_KIND    = 7,
	ETUXPT_TIMER_KIND_NR
};

struct etuxpt_timer_event;
typedef void (etuxpt_timer_process_fn)
             (const struct etuxpt_timer_event * __restrict);

struct etuxpt_timer_event {
	etuxpt_timer_process_fn *     process;
	struct timespec               stamp;
	char *                        task;
	unsigned long                 addr;
	union {
		struct timespec       tspec;
		int                   msec;
		int                   sec;
	};
};

struct etuxpt_timer {
	unsigned long       id;
	struct etux_timer * base;
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
etuxpt_timer_process_null(
	const struct etuxpt_timer_event * __restrict event __unused)
{
}

static
void
etuxpt_timer_process_arm_tspec(
	const struct etuxpt_timer_event * __restrict event)
{
	struct etuxpt_timer * tmr = etuxpt_timer_get(event->addr);

	assert(tmr);

	etux_timer_arm_tspec(tmr->base, &event->tspec);
}

static
void
etuxpt_timer_process_arm_msec(
	const struct etuxpt_timer_event * __restrict event)
{
	struct etuxpt_timer * tmr = etuxpt_timer_get(event->addr);

	assert(tmr);

	etux_timer_arm_msec(tmr->base, event->msec);
}

static
void
etuxpt_timer_process_arm_sec(
	const struct etuxpt_timer_event * __restrict event)
{
	struct etuxpt_timer * tmr = etuxpt_timer_get(event->addr);

	assert(tmr);

	etux_timer_arm_sec(tmr->base, event->sec);
}

static
void
etuxpt_timer_process_cancel(const struct etuxpt_timer_event * __restrict event)
{
	struct etuxpt_timer * tmr = etuxpt_timer_get(event->addr);

	assert(tmr);

	etux_timer_cancel(tmr->base);
}

static
void
etuxpt_timer_process_run(
	const struct etuxpt_timer_event * __restrict event  __unused)
{
	if (!etux_timer_issue_msec())
		etux_timer_run();
}

static
void
etuxpt_timer_expire(struct etux_timer * timer __unused)
{
#if 0
	struct timespec             now;
	struct timespec             diff;
	const struct timespec *     exp;

	utime_monotonic_now(&now);

	diff = now;
	exp = etux_timer_expiry_tspec(timer);
	if (utime_tspec_sub(&diff, exp) < 0)
		assert(0);

	printf("<%ld.%09ld> expire=%ld.%09ld latency=%ld.%09ld\n",
	       now.tv_sec, now.tv_nsec,
	       exp->tv_sec, exp->tv_nsec,
	       diff.tv_sec, diff.tv_nsec);
#endif
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

		etuxpt_timers[etuxpt_timer_cnt].id = id;
		etuxpt_timers[etuxpt_timer_cnt].base = base;

		return &etuxpt_timers[etuxpt_timer_cnt++];
	}

	return tmr;
}

enum etuxpt_endian {
	ETUXPT_NATIVE_ENDIAN,
	ETUXPT_LITTLE_ENDIAN,
	ETUXPT_BIG_ENDIAN
};

struct etuxpt_timer_data {
	FILE *             file;
	enum etuxpt_endian endian;
	unsigned int       nr;
	long               start;
};

static int
etuxpt_timer_read_data_u8(const struct etuxpt_timer_data * __restrict data,
                          uint8_t * __restrict                        value)
{
	if (fread(value, sizeof(*value), 1, data->file) == 1)
		return 0;

	if (feof(data->file))
		return -EPERM;

	return -EIO;
}

static int
etuxpt_timer_read_data_u16(const struct etuxpt_timer_data * __restrict data,
                           uint16_t * __restrict                       value)
{
	if (fread(value, sizeof(*value), 1, data->file) == 1) {
		switch (data->endian) {
		case ETUXPT_NATIVE_ENDIAN:
			break;
		case ETUXPT_LITTLE_ENDIAN:
			*value = le16toh(*value);
			break;
		case ETUXPT_BIG_ENDIAN:
			*value = be16toh(*value);
			break;
		default:
			assert(0);
		}

		return 0;
	}

	if (feof(data->file))
		return -EPERM;

	return -EIO;
}

static int
etuxpt_timer_read_data_u64(const struct etuxpt_timer_data * __restrict data,
                           uint64_t * __restrict                       value)
{
	if (fread(value, sizeof(*value), 1, data->file) == 1) {
		switch (data->endian) {
		case ETUXPT_NATIVE_ENDIAN:
			break;
		case ETUXPT_LITTLE_ENDIAN:
			*value = le64toh(*value);
			break;
		case ETUXPT_BIG_ENDIAN:
			*value = be64toh(*value);
			break;
		default:
			assert(0);
		}

		return 0;
	}

	if (feof(data->file))
		return -EPERM;

	return -EIO;
}

static int
etuxpt_timer_read_data_i64(const struct etuxpt_timer_data * __restrict data,
                           int64_t * __restrict                        value)
{
	return etuxpt_timer_read_data_u64(data, (uint64_t *)value);
}

static ssize_t
etuxpt_timer_read_data_str(const struct etuxpt_timer_data * __restrict data,
                           char ** __restrict                          value)
{
	uint16_t len;
	char *   str;
	int      err;

	err = etuxpt_timer_read_data_u16(data, &len);
	if (err)
		return err;

	str = malloc(len + 1);
	if (!str)
		return -errno;

	if (fread(str, (size_t)len, 1, data->file) == 1) {
		str[len] = '\0';
		*value = str;

		return (ssize_t)len;
	}

	free(str);

	if (feof(data->file))
		return -EPERM;

	return -EIO;
}

static int
etuxpt_timer_read_data_tspec(const struct etuxpt_timer_data * __restrict data,
                             struct timespec * __restrict                tspec)
{
	int     err;
	int64_t sec;
	int64_t nsec;

	err = etuxpt_timer_read_data_i64(data, &sec);
	if (err)
		return err;
	err = etuxpt_timer_read_data_i64(data, &nsec);
	if (err)
		return err;

	if ((sec < 0) || (sec > (int64_t)UTIME_TIMET_MAX))
		return -ERANGE;
	if ((nsec < 0) || (nsec > 1000000000L))
		return -ERANGE;

	tspec->tv_sec = (time_t)sec;
	tspec->tv_nsec = (long)nsec;

	return 0;
}

static int
etuxpt_timer_read_data_stamp(const struct etuxpt_timer_data * __restrict data,
                             struct timespec * __restrict                stamp)
{
	int err;

	err = etuxpt_timer_read_data_tspec(data, stamp);
	if (err) {
		if (err == -EPERM)
			return -EPERM;
		etuxpt_err("failed to fetch timer event timestamp: %s (%d).\n",
		           strerror(-err),
		           -err);
		return err;
	}

	return 0;
}

static int
etuxpt_timer_read_data_task(const struct etuxpt_timer_data * __restrict data,
                            char ** __restrict                          task)
{
	ssize_t ret;
	char *  str = NULL;

	ret = etuxpt_timer_read_data_str(data, &str);
	if (ret < 0) {
		if (ret == -EPERM)
			return -EPERM;
		etuxpt_err("failed to fetch timer event task name: %s (%d).\n",
		           strerror((int)(-ret)),
		           (int)(-ret));
		return (int)ret;
	}

#define ETUXPT_TIMER_TASK_LEN_MAX (255)
	if (!ret || (ret > ETUXPT_TIMER_TASK_LEN_MAX)) {
		etuxpt_err("failed to fetch timer event task name: %s (%d).\n",
		           strerror(ENAMETOOLONG),
		           ENAMETOOLONG);
		free(str);
		return -ENAMETOOLONG;
	}

	*task = str;

	return 0;
}

static int
etuxpt_timer_read_data_kind(const struct etuxpt_timer_data * __restrict data,
                            enum etuxpt_timer_kind * __restrict         kind)
{
	int     err;
	uint8_t val;

	err = etuxpt_timer_read_data_u8(data, &val);
	if (err) {
		if (err == -EPERM)
			return -EPERM;
		etuxpt_err("failed to fetch timer event kind: %s (%d).\n",
		           strerror(-err),
		           -err);
		return err;
	}

	if ((enum etuxpt_timer_kind)val >= ETUXPT_TIMER_KIND_NR) {
		etuxpt_err("failed to fetch timer event kind: %s (%d).\n",
		           strerror(EINVAL),
		           EINVAL);
		return -EINVAL;
	}

	*kind = (enum etuxpt_timer_kind)val;

	return 0;
}

static int
etuxpt_timer_read_data_addr(const struct etuxpt_timer_data * __restrict data,
                            unsigned long * __restrict                  addr)
{
	int err;

	err = etuxpt_timer_read_data_u64(data, addr);
	if (err) {
		if (err == -EPERM)
			return -EPERM;
		etuxpt_err("failed to fetch timer event address: %s (%d).\n",
		           strerror(-err),
		           -err);
		return err;
	}

	if (!*addr) {
		etuxpt_err("failed to fetch timer event address: %s (%d).\n",
		           strerror(ERANGE),
		           -ERANGE);
		return -ERANGE;
	}

	return 0;
}

static int
etuxpt_timer_read_data_expiry(const struct etuxpt_timer_data * __restrict data,
                              struct timespec * __restrict                tspec)
{
	int err;

	err = etuxpt_timer_read_data_tspec(data, tspec);
	if (err) {
		if (err == -EPERM)
			return -EPERM;
		etuxpt_err(
			"failed to fetch timer event timespec expiry: %s (%d).\n",
			strerror(-err),
			-err);
		return err;
	}

	return 0;
}

static int
etuxpt_timer_read_data_msec(const struct etuxpt_timer_data * __restrict data,
                            int * __restrict                            msec)
{
	int      err;
	uint64_t val;

	err = etuxpt_timer_read_data_u64(data, &val);
	if (err) {
		if (err == -EPERM)
			return -EPERM;
		etuxpt_err(
			"failed to fetch timer milliseconds expiry: %s (%d).\n",
			strerror(-err),
			-err);
		return err;
	}

	if (val > (uint64_t)INT_MAX) {
		etuxpt_err(
			"failed to fetch timer milliseconds expiry: %s (%d).\n",
			strerror(ERANGE),
			-ERANGE);
		return -ERANGE;
	}

	*msec = (int)val;

	return 0;
}

static int
etuxpt_timer_read_data_sec(const struct etuxpt_timer_data * __restrict data,
                           int * __restrict                            sec)
{
	int      err;
	uint64_t val;

	err = etuxpt_timer_read_data_u64(data, &val);
	if (err) {
		if (err == -EPERM)
			return -EPERM;
		etuxpt_err(
			"failed to fetch timer seconds expiry: %s (%d).\n",
			strerror(err),
			-err);
		return err;
	}

	if (val > (uint64_t)INT_MAX) {
		etuxpt_err(
			"failed to fetch timer seconds expiry: %s (%d).\n",
			strerror(ERANGE),
			-ERANGE);
		return -ERANGE;
	}

	*sec = (int)val;

	return 0;
}

static int
etuxpt_timer_step_data_iter(const struct etuxpt_timer_data * __restrict data,
                            struct etuxpt_timer_event * __restrict      event)
{
	assert(data);
	assert(event);

	int                    err;
	enum etuxpt_timer_kind kind;

	err = etuxpt_timer_read_data_stamp(data, &event->stamp);
	if (err)
		return err;

	err = etuxpt_timer_read_data_task(data, &event->task);
	if (err)
		return err;

	err = etuxpt_timer_read_data_kind(data, &kind);
	if (err)
		goto free;

	switch (kind) {
	case ETUXPT_TIMER_ARM_TSPEC_KIND:
		event->process = etuxpt_timer_process_arm_tspec;
		err = etuxpt_timer_read_data_addr(data, &event->addr);
		if (!err)
			err = etuxpt_timer_read_data_expiry(data,
			                                    &event->tspec);
		break;

	case ETUXPT_TIMER_ARM_MSEC_KIND:
		event->process = etuxpt_timer_process_arm_msec;
		err = etuxpt_timer_read_data_addr(data, &event->addr);
		if (!err)
			err = etuxpt_timer_read_data_msec(data, &event->msec);
		break;

	case ETUXPT_TIMER_ARM_SEC_KIND:
		event->process = etuxpt_timer_process_arm_sec;
		err = etuxpt_timer_read_data_addr(data, &event->addr);
		if (!err)
			err = etuxpt_timer_read_data_sec(data, &event->sec);
		break;

	case ETUXPT_TIMER_CANCEL_KIND:
		event->process = etuxpt_timer_process_cancel;
		err = etuxpt_timer_read_data_addr(data, &event->addr);
		break;

	case ETUXPT_TIMER_RUN_KIND:
		event->process = etuxpt_timer_process_run;
		event->addr = 0;
		break;

	case ETUXPT_TIMER_ISSUE_TSPEC:
	case ETUXPT_TIMER_ISSUE_MSEC:
	case ETUXPT_TIMER_EXPIRE_KIND:
		event->process = etuxpt_timer_process_null;
		event->addr = 0;
		break;

	default:
		assert(0);
	}

	if (!err)
		return 0;

free:
	free(event->task);

	return err;
}

static int
etuxpt_timer_init_data_iter(const struct etuxpt_timer_data * __restrict data)
{
	assert(data);

	if (fseek(data->file,  data->start, SEEK_SET)) {
		int err = errno;

		etuxpt_err("failed to initialize data iterator: %s (%d).\n",
		           strerror(err),
		           err);
		return -err;
	}

	return 0;
}

static int
etuxpt_timer_open_data(struct etuxpt_timer_data * __restrict data,
                       const char * __restrict               pathname)
{
	assert(data);
	assert(pathname);

	char buff[5];

	data->file = fopen(pathname, "r");
	if (!data->file) {
		etuxpt_err("failed to open '%s' file: %s (%d).\n",
		           pathname,
		           strerror(errno),
		           errno);
		return EXIT_FAILURE;
	}

	if (fread(buff, sizeof(buff), 1, data->file) != 1) {
		etuxpt_err("failed to fetch data header: %s (%d).\n",
		           strerror(errno),
		           errno);
		goto close;
	}

	switch (buff[0]) {
	case 'n':
		data->endian = ETUXPT_NATIVE_ENDIAN;
		break;
	case 'l':
		data->endian = ETUXPT_LITTLE_ENDIAN;
		break;
	case 'b':
		data->endian = ETUXPT_BIG_ENDIAN;
		break;
	default:
		etuxpt_err("unexpected data endianness.\n");
		goto close;
	}

	memcpy(&data->nr, &buff[1], sizeof(data->nr));

	switch (data->endian) {
	case ETUXPT_NATIVE_ENDIAN:
		break;
	case ETUXPT_LITTLE_ENDIAN:
		data->nr = le32toh(data->nr);
		break;
	case ETUXPT_BIG_ENDIAN:
		data->nr = be32toh(data->nr);
		break;
	}

	if (!data->nr) {
		etuxpt_err("invalid number of data elements.\n");
		goto close;
	}

	data->start = sizeof(buff);

	return EXIT_SUCCESS;

close:
	fclose(data->file);

	return EXIT_FAILURE;
}

static void
etuxpt_timer_close_data(const struct etuxpt_timer_data * __restrict data)
{
	assert(data);

	fclose(data->file);
}

static
struct etuxpt_timer_event *
etuxpt_timer_load_evts(struct etuxpt_timer_data * __restrict data,
                       const char * __restrict               pathname)
{
	struct etuxpt_timer_event * evts;
	struct etuxpt_timer_event * e;
	int                         ret;

	if (etuxpt_timer_open_data(data, pathname))
		return NULL;

	evts = malloc(sizeof(*e) * data->nr);
	if (!evts)
		goto close;
	e = evts;

	ret = etuxpt_timer_init_data_iter(data);
	if (ret)
		goto free;

	while (true) {
		ret = etuxpt_timer_step_data_iter(data, e);
		if (ret)
			break;

		e++;
	}

	if (ret == -EPERM) {
		etuxpt_timer_close_data(data);
		return evts;
	}

free:
	while (e-- > evts)
		free(e->task);
	free(evts);

close:
	etuxpt_timer_close_data(data);

	etuxpt_err("failed to load timer event data.\n");

	return NULL;
}

static
void
etuxpt_timer_run_evts(struct etuxpt_timer_event * events, unsigned int nr)
{
	unsigned int    e;
	unsigned int    t;
	struct timespec start;
	struct timespec off = events[0].stamp;

	for (e = 0; e < nr; e++) {
		utime_tspec_sub(&events[e].stamp, &off);
		etuxpt_timer_build(events[e].addr);
	}

	utime_monotonic_now(&start);
	for (e = 0; e < nr; e++) {
		struct timespec wait = start;

		utime_tspec_add_clamp(&wait, &events[e].stamp);

		etuxpt_timer_clock_expect(&wait);
		events[e].process(&events[e]);
		etuxpt_timer_clock_expect(NULL);
	}

	for (t = 0; t < etuxpt_timer_cnt; t++)
		free(etuxpt_timers[t].base);

	free(etuxpt_timers);
}

static
void
etuxpt_timer_unload_evts(struct etuxpt_timer_event * events, unsigned int nr)
{
	unsigned int e;

	for (e = 0; e < nr; e++)
		free(events[e].task);

	free(events);
}

int
main(int argc, char * const argv[])
{
	struct etuxpt_timer_data    data;
	struct etuxpt_timer_event * events;
	int                         err;

	err = etuxpt_timer_setup_lttng_clock();
	if (err) {
		etuxpt_err("failed to setup LTTng clock: %s (%d).\n",
		           strerror(-err),
		           -err);
		return EXIT_FAILURE;
	}

	events = etuxpt_timer_load_evts(&data, argv[1]);
	if (!events)
		return EXIT_FAILURE;

	etuxpt_timer_run_evts(events, data.nr);

	etuxpt_timer_unload_evts(events, data.nr);

	return EXIT_SUCCESS;
}
