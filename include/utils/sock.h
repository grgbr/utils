/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Etux.
 * Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Socket interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      10 Oct 2025
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _ETUX_SOCK_H
#define _ETUX_SOCK_H

#include <utils/fd.h>
#include <sys/types.h>
#include <sys/socket.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_sock_assert_api(_expr) \
	stroll_assert("etux:sock", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_sock_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(4, 5) __utils_nothrow
void
etux_sock_getopt(int                    fd,
                 int                    level,
                 int                    option,
                 void * __restrict      value,
                 socklen_t * __restrict size)
{
	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api(level >= 0);
	etux_sock_assert_api(option >= 0);
	etux_sock_assert_api(value);
	etux_sock_assert_api(size);
	etux_sock_assert_api(*size);

	int err __unused;

	err = getsockopt(fd, level, option, value, size);
	etux_sock_assert_api(!err);
}

static inline __utils_nonull(4) __utils_nothrow __warn_result
int
etux_sock_setopt(int                     fd,
                 int                     level,
                 int                     option,
                 const void * __restrict value,
                 socklen_t               size)
{
	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api(level >= 0);
	etux_sock_assert_api(option >= 0);
	etux_sock_assert_api(value);
	etux_sock_assert_api(size);

	if (!setsockopt(fd, level, option, value, size))
		return 0;

	etux_sock_assert_api(errno != EBADF);
	etux_sock_assert_api(errno != EFAULT);
	etux_sock_assert_api(errno != EINVAL);
	etux_sock_assert_api(errno != ENOPROTOOPT);
	etux_sock_assert_api(errno != ENOTSOCK);

	return -errno;
}

/**
 * Send a buffer over socket to its connected peer if any.
 *
 * @return A non zero number of bytes sent upon success, a negative `errno`
 *         like code otherwise.
 * @retval -EAGAIN     Underlying socket outgoing buffer full, try again later
 * @retval -EALREADY   Another (TCP) fast open is already in progress
 * @retval -EMSGSIZE   Message could not be sent atomically (MTU ?)
 * @retval -EPIPE      Remote peer consumed all of its data and closed
 * @retval -ECONNRESET Remote peer closed while there were still unhandled data
 *                     in its socket buffer
 * @retval -ENOBUFS    Underlying network interface output queue full, i.e,
 *                     transient congestion or interface stopped
 *                     (administratively ?)
 * @retval -EINTR      Interrupted by a signal before any data was transmitted
 * @retval -ENOMEM     No more memory available
 *
 * When size is zero, buff may be NULL to send a zero sized payload for datagram
 * and seqpacket sockets.
 */
static inline __warn_result
ssize_t
etux_sock_send(int fd, const void * __restrict buff, size_t size, int flags)
{
#define ETUX_SOCK_VALID_SEND_FLAGS \
	(MSG_CONFIRM | MSG_DONTROUTE | MSG_DONTWAIT | MSG_EOR | \
	 MSG_MORE | MSG_NOSIGNAL | MSG_OOB)

	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api(!buff || size);
	etux_sock_assert_api(!size || (size <= SSIZE_MAX));
	etux_sock_assert_api(!(flags & ~ETUX_SOCK_VALID_SEND_FLAGS));

	ssize_t bytes;

	bytes = send(fd, buff, size, flags);
	if (bytes >= 0)
		return bytes;

	/*
	 * Should never happen since we should have been validated by a previous
	 * connect(2) call.
	 */
	etux_sock_assert_api(errno != EACCES);

	etux_sock_assert_api(errno != EBADF);
	etux_sock_assert_api(errno != EDESTADDRREQ);
	etux_sock_assert_api(errno != EFAULT);
	etux_sock_assert_api(errno != EINVAL);
	etux_sock_assert_api(errno != EISCONN);
	etux_sock_assert_api(errno != ENOTCONN);
	etux_sock_assert_api(errno != ENOTSOCK);
	etux_sock_assert_api(errno != EOPNOTSUPP);

	return -errno;
}

/**
 * Receive from a socket connected peer if any.
 *
 * @return Number of bytes received upon success, a negative `errno` like code
 *         otherwise.
 * @retval -EAGAIN       Underlying socket incoming buffer empty, try again
 *                       later
 * @retval -ECONNREFUSED Remote peer refused to allow the network connection
 *                       (typically because it is not running the requested
 *                       service)
 * @retval -EINTR        Interrupted by a signal before any data was received
 * @retval -ENOMEM       No more memory available
 */
static inline __utils_nonull(2) __warn_result
ssize_t
etux_sock_recv(int fd, void * __restrict buff, size_t size, int flags)
{
#define ETUX_SOCK_RECV_VALID_FLAGS \
	(MSG_DONTWAIT | MSG_ERRQUEUE | MSG_OOB | MSG_PEEK | MSG_TRUNC | \
	 MSG_WAITALL)
	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api(buff);
	etux_sock_assert_api(size);
	etux_sock_assert_api(size <= SSIZE_MAX);
	etux_sock_assert_api(!(flags & ~ETUX_SOCK_RECV_VALID_FLAGS));

	ssize_t bytes;

	bytes = recv(fd, buff, size, flags);
	if (bytes >= 0)
		return bytes;

	etux_sock_assert_api(errno != EBADF);
	etux_sock_assert_api(errno != EFAULT);
	etux_sock_assert_api(errno != EINVAL);
	etux_sock_assert_api(errno != ENOTCONN);
	etux_sock_assert_api(errno != ENOTSOCK);

	return -errno;
}

static inline __utils_nonull(2) __warn_result
ssize_t
etux_sock_recvmsg(int fd, struct msghdr * __restrict msg, int flags)
{
#define ETUX_SOCK_RECVMSG_VALID_FLAGS \
	(MSG_CMSG_CLOEXEC | ETUX_SOCK_RECV_VALID_FLAGS)
	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api(msg);
	etux_sock_assert_api(!msg->msg_name || msg->msg_namelen);
	etux_sock_assert_api(msg->msg_iov || msg->msg_control);
	etux_sock_assert_api(!msg->msg_iov || msg->msg_iovlen);
	etux_sock_assert_api(!msg->msg_control || msg->msg_controllen);
	etux_sock_assert_api(!(flags & ~ETUX_SOCK_RECVMSG_VALID_FLAGS));

	ssize_t bytes;

	bytes = recvmsg(fd, msg, flags);
	if (bytes >= 0) {
		etux_sock_assert_api(msg->msg_flags &
		                     ~(MSG_EOR | MSG_TRUNC | MSG_CTRUNC |
		                       MSG_OOB | MSG_ERRQUEUE));
		return bytes;
	}

	etux_sock_assert_api(errno != EBADF);
	etux_sock_assert_api(errno != EFAULT);
	etux_sock_assert_api(errno != EINVAL);
	etux_sock_assert_api(errno != ENOTCONN);
	etux_sock_assert_api(errno != ENOTSOCK);

	return -errno;
}

static inline __utils_nonull(2) __warn_result
int
etux_sock_connect(int                                fd,
                  const struct sockaddr * __restrict peer,
                  socklen_t                          size)
{
	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api(peer);
	etux_sock_assert_api(size >= sizeof(sa_family_t));

	if (!connect(fd, peer, size))
		return 0;

	etux_sock_assert_api(errno != EAFNOSUPPORT);
	etux_sock_assert_api(errno != EBADF);
	etux_sock_assert_api(errno != EFAULT);
	etux_sock_assert_api(errno != EISCONN);
	etux_sock_assert_api(errno != ENOTSOCK);

	return -errno;
}

/**
 * Accept a socket connection.
 *
 * @return New accepted socket file descriptor if successful, a negative `errno`
 *         like code otherwise.
 * @retval -EAGAIN       No queued connection to be accepted, try again later
 * @retval -ECONNABORTED A queued connection has been aborted
 * @retval -EINTR        Interrupted by a signal before any connection could be
 *                       accepted
 * @retval -EMFILE       Maximum per-process number of opened file descriptors
 *                       reached
 * @retval -ENFILE       Maximum system number of opened file descriptors
 *                       reached
 * @retval -ENOBUFS      Not enought socket buffer memory or same as -ENOMEM
 * @retval -ENOMEM       No more (system / process) memory available
 * @retval -EPROTO       Protocol error
 * @retval -EPERM        Connection rejected by firewall rules
 *
 * @note
 * As stated into @man{accept4(2)}, additional protocol specific network errors
 * may be returned. Various Linux kernels can return other errors such as
 * `ENOSR`, `ESOCKTNOSUPPORT`, `EPROTONOSUPPORT`, `ETIMEDOUT` and `ERESTARTSYS`.
 *
 * @warning
 * As stated into @man{accept(2)} man page, Linux  @man{accept(2)} and
 * @man{accept4(2)} passes already-pending network errors on the new socket as
 * an error code from @man{accept(2)}. This behavior differs from other BSD
 * socket implementations. For reliable operation the application should detect
 * the network errors defined for the protocol after @man{accept(2)} and treat
 * them like EAGAIN by retrying. In the case of TCP/IP, these  are `ENETDOWN`,
 * `EPROTO`, `ENOPROTOOPT`, `EHOSTDOWN`, `ENONET`, `EHOSTUNREACH`, `EOPNOTSUPP`,
 * and `ENETUNREACH`.
 */
static inline __warn_result
int
etux_sock_accept(int                          fd,
                 struct sockaddr * __restrict peer,
                 socklen_t * __restrict       size,
                 int                          flags)
{
#define ETUX_SOCK_ACCEPT_VALID_FLAGS \
	(SOCK_NONBLOCK | SOCK_CLOEXEC)
#define ETUX_SOCK_ACCEPT_INVALID_FLAGS \
	(~ETUX_SOCK_ACCEPT_VALID_FLAGS)
	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api(!peer || size);
	etux_sock_assert_api(!size || (*size >= sizeof(sa_family_t)));
	etux_sock_assert_api(!(flags & ETUX_SOCK_ACCEPT_INVALID_FLAGS));

	int sk;

	sk = accept4(fd, peer, size, flags);
	if (sk >= 0) {
		etux_sock_assert_api(!size || (*size >= sizeof(sa_family_t)));
		return sk;
	}

	etux_sock_assert_api(errno != EBADF);
	etux_sock_assert_api(errno != EFAULT);
	etux_sock_assert_api(errno != EINVAL);
	etux_sock_assert_api(errno != ENOTSOCK);
	etux_sock_assert_api(errno != EOPNOTSUPP);

	return -errno;
}

static inline __utils_nothrow __warn_result
int
etux_sock_listen(int fd, int backlog)
{
	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api(backlog >= 0);

	if (!listen(fd, backlog))
		return 0;

	etux_sock_assert_api(errno != EBADF);
	etux_sock_assert_api(errno != ENOTSOCK);
	etux_sock_assert_api(errno != EOPNOTSUPP);

	return -errno;
}

#if defined(CONFIG_ETUX_NETIF)

#include <net/if.h>

static inline __utils_nonull(2) __utils_nothrow __warn_result
int
etux_sock_bind_netif(int fd, const char * __restrict iface, size_t len)
{
	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api(iface);
	etux_sock_assert_api(len);
	etux_sock_assert_api(len < IFNAMSIZ);
	etux_sock_assert_api(strnlen(iface, IFNAMSIZ) == len);

	return etux_sock_setopt(fd,
	                        SOL_SOCKET,
	                        SO_BINDTODEVICE,
	                        iface,
	                        (socklen_t)len + 1);
}

#endif /* defined(CONFIG_ETUX_NETIF) */

static inline __utils_nonull(2) __utils_nothrow __warn_result
int
etux_sock_bind(int                                fd,
               const struct sockaddr * __restrict local,
               socklen_t                          size)
{
	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api(local);
	etux_sock_assert_api(local->sa_family != AF_UNSPEC);
	etux_sock_assert_api(size);

	if (!bind(fd, local, size))
		return 0;

	etux_sock_assert_api(errno != EBADF);
	etux_sock_assert_api(errno != EINVAL);
	etux_sock_assert_api(errno != ENOTSOCK);

	return -errno;
}

static inline __utils_nothrow __warn_result
int
etux_sock_open(int domain, int type, int proto, int flags)
{
	etux_sock_assert_api(domain > AF_UNSPEC);
	etux_sock_assert_api(type > 0);
	etux_sock_assert_api(proto >= 0);
	etux_sock_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	int fd;

	fd = socket(domain, type | flags, proto);
	if (fd >= 0)
		return fd;

	etux_sock_assert_api(errno != EINVAL);
	etux_sock_assert_api(errno != EPROTONOSUPPORT);

	return -errno;
}

/**
 * Shutdown part(s) of a full-duplex socket.
 */
static inline __utils_nothrow
void
etux_sock_shutdown(int fd, int how)
{
	etux_sock_assert_api(fd >= 0);
	etux_sock_assert_api((how == SHUT_RD) ||
	                     (how == SHUT_WR) ||
	                     (how == SHUT_RDWR));

	int err __unused;

	err = shutdown(fd, how);
	etux_sock_assert_api(!err);
}

/**
 * Close a socket.
 *
 * @return 0 if succesful, a negative `errno` like code otherwise.
 * @retval -EINTR A signal raised during closure
 * @retval -EIO   An I/O error occured
 *
 * On Linux, in case of error, *DO NOT* ever retry to close the same file
 * descriptor again. This is useless and the error code is returned for
 * informational purpose only.
 *
 * See section *Dealing with error returns from close()* of @man{close(2)} for
 * further details.
 */
static inline
int
etux_sock_close(int fd)
{
	int ret;

	ret = ufd_close(fd);

	etux_sock_assert_api(ret != -ENOSPC);
	etux_sock_assert_api(ret != -EDQUOT);

	return ret;
}

#endif /* _ETUX_SOCK_H */
