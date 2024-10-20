/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * POSIX message queue interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      13 Jan 2022
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_MQUEUE_H
#define _UTILS_MQUEUE_H

#include <utils/cdefs.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define umq_assert_api(_expr) \
	stroll_assert("utils:umq", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define umq_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#define umq_assert_intern(_expr) \
	stroll_assert("utils:umq", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define umq_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

extern ssize_t
umq_validate_name(const char * __restrict name)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf;

#if defined(CONFIG_UTILS_ASSERT_API)

static inline __utils_nonull(2) __utils_nothrow
void
umq_getattr(mqd_t mqd, struct mq_attr * attr)
{
	umq_assert_api((int)mqd >= 0);
	umq_assert_api(attr);

	umq_assert_api(!mq_getattr(mqd, attr));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(2) __utils_nothrow
void
umq_getattr(mqd_t mqd, struct mq_attr * attr)
{
	mq_getattr(mqd, attr);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static inline
int
umq_send(mqd_t                   mqd,
         const char * __restrict data,
         size_t                  size,
         unsigned int            prio)
{
	umq_assert_api((int)mqd >= 0);
	umq_assert_api(!size || data);
	umq_assert_api(prio < MQ_PRIO_MAX);

	int err;

	err = mq_send(mqd, data, size, prio);
	if (!err)
		return 0;

	umq_assert_api(errno != EBADF);
	umq_assert_api(errno != EINVAL);
	umq_assert_api(errno != EMSGSIZE);
	umq_assert_api(errno != ETIMEDOUT);

	return -errno;
}

static inline __utils_nonull(2)
ssize_t
umq_recv(mqd_t                     mqd,
         char * __restrict         data,
         size_t                    size,
         unsigned int * __restrict prio)
{
	umq_assert_api((int)mqd >= 0);
	umq_assert_api(data);
	umq_assert_api(size);

	ssize_t ret;

	ret = mq_receive(mqd, data, size, prio);
	if (ret >= 0) {
		umq_assert_api((size_t)ret <= size);
		return ret;
	}

	umq_assert_api(errno != EBADF);
	umq_assert_api(errno != EINVAL);
	umq_assert_api(errno != ETIMEDOUT);

	return -errno;
}

static inline __utils_nonull(1) __utils_nothrow
mqd_t
umq_open(const char * __restrict name, int flags)
{
	umq_assert_api(umq_validate_name(name) > 0);
	umq_assert_api(!(flags & ~(O_RDONLY | O_WRONLY | O_RDWR |
	                           O_CLOEXEC | O_NONBLOCK)));

	mqd_t mqd;

	mqd = mq_open(name, flags);
	if ((int)mqd >= 0)
		return mqd;

	umq_assert_intern(errno != EINVAL);
	umq_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

#if defined(HARD_MSGMAX)
#define UMQ_MSG_MAX_NR ((unsigned int)HARD_MSGMAX)
#else  /* !defined(HARD_MSGMAX) */
#define UMQ_MSG_MAX_NR (32767U)
#endif /* defined(HARD_MSGMAX) */

static inline __utils_nonull(1) __utils_nothrow
mqd_t
umq_new(const char * __restrict     name,
        int                         flags,
        mode_t                      mode,
        struct mq_attr * __restrict attr)
{
	umq_assert_api(umq_validate_name(name) > 0);
	umq_assert_api(!(flags & ~(O_RDONLY | O_WRONLY | O_RDWR |
	                           O_CLOEXEC | O_NONBLOCK | O_NOATIME |
	                           O_CREAT | O_EXCL)));
	umq_assert_api(!attr ||
	               ((attr->mq_maxmsg > 0) &&
	                (attr->mq_maxmsg <= (long)UMQ_MSG_MAX_NR) &&
	                (attr->mq_msgsize > 0)));

	mqd_t mqd;

	mqd = mq_open(name, flags | O_CREAT, mode, attr);
	if ((int)mqd >= 0)
		return mqd;

	umq_assert_api(attr || (errno != EINVAL));
	umq_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

#if defined(CONFIG_UTILS_ASSERT_API)

static inline __utils_nothrow
void
umq_close(mqd_t mqd)
{
	umq_assert_api((int)mqd >= 0);

	umq_assert_api(!mq_close(mqd));
}

#else /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nothrow
void
umq_close(mqd_t mqd)
{
	mq_close(mqd);
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(1) __utils_nothrow
int
umq_unlink(const char * __restrict name)
{
	umq_assert_api(umq_validate_name(name) > 0);

	if (!mq_unlink(name))
		return 0;

	umq_assert_intern(errno != ENAMETOOLONG);

	return -errno;
}

#endif /* _UTILS_MQUEUE_H */
