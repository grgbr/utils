/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/unsk.h"
#include <stdlib.h>

#define UNSK_NAMED_PATH_MAX     (sizeof_member(struct sockaddr_un, sun_path))
#define UNSK_ABSTRACT_PATH_LEN  (5U)
#define UNSK_ABSTRACT_PATH_MAX  (UNSK_ABSTRACT_PATH_LEN + 1)
#define UNSK_ABSTRACT_ADDR_LEN \
	(sizeof(sa_family_t) + 1 + UNSK_ABSTRACT_PATH_LEN)

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define unsk_assert_intern(_expr) \
	stroll_assert("utils:unsk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define unsk_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

/******************************************************************************
 * low-level UNIX socket wrappers
 ******************************************************************************/

ssize_t
unsk_validate_named_path(const char * __restrict path)
{
	unsk_assert_api(path);

	ssize_t len;

	len = upath_validate_path(path, UNSK_NAMED_PATH_MAX);
	unsk_assert_intern(len);

	return len;
}

socklen_t
unsk_make_sized_addr(struct sockaddr_un * __restrict addr,
                     const char * __restrict         path,
                     size_t                          len)
{
	unsk_assert_api(addr);
	unsk_assert_api(path);
	unsk_assert_api(unsk_validate_named_path(path) == (ssize_t)len);

	addr->sun_family = AF_UNIX;
	memcpy(addr->sun_path, path, len + 1);

	return (socklen_t)(offsetof(typeof(*addr), sun_path) + len + 1);
}

socklen_t
unsk_make_named_addr(struct sockaddr_un * __restrict addr,
                     const char * __restrict         path)
{
	unsk_assert_api(addr);
	unsk_assert_api(path);
	unsk_assert_api(unsk_validate_named_path(path) > 0);

	return unsk_make_sized_addr(addr,
	                            path,
	                            strnlen(path, sizeof(addr->sun_path)));
}

#if defined(CONFIG_UTILS_ASSERT_API)

ssize_t
unsk_send_dgram_msg(int fd, const struct msghdr * __restrict msg, int flags)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(msg);
	unsk_assert_api(msg->msg_namelen > (sizeof(sa_family_t) + 1));
	unsk_assert_api(msg->msg_name);
	unsk_assert_api(msg->msg_iovlen || msg->msg_controllen);
	unsk_assert_api(!msg->msg_iovlen || msg->msg_iov);
	unsk_assert_api(!msg->msg_controllen || msg->msg_control);
	unsk_assert_api(!(flags & ~(MSG_DONTWAIT | MSG_MORE)));

	const struct sockaddr_un * addr = (struct sockaddr_un *)msg->msg_name;
	unsigned int               v;
	ssize_t                    ret;

	/*
	 * Make destination address mandatory and reject the unamed socket
	 * space.
	 */
	unsk_assert_api(addr->sun_family == AF_UNIX);
	if (addr->sun_path[0])
		unsk_assert_api(upath_validate_path(addr->sun_path,
		                                    UNSK_NAMED_PATH_MAX) > 0);

	/* Make sure I/O vectors are properly filled. */
	for (v = 0; v < msg->msg_iovlen; v++) {
		const struct iovec * vec = &msg->msg_iov[v];

		unsk_assert_api(vec->iov_base);
		unsk_assert_api(vec->iov_len);
		unsk_assert_api(vec->iov_len <= UNSK_BUFF_SIZE_MAX);
	}

	ret = sendmsg(fd, msg, flags);
	if (ret > 0)
		return ret;
	else if (!ret)
		return -EAGAIN;

	unsk_assert_api(errno != EALREADY);
	unsk_assert_api(errno != EBADF);
	unsk_assert_api(errno != ECONNRESET);
	unsk_assert_api(errno != EDESTADDRREQ);
	unsk_assert_api(errno != EFAULT);
	unsk_assert_api(errno != EINVAL);
	unsk_assert_api(errno != EISCONN);
	unsk_assert_api(errno != EMSGSIZE);
	unsk_assert_api(errno != ENOBUFS);
	unsk_assert_api(errno != ENOTCONN);
	unsk_assert_api(errno != ENOTSOCK);
	unsk_assert_api(errno != EOPNOTSUPP);
	unsk_assert_api(errno != EPIPE);
	unsk_assert_api(errno != ETOOMANYREFS);

	return -errno;
}

ssize_t
unsk_recv_dgram_msg(int fd, struct msghdr * __restrict msg, int flags)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(msg);
	unsk_assert_api(!msg->msg_namelen ||
	                ((msg->msg_namelen == sizeof(struct sockaddr_un)) &&
	                 msg->msg_name));
	unsk_assert_api(msg->msg_iovlen || msg->msg_controllen);
	unsk_assert_api(!msg->msg_iovlen || msg->msg_iov);
	unsk_assert_api(!msg->msg_controllen || msg->msg_control);
	unsk_assert_api(!(flags & ~(MSG_CMSG_CLOEXEC | MSG_DONTWAIT)));

	unsigned int v;
	ssize_t      ret;

	/* Make sure I/O vectors are properly filled. */
	for (v = 0; v < msg->msg_iovlen; v++) {
		const struct iovec * vec = &msg->msg_iov[v];

		unsk_assert_api(vec->iov_base);
		unsk_assert_api(vec->iov_len);
		unsk_assert_api(vec->iov_len <= UNSK_BUFF_SIZE_MAX);
	}

	ret = recvmsg(fd, msg, flags);
	if (ret > 0)
		return ret;
	else if (!ret)
		return -EAGAIN;

	unsk_assert_api(errno != EBADF);
	unsk_assert_api(errno != ECONNREFUSED);
	unsk_assert_api(errno != EFAULT);
	unsk_assert_api(errno != EINVAL);
	unsk_assert_api(errno != ENOTCONN);
	unsk_assert_api(errno != ENOTSOCK);

	return -errno;
}

int
unsk_bind(int fd, const struct sockaddr_un * __restrict addr, socklen_t size)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(addr);
	unsk_assert_api(addr->sun_family == AF_UNIX);
	unsk_assert_api(size >= sizeof(sa_family_t));

	if (!bind(fd, (const struct sockaddr *)addr, size))
		return 0;

	unsk_assert_api(errno != EBADF);
	unsk_assert_api(errno != EINVAL);
	unsk_assert_api(errno != ENOTSOCK);
	unsk_assert_api(errno != EADDRNOTAVAIL);
	unsk_assert_api(errno != EFAULT);
	unsk_assert_api(errno != ENAMETOOLONG);

	return -errno;
}

int
unsk_open(int type, int flags)
{
	unsk_assert_api((type == SOCK_DGRAM) ||
	                (type == SOCK_STREAM) ||
	                (type == SOCK_SEQPACKET));
	unsk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	int fd;

	fd = socket(AF_UNIX, type | flags, 0);
	if (fd < 0) {
		unsk_assert_api(errno != EAFNOSUPPORT);
		unsk_assert_api(errno != EINVAL);
		unsk_assert_api(errno != EPROTONOSUPPORT);

		return -errno;
	}

	return fd;
}

int
unsk_close(int fd)
{
	unsk_assert_api(fd >= 0);

	int err;

	err = ufd_close(fd);

	unsk_assert_api(err != -ENOSPC);
	unsk_assert_api(err != -EDQUOT);

	return err;
}

int
unsk_unlink(const char * __restrict path)
{
	unsk_assert_api(upath_validate_path(path, UNSK_NAMED_PATH_MAX) > 0);

	if (!upath_unlink(path) || (errno == ENOENT))
		return 0;

	unsk_assert_api(errno != EFAULT);
	unsk_assert_api(errno != ENAMETOOLONG);

	return -errno;
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

int
unsk_connect_dgram(int                             fd,
                   const char * __restrict         peer_path,
                   struct sockaddr_un * __restrict peer_addr,
                   socklen_t * __restrict          addr_len)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(!unsk_is_named_path_ok(peer_path));

	const struct sockaddr_un local = { .sun_family = AF_UNIX };
	int                      err;

	/*
	 * Explicitly bind to instantiate a local abstract UNIX socket.
	 *
	 * See description of "abstract" sockets in section "Address format" of
	 * unix(7) man page.
	 */
	err = unsk_bind(fd, &local, sizeof(sa_family_t));
	if (err) {
		unsk_assert_api(err != -EADDRINUSE);
		unsk_assert_api(err != -ELOOP);
		unsk_assert_api(err != -ENOENT);
		unsk_assert_api(err != -ENOTDIR);
		unsk_assert_api(err != -EROFS);

		return err;
	}

	/* Setup peer address to send messages to. */
	*addr_len = unsk_make_named_addr(peer_addr, peer_path);

	return 0;
}

/******************************************************************************
 * UNIX socket buffer and queue handling
 ******************************************************************************/

static __utils_nothrow __warn_result
struct unsk_buff *
unsk_buff_alloc(size_t size)
{
	unsk_assert_intern(size > sizeof(struct unsk_buff));

	return malloc(size);
}

static __utils_nonull(1) __utils_nothrow
void
unsk_buff_free(struct unsk_buff * buff)
{
	unsk_assert_intern(buff);

	free(buff);
}

static __utils_nonull(1) __utils_pure __utils_nothrow __returns_nonull
struct unsk_buff *
unsk_buffq_peek(const struct stroll_slist * __restrict list)
{
	return stroll_slist_first_entry(list, struct unsk_buff, node);
}

struct unsk_buff *
unsk_buffq_peek_busy(const struct unsk_buffq * __restrict buffq)
{
	unsk_assert_api(buffq);

	return unsk_buffq_peek(&buffq->busy);
}

struct unsk_buff *
unsk_buffq_peek_free(const struct unsk_buffq * __restrict buffq)
{
	unsk_assert_api(buffq);

	return unsk_buffq_peek(&buffq->free);
}

static __utils_nonull(1, 2) __utils_nothrow
void
unsk_buffq_requeue(struct stroll_slist * __restrict list,
                   struct unsk_buff * __restrict    buff)
{
	unsk_assert_intern(list);
	unsk_assert_intern(buff);

	stroll_slist_nqueue_front(list, &buff->node);
}

void
unsk_buffq_nqueue_busy(struct unsk_buffq * __restrict buffq,
                       struct unsk_buff * __restrict  buff)
{
	unsk_assert_api(buffq);
	unsk_assert_api(buff);

	stroll_slist_nqueue_back(&buffq->busy, &buff->node);
}

void
unsk_buffq_requeue_busy(struct unsk_buffq * __restrict buffq,
                        struct unsk_buff * __restrict  buff)
{
	unsk_assert_api(buffq);

	unsk_buffq_requeue(&buffq->busy, buff);
}

static __utils_nonull(1) __utils_nothrow __returns_nonull
struct unsk_buff *
unsk_buffq_xtract(struct stroll_slist * __restrict list)
{
	return stroll_slist_entry(stroll_slist_dqueue_front(list),
	                          struct unsk_buff,
	                          node);
}

struct unsk_buff *
unsk_buffq_dqueue_busy(struct unsk_buffq * __restrict buffq)
{
	unsk_assert_api(buffq);

	return unsk_buffq_xtract(&buffq->busy);
}

struct unsk_buff *
unsk_buffq_dqueue_free(struct unsk_buffq * __restrict buffq)
{
	unsk_assert_api(buffq);

	return unsk_buffq_xtract(&buffq->free);
}

void
unsk_buffq_release(struct unsk_buffq * __restrict buffq,
                   struct unsk_buff * __restrict  buff)
{
	unsk_assert_api(buffq);

	unsk_buffq_requeue(&buffq->free, buff);
}

int
unsk_buffq_init(struct unsk_buffq * __restrict buffq,
                size_t                         buff_desc_sz,
                size_t                         max_data_sz,
                unsigned int                   max_buff_nr)
{
	unsk_assert_api(buffq);
	unsk_assert_api(buff_desc_sz >= sizeof(struct unsk_buff));
	unsk_assert_api(max_data_sz <= UNSK_BUFF_SIZE_MAX);
	unsk_assert_api(max_data_sz);
	unsk_assert_api(max_data_sz <= UNSK_BUFF_SIZE_MAX);
	unsk_assert_api(max_buff_nr);
	unsk_assert_api(max_buff_nr <= UNSK_BUFF_COUNT_MAX);

	size_t sz = buff_desc_sz + max_data_sz;
	int    err;

	stroll_slist_init(&buffq->busy);
	stroll_slist_init(&buffq->free);

	while (max_buff_nr--) {
		struct unsk_buff * buff;

		buff = unsk_buff_alloc(sz);
		if (!buff)
			goto free;

		stroll_slist_nqueue_back(&buffq->free, &buff->node);
	}

	return 0;

free:
	err = -errno;

	while (!stroll_slist_empty(&buffq->free))
		unsk_buff_free(unsk_buffq_xtract(&buffq->free));

	return err;
}

void
unsk_buffq_fini(struct unsk_buffq * __restrict buffq)
{
	unsk_assert_api(buffq);

	while (!stroll_slist_empty(&buffq->busy))
		unsk_buff_free(unsk_buffq_xtract(&buffq->busy));

	while (!stroll_slist_empty(&buffq->free))
		unsk_buff_free(unsk_buffq_xtract(&buffq->free));
}

/******************************************************************************
 * Service / server side UNIX socket handling
 ******************************************************************************/

int
unsk_dgram_svc_send(const struct unsk_svc * __restrict    sock,
                    const void * __restrict               data,
                    size_t                                size,
                    const struct sockaddr_un * __restrict peer,
                    int                                   flags)
{
	unsk_assert_api(sock);
	unsk_assert_api(sock->fd >= 0);
	unsk_assert_api(data);
	unsk_assert_api(size);
	unsk_assert_api(size <= UNSK_BUFF_SIZE_MAX);
	unsk_assert_api(peer);
	unsk_assert_api(peer->sun_family == AF_UNIX);
	unsk_assert_api(!peer->sun_path[0]);
	unsk_assert_api(!(flags & ~(MSG_DONTWAIT | MSG_MORE)));

STROLL_IGNORE_WARN("-Wcast-qual")
	const struct iovec  vec = {
		.iov_base = (void *)data,
		.iov_len  = size
	};
	const struct msghdr msg = {
		.msg_name       = (struct sockaddr *)peer,
		.msg_namelen    = UNSK_ABSTRACT_ADDR_LEN,
		.msg_iov        = (struct iovec *)&vec,
		.msg_iovlen     = 1,
		0,
	};
STROLL_RESTORE_WARN
	ssize_t             ret;

	ret = unsk_send_dgram_msg(sock->fd, &msg, flags);
	if (ret > 0) {
		/* Sending a single datagram is an atomic operation. */
		unsk_assert_intern((size_t)ret == size);
		return 0;
	}
	else if ((ret == -EAGAIN) || (ret == -EINTR))
		return (int)ret;

	unsk_assert_intern(ret);
	unsk_assert_intern(ret != -EACCES); /* Cannot happen when sending to
	                                       UNIX abstract sockets. */
	return (int)ret;
}

ssize_t
unsk_dgram_svc_recv(const struct unsk_svc * __restrict sock,
                    void * __restrict                  data,
                    size_t                             size,
                    struct sockaddr_un *               peer,
                    struct ucred * __restrict          creds,
                    int                                flags)
{
	unsk_assert_api(sock);
	unsk_assert_api(sock->fd >= 0);
	unsk_assert_api(data);
	unsk_assert_api(size);
	unsk_assert_api(size <= UNSK_BUFF_SIZE_MAX);
	unsk_assert_api(peer);
	unsk_assert_api(creds);
	unsk_assert_api(!(flags & ~(MSG_CMSG_CLOEXEC | MSG_DONTWAIT)));

	const struct iovec     vec = {
		.iov_base = data,
		.iov_len  = size
	};
	union unsk_creds       anc;
STROLL_IGNORE_WARN("-Wcast-qual")
	struct msghdr          msg = {
		.msg_name       = peer,
		.msg_namelen    = sizeof(*peer),
		.msg_iov        = (struct iovec *)&vec,
		.msg_iovlen     = 1,
		.msg_control    = anc.buff,
		.msg_controllen = sizeof(anc.buff),
		0,
	};
STROLL_RESTORE_WARN
	ssize_t                ret;

	ret = unsk_recv_dgram_msg(sock->fd, &msg, flags);
	if (ret > 0) {
		const struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);

		if ((msg.msg_namelen != UNSK_ABSTRACT_ADDR_LEN) ||
		    peer->sun_path[0])
			return -EADDRNOTAVAIL;

		unsk_assert_intern(!(msg.msg_flags & MSG_EOR));
		unsk_assert_intern(!(msg.msg_flags & MSG_OOB));
		unsk_assert_intern(!(msg.msg_flags & MSG_ERRQUEUE));
		if (msg.msg_flags & (MSG_TRUNC | MSG_CTRUNC))
			return -EMSGSIZE;

		if (!cmsg ||
		    (cmsg->cmsg_level != SOL_SOCKET) ||
		    (cmsg->cmsg_type != SCM_CREDENTIALS) ||
		    (cmsg->cmsg_len != CMSG_LEN(sizeof(*creds))))
			return -EPROTO;

		*creds = *(const struct ucred *)CMSG_DATA(cmsg);

		return ret;
	}
	else if ((ret == -EAGAIN) || (ret == -EINTR))
		return ret;

	unsk_assert_intern(ret);

	return ret;
}

int
unsk_svc_bind(struct unsk_svc * __restrict sock, const char * __restrict path)
{
	unsk_assert_api(sock);
	unsk_assert_api(sock->fd >= 0);
	unsk_assert_api(!unsk_svc_is_path_ok(path));

	int                  err;
	size_t               sz;
	struct sockaddr_un * addr = &sock->local;
	const int            cred = 1;

	/* Build local listen address. */
	sz = unsk_make_named_addr(addr, path);

	/*
	 * Remove local filesystem pathname if existing.
	 *
	 * This is required since binding a named UNIX socket to a filesystem
	 * entry that already exists will fail with EADDRINUSE error code
	 * (AF_UNIX sockets do not support the SO_REUSEADDR socket option).
	 */
	err = unsk_unlink(path);
	if (err)
		return err;

	/*
	 * Bind to the given local filesystem pathname.
	 *
	 * This will effectively create the filesystem entry according to
	 * current process priviledges.
	 * See "Pathname socket ownership and permissions" section of unix(7)
	 * man page.
	 */
	err = unsk_bind(sock->fd,
	                addr,
	                (socklen_t)(offsetof(typeof(*addr), sun_path) + sz));
	if (err)
		return err;

	/* Enable receiving of peer socket credentials. */
	unsk_setsockopt(sock->fd, SO_PASSCRED, &cred, sizeof(cred));

	return 0;
}

int
unsk_dgram_svc_open(struct unsk_svc * __restrict sock, int flags)
{
	unsk_assert_api(sock);

	int ret;

	ret = unsk_open(SOCK_DGRAM, flags);
	if (ret < 0)
		return ret;

	sock->fd = ret;
	sock->local.sun_path[0] = '\0';

	return 0;
}

int
unsk_svc_close(const struct unsk_svc * __restrict sock)
{
	unsk_assert_api(sock);

	const char * path = sock->local.sun_path;

	unsk_close(sock->fd);

	if (path[0] && unlink(path)) {
		if (errno == ENOENT)
			return 0;

		unsk_assert_intern(errno != EFAULT);
		unsk_assert_intern(errno != ENAMETOOLONG);

		return -errno;
	}

	return 0;
}

/******************************************************************************
 * Client side UNIX socket handling
 ******************************************************************************/

int
unsk_dgram_clnt_send(const struct unsk_clnt * __restrict sock,
                     const void * __restrict             data,
                     size_t                              size,
                     int                                 flags)
{
	unsk_assert_api(sock);
	unsk_assert_api(sock->fd >= 0);
	unsk_assert_api(data);
	unsk_assert_api(size);
	unsk_assert_api(size <= UNSK_BUFF_SIZE_MAX);
	unsk_assert_api(sock->peer.sun_family == AF_UNIX);
	unsk_assert_api(sock->peer.sun_path[0]);
	unsk_assert_api(!(flags & ~(MSG_DONTWAIT | MSG_MORE)));

STROLL_IGNORE_WARN("-Wcast-qual")
	const struct iovec  vec = {
		.iov_base = (void *)data,
		.iov_len  = size
	};
	const struct msghdr msg = {
		.msg_name       = (struct sockaddr *)&sock->peer,
		.msg_namelen    = sock->peer_sz,
		.msg_iov        = (struct iovec *)&vec,
		.msg_iovlen     = 1,
		.msg_control    = (void *)sock->creds.buff,
		.msg_controllen = sizeof(sock->creds.buff),
		0,
	};
STROLL_RESTORE_WARN
	ssize_t             ret;

	ret = unsk_send_dgram_msg(sock->fd, &msg, flags);
	if (ret > 0) {
		/* Sending a single datagram is an atomic operation. */
		unsk_assert_api((size_t)ret == size);
		return 0;
	}
	else if ((ret == -EAGAIN) || (ret == -EINTR))
		return (int)ret;

	unsk_assert_api(ret);

	return (int)ret;
}

ssize_t
unsk_dgram_clnt_recv(const struct unsk_clnt * __restrict sock,
                     void * __restrict                   data,
                     size_t                              size,
                     int                                 flags)
{
	unsk_assert_api(sock);
	unsk_assert_api(sock->fd >= 0);
	unsk_assert_api(data);
	unsk_assert_api(size);
	unsk_assert_api(size <= UNSK_BUFF_SIZE_MAX);
	unsk_assert_api(!(flags & ~(MSG_CMSG_CLOEXEC | MSG_DONTWAIT)));

	const struct iovec vec = {
		.iov_base = data,
		.iov_len  = size
	};
	struct sockaddr_un peer;
STROLL_IGNORE_WARN("-Wcast-qual")
	struct msghdr      msg = {
		.msg_name    = &peer,
		.msg_namelen = sizeof(peer),
		.msg_iov     = (struct iovec *)&vec,
		.msg_iovlen  = 1,
		0,
	};
STROLL_RESTORE_WARN
	ssize_t            ret;

	ret = unsk_recv_dgram_msg(sock->fd, &msg, flags);
	if (ret > 0) {
		if ((msg.msg_namelen != sock->peer_sz) ||
		    memcmp(&peer, &sock->peer, sock->peer_sz))
			return -EADDRNOTAVAIL;

		unsk_assert_intern(!(msg.msg_flags & MSG_EOR));
		unsk_assert_intern(!(msg.msg_flags & MSG_OOB));
		unsk_assert_intern(!(msg.msg_flags & MSG_ERRQUEUE));
		if (msg.msg_flags & (MSG_TRUNC | MSG_CTRUNC))
			return -EMSGSIZE;

		return ret;
	}
	else if ((ret == -EAGAIN) || (ret == -EINTR))
		return ret;

	unsk_assert_intern(ret);

	return ret;
}

int
unsk_dgram_clnt_connect(struct unsk_clnt * __restrict sock,
                        const char * __restrict       path)
{
	unsk_assert_api(sock);
	unsk_assert_api(sock->fd >= 0);

	int              err;
	struct cmsghdr * cmsg = &sock->creds.head;
	struct ucred *   creds = (struct ucred *)CMSG_DATA(cmsg);

	err = unsk_connect_dgram(sock->fd, path, &sock->peer, &sock->peer_sz);
	if (err)
		return err;

	/* Setup credentials ancillary message to send messages with. */
	memset(cmsg, 0, sizeof(*cmsg));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_CREDENTIALS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(struct ucred));
	creds->pid = getpid();
	creds->uid = geteuid();
	creds->gid = getegid();

	return 0;
}

int
unsk_dgram_clnt_open(struct unsk_clnt * __restrict sock, int flags)
{
	unsk_assert_api(sock);

	int ret;

	ret = unsk_open(SOCK_DGRAM, flags);
	if (ret < 0)
		return ret;

	sock->fd = ret;

	return 0;
}

void
unsk_clnt_close(const struct unsk_clnt * __restrict sock)
{
	unsk_assert_api(sock);

	unsk_close(sock->fd);
}

/******************************************************************************
 * Asynchronous service / server side UNIX socket handling
 ******************************************************************************/

#if defined(CONFIG_UTILS_POLL_UNSK)

int
unsk_dgram_async_svc_recv(const struct unsk_async_svc * __restrict svc,
                          struct unsk_dgram_buff * __restrict      buff,
                          size_t                                   size,
                          struct ucred * __restrict                creds,
                          int                                      flags)
{
	unsk_assert_api(svc);
	unsk_assert_api(!flags || (flags == MSG_CMSG_CLOEXEC));

	ssize_t ret;

	ret = unsk_dgram_svc_recv(&svc->sock,
	                          buff->data,
	                          size,
	                          &buff->peer,
	                          creds,
	                          flags);
	unsk_assert_intern(ret);
	if (ret > 0) {
		buff->unsk.bytes = (size_t)ret;
		return 0;
	}

	return (int)ret;
}

int
unsk_dgram_async_svc_open(struct unsk_async_svc * __restrict svc,
                          const char * __restrict            path,
                          int                                sock_flags,
                          const struct upoll * __restrict    poller,
                          uint32_t                           poll_flags,
                          upoll_dispatch_fn *                dispatch)
{
	unsk_assert_api(svc);
	unsk_assert_api(!sock_flags || (sock_flags == SOCK_CLOEXEC));

	int err;

	err = unsk_dgram_svc_open(&svc->sock, SOCK_NONBLOCK | sock_flags);
	if (err)
		return err;

	err = unsk_svc_bind(&svc->sock, path);
	if (err)
		goto close;

	svc->work.dispatch = dispatch;
	err = upoll_register(poller, svc->sock.fd, poll_flags, &svc->work);
	if (err)
		goto close;

	return 0;

close:
	unsk_svc_close(&svc->sock);

	return err;
}

int
unsk_dgram_async_svc_close(struct unsk_async_svc * __restrict svc,
                           const struct upoll * __restrict    poller)
{
	upoll_unregister(poller, svc->sock.fd);

	return unsk_svc_close(&svc->sock);
}

#endif /* defined(CONFIG_UTILS_POLL_UNSK) */
