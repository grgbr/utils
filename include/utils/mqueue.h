#ifndef _UTILS_MQUEUE_H
#define _UTILS_MQUEUE_H

#include <utils/cdefs.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __umq_nonull(_arg_index, ...)

#define umq_assert(_expr) \
	uassert("umq", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __umq_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define umq_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

extern ssize_t
umq_validate_name(const char * __restrict name)
	__umq_nonull(1) __pure __nothrow __leaf;

static inline void __umq_nonull(2) __nothrow
umq_getattr(mqd_t mqd, struct mq_attr * attr)
{
	umq_assert((int)mqd >= 0);
	umq_assert(attr);

	int err __unused;

	err = mq_getattr(mqd, attr);
	umq_assert(!err);
}

static inline int
umq_send(mqd_t mqd, const char * data, size_t size, unsigned int prio)
{
	umq_assert((int)mqd >= 0);
	umq_assert(!size || data);
	umq_assert(prio < MQ_PRIO_MAX);

	int err;

	err = mq_send(mqd, data, size, prio);
	if (!err)
		return 0;

	umq_assert(errno != EBADF);
	umq_assert(errno != EINVAL);
	umq_assert(errno != EMSGSIZE);
	umq_assert(errno != ETIMEDOUT);

	return -errno;
}

static inline ssize_t __umq_nonull(2)
umq_recv(mqd_t mqd, char * data, size_t size, unsigned int * prio)
{
	umq_assert((int)mqd >= 0);
	umq_assert(data);
	umq_assert(size);

	int ret;

	ret = mq_receive(mqd, data, size, prio);
	if (ret >= 0) {
		umq_assert((size_t)ret <= size);
		return ret;
	}

	umq_assert(errno != EBADF);
	umq_assert(errno != EINVAL);
	umq_assert(errno != ETIMEDOUT);

	return -errno;
}

static inline mqd_t __umq_nonull(1) __nothrow
umq_open(const char * name, int flags)
{
	umq_assert(umq_validate_name(name) > 0);
	umq_assert(!(flags &
	             ~(O_RDONLY | O_WRONLY | O_RDWR | O_CLOEXEC | O_NONBLOCK)));

	mqd_t mqd;

	mqd = mq_open(name, flags);
	if ((int)mqd >= 0)
		return mqd;

	umq_assert(errno != EINVAL);
	umq_assert(errno != ENAMETOOLONG);

	return -errno;
}

#if defined(HARD_MSGMAX)
#define UMQ_MSG_MAX_NR ((unsigned int)HARD_MSGMAX)
#else  /* !defined(HARD_MSGMAX) */
#define UMQ_MSG_MAX_NR (32767U)
#endif /* defined(HARD_MSGMAX) */

static inline mqd_t __umq_nonull(1) __nothrow
umq_new(const char * name, int flags, mode_t mode, struct mq_attr * attr)
{
	umq_assert(umq_validate_name(name) > 0);
	umq_assert(!(flags &
	             ~(O_RDONLY | O_WRONLY | O_RDWR | O_CLOEXEC | O_NONBLOCK |
	               O_NOATIME | O_CREAT | O_EXCL)));
	umq_assert(!attr ||
	           ((attr->mq_maxmsg > 0) &&
	            (attr->mq_maxmsg <= (long)UMQ_MSG_MAX_NR) &&
	            (attr->mq_msgsize > 0)));

	mqd_t mqd;

	mqd = mq_open(name, flags | O_CREAT, mode, attr);
	if ((int)mqd >= 0)
		return mqd;

	umq_assert(attr || (errno != EINVAL));
	umq_assert(errno != ENAMETOOLONG);

	return -errno;
}

static inline void __nothrow
umq_close(mqd_t mqd)
{
	umq_assert((int)mqd >= 0);

	int err __unused;

	err = mq_close(mqd);
	umq_assert(!err);
}

static inline int __umq_nonull(1) __nothrow
umq_unlink(const char * name)
{
	umq_assert(umq_validate_name(name) > 0);

	if (!mq_unlink(name))
		return 0;

	umq_assert(errno != ENAMETOOLONG);

	return -errno;
}

#endif /* _UTILS_MQUEUE_H */
