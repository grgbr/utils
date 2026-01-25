/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * UNIX socket interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      04 Oct 2021
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_UNSK_H
#define _UTILS_UNSK_H

#include <utils/sock.h>
#include <stroll/slist.h>
#include <utils/poll.h>
#include <sys/un.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define unsk_assert_api(_expr) \
	stroll_assert("utils:unsk", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define unsk_assert_api(_expr) \
	do { } while (0)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

/******************************************************************************
 * low-level UNIX socket wrappers
 ******************************************************************************/

#define UNSK_NAMED_PATH_MAX \
	(sizeof_member(struct sockaddr_un, sun_path))

#define UNSK_NAMED_ADDR(_path) \
	{ .sun_family = AF_UNIX, .sun_path = _path }

#define UNSK_NAMED_ADDR_LEN(_path) \
	(offsetof(struct sockaddr_un, sun_path) + sizeof(_path))

extern ssize_t
unsk_validate_named_path(const char * __restrict path)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__warn_result
	__export_public;

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
int
unsk_is_named_path_ok(const char * __restrict path)
{
	unsk_assert_api(path);

	ssize_t len;

	len = unsk_validate_named_path(path);

	return (len < 0) ? (int)len : 0;
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
unsk_is_named_addr(const struct sockaddr_un * __restrict addr,
                   socklen_t                             size)
{
	unsk_assert_api(addr);
	unsk_assert_api(size >= sizeof(sa_family_t));

	if ((size <= (sizeof(sa_family_t) + 1)) ||
	    (addr->sun_path[0] == '\0'))
		return false;

	unsk_assert_api((sizeof(sa_family_t) +
	                 (size_t)unsk_validate_named_path(addr->sun_path) +
	                 1) == size);

	return true;
}

extern const char *
unsk_make_addr_string(
	char                                  string[UNSK_NAMED_PATH_MAX],
	const struct sockaddr_un * __restrict addr,
	socklen_t                             length)
	__utils_nonull(1, 2)
	__utils_nothrow
	__leaf
	__warn_result
	__export_public;

extern socklen_t
unsk_make_sized_addr(struct sockaddr_un * __restrict addr,
                     const char * __restrict         path,
                     size_t                          len)
	__utils_nonull(1, 2) __utils_nothrow __warn_result __export_public;

extern socklen_t
unsk_make_named_addr(struct sockaddr_un * __restrict addr,
                     const char * __restrict         path)
	__utils_nonull(1, 2) __utils_nothrow __warn_result __export_public;

static inline __utils_nonull(3, 4) __utils_nothrow
void
unsk_getsockopt(int                    fd,
                int                    option,
                void * __restrict      value,
                socklen_t * __restrict size)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(option >= 0);
	unsk_assert_api(value);
	unsk_assert_api(size);
	unsk_assert_api(*size);

	etux_sock_getopt(fd, SOL_SOCKET, option, value, size);
}

static inline __utils_nonull(3) __utils_nothrow
void
unsk_setsockopt(int                     fd,
                int                     option,
                const void * __restrict value,
                socklen_t               size)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(option >= 0);
	unsk_assert_api(value);
	unsk_assert_api(size);

	int err __unused;

	err = etux_sock_setopt(fd, SOL_SOCKET, option, value, size);
	unsk_assert_api(!err);
}

/*
 * When size is zero, buff may be NULL to send a zero sized payload for datagram
 * and seqpacket sockets.
 *
 * As stated into unix(7), unix sockets don't support the transmission of
 * out-of-band data. However, Linux seems to support it since 2021: also allow
 * passing the MSG_OOB flag.
 */
static inline __warn_result
ssize_t
unsk_send(int fd, const void * __restrict buff, size_t size, int flags)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(!buff || size);
	unsk_assert_api(!size || (size <= SSIZE_MAX));
	unsk_assert_api(!(flags & ~(MSG_DONTWAIT | MSG_EOR | MSG_MORE |
		                    MSG_NOSIGNAL | MSG_OOB)));

	return etux_sock_send(fd, buff, size, flags);
}

#if defined(CONFIG_UTILS_ASSERT_API)

extern ssize_t
unsk_send_dgram_msg(int fd, const struct msghdr * __restrict msg, int flags)
	__utils_nonull(2) __warn_result __export_public;

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(2) __warn_result
ssize_t
unsk_send_dgram_msg(int fd, const struct msghdr * __restrict msg, int flags)
{
	ssize_t ret;

	ret = sendmsg(fd, msg, flags);
	if (ret > 0)
		return ret;
	else if (!ret)
		return -EAGAIN;

	return -errno;
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

/*
 * As stated into unix(7), unix sockets don't support the transmission of
 * out-of-band data. However, Linux seems to support it since 2021: also allow
 * passing the MSG_OOB flag.
 */
static inline __utils_nonull(2) __warn_result
ssize_t
unsk_recv(int fd, void * __restrict buff, size_t size, int flags)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(buff);
	unsk_assert_api(size);
	unsk_assert_api(size <= SSIZE_MAX);
	unsk_assert_api(!(flags & ~(MSG_DONTWAIT | MSG_PEEK | MSG_OOB |
	                            MSG_TRUNC | MSG_WAITALL)));

	return etux_sock_recv(fd, buff, size, flags);
}

#if defined(CONFIG_UTILS_ASSERT_API)

extern ssize_t
unsk_recv_dgram_msg(int fd, struct msghdr * __restrict msg, int flags)
	__utils_nonull(2) __warn_result __export_public;

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(2) __warn_result
ssize_t
unsk_recv_dgram_msg(int fd, struct msghdr * __restrict msg, int flags)
{
	ssize_t ret;

	ret = recvmsg(fd, msg, flags);
	if (ret > 0)
		return ret;
	else if (!ret)
		return -EAGAIN;

	return -errno;
}

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

static inline __utils_nonull(2) __warn_result
int
unsk_connect(int                                   fd,
             const struct sockaddr_un * __restrict peer,
             socklen_t                             size)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(peer);
	unsk_assert_api(size > sizeof(sa_family_t));

	return etux_sock_connect(fd, (const struct sockaddr *)peer, size);
}

extern int
unsk_connect_dgram(int                             fd,
                   const char * __restrict         peer_path,
                   struct sockaddr_un * __restrict peer_addr,
                   socklen_t * __restrict          addr_len)
	__utils_nonull(2, 3, 4) __utils_nothrow __export_public;

static inline __warn_result
int
unsk_accept(int                             fd,
            struct sockaddr_un * __restrict peer,
            socklen_t * __restrict          size,
            int                             flags)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(!peer || size);
	unsk_assert_api(!size || (*size >= sizeof(sa_family_t)));
	unsk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	int sk;

	sk = etux_sock_accept(fd, (struct sockaddr *)peer, size, flags);
	if (sk >= 0)
		unsk_assert_api(!size || (*size <= sizeof(*peer)));

	return sk;
}

static inline __utils_nothrow __warn_result
int
unsk_listen(int fd, int backlog)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(backlog >= 0);

	return etux_sock_listen(fd, backlog);
}

static inline __utils_nonull(2) __utils_nothrow __warn_result
int
unsk_bind(int fd, const struct sockaddr_un * __restrict addr, socklen_t size)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api(addr);
	unsk_assert_api(addr->sun_family == AF_UNIX);
	unsk_assert_api(size >= sizeof(sa_family_t));

	return etux_sock_bind(fd, (const struct sockaddr *)addr, size);
}

static inline __utils_nothrow __warn_result
int
unsk_open(int type, int flags)
{
	unsk_assert_api((type == SOCK_DGRAM) ||
	                (type == SOCK_STREAM) ||
	                (type == SOCK_SEQPACKET));
	unsk_assert_api(!(flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)));

	return etux_sock_open(AF_UNIX, type, 0, flags);
}

static inline __utils_nothrow
void
unsk_shutdown(int fd, int how)
{
	unsk_assert_api(fd >= 0);
	unsk_assert_api((how == SHUT_RD) ||
	                (how == SHUT_WR) ||
	                (how == SHUT_RDWR));

	etux_sock_shutdown(fd, how);
}

static inline
int
unsk_close(int fd)
{
	unsk_assert_api(fd >= 0);

	return etux_sock_close(fd);
}

static inline __utils_nonull(1) __utils_nothrow
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

/******************************************************************************
 * UNIX socket buffer and queue handling
 ******************************************************************************/

#define UNSK_BUFF_SIZE_MAX (256U * 1024U)

struct unsk_buff {
	struct stroll_slist_node node;
	size_t                   bytes;
};

#define UNSK_BUFF_COUNT_MAX (128U)

struct unsk_buffq {
	struct stroll_slist busy;
	struct stroll_slist free;
};

static inline __utils_nonull(1) __utils_pure __utils_nothrow
bool
unsk_buffq_has_busy(const struct unsk_buffq * __restrict buffq)
{
	unsk_assert_api(buffq);

	return !stroll_slist_empty(&buffq->busy);
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow
bool
unsk_buffq_has_free(const struct unsk_buffq * __restrict buffq)
{
	unsk_assert_api(buffq);

	return !stroll_slist_empty(&buffq->free);
}

extern struct unsk_buff *
unsk_buffq_peek_busy(const struct unsk_buffq * __restrict buffq)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__returns_nonull
	__export_public;

extern struct unsk_buff *
unsk_buffq_peek_free(const struct unsk_buffq * __restrict buffq)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__returns_nonull
	__export_public;

extern void
unsk_buffq_nqueue_busy(struct unsk_buffq * __restrict buffq,
                       struct unsk_buff * __restrict  buff)
	__utils_nonull(1, 2) __utils_nothrow __leaf __export_public;

extern void
unsk_buffq_requeue_busy(struct unsk_buffq * __restrict buffq,
                        struct unsk_buff * __restrict  buff)
	__utils_nonull(1, 2) __utils_nothrow __leaf __export_public;

extern struct unsk_buff *
unsk_buffq_dqueue_busy(struct unsk_buffq * __restrict buffq)
	__utils_nonull(1)
	__utils_nothrow
	__leaf
	__returns_nonull
	__export_public;

extern struct unsk_buff *
unsk_buffq_dqueue_free(struct unsk_buffq * __restrict buffq)
	__utils_nonull(1)
	__utils_nothrow
	__leaf
	__returns_nonull
	__export_public;

extern void
unsk_buffq_release(struct unsk_buffq * __restrict buffq,
                   struct unsk_buff * __restrict  buff)
	__utils_nonull(1, 2) __utils_nothrow __leaf __export_public;

extern int
unsk_buffq_init(struct unsk_buffq * __restrict buffq,
                size_t                         buff_desc_sz,
                size_t                         max_data_sz,
                unsigned int                   max_buff_nr)
	__utils_nonull(1) __utils_nothrow __leaf __export_public;

extern void
unsk_buffq_fini(struct unsk_buffq * __restrict buffq)
	__utils_nonull(1) __utils_nothrow __leaf __export_public;

struct unsk_dgram_buff {
	struct unsk_buff   unsk;
	struct sockaddr_un peer;
	char               data[];
};

static inline __utils_nonull(1) __utils_pure __utils_nothrow __returns_nonull
struct unsk_dgram_buff *
unsk_dgram_from_buff(const struct unsk_buff * __restrict buff)
{
	unsk_assert_api(buff);

	return containerof(buff, struct unsk_dgram_buff, unsk);
}

static inline __utils_nonull(1, 2) __utils_nothrow
void
unsk_dgram_buffq_nqueue_busy(struct unsk_buffq * __restrict      buffq,
                             struct unsk_dgram_buff * __restrict buff)
{
	unsk_assert_api(buff);

	unsk_buffq_nqueue_busy(buffq, &buff->unsk);
}

static inline __utils_nonull(1, 2) __utils_nothrow
void
unsk_dgram_buffq_requeue_busy(struct unsk_buffq * __restrict      buffq,
                              struct unsk_dgram_buff * __restrict buff)
{
	unsk_assert_api(buff);

	unsk_buffq_requeue_busy(buffq, &buff->unsk);
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __returns_nonull
struct unsk_dgram_buff *
unsk_dgram_buffq_peek_busy(const struct unsk_buffq * __restrict buffq)
{
	return unsk_dgram_from_buff(unsk_buffq_peek_busy(buffq));
}

static inline __utils_nonull(1) __utils_nothrow __returns_nonull
struct unsk_dgram_buff *
unsk_dgram_buffq_dqueue_busy(struct unsk_buffq * __restrict buffq)
{
	return unsk_dgram_from_buff(unsk_buffq_dqueue_busy(buffq));
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __returns_nonull
struct unsk_dgram_buff *
unsk_dgram_buffq_peek_free(const struct unsk_buffq * __restrict buffq)
{
	return unsk_dgram_from_buff(unsk_buffq_peek_free(buffq));
}

static inline __utils_nonull(1) __utils_nothrow __returns_nonull
struct unsk_dgram_buff *
unsk_dgram_buffq_dqueue_free(struct unsk_buffq * __restrict buffq)
{
	return unsk_dgram_from_buff(unsk_buffq_dqueue_free(buffq));
}

static inline __utils_nonull(1, 2) __utils_nothrow
void
unsk_dgram_buffq_release(struct unsk_buffq * __restrict      buffq,
                         struct unsk_dgram_buff * __restrict buff)
{
	unsk_assert_api(buff);

	unsk_buffq_release(buffq, &buff->unsk);
}

static inline __utils_nonull(1) __utils_nothrow
int
unsk_dgram_buffq_init(struct unsk_buffq * __restrict buffq,
                      size_t                         max_data_sz,
                      unsigned int                   max_buff_nr)
{
	return unsk_buffq_init(buffq,
	                       sizeof(struct unsk_dgram_buff),
	                       max_data_sz,
	                       max_buff_nr);
}

/******************************************************************************
 * Service / server side UNIX socket handling
 ******************************************************************************/

/**
 * Service / server side UNIX socket.
 */
struct unsk_svc {
	/**
	 * @internal
	 *
	 * System socket file descriptor
	 */
	int                fd;

	/**
	 * @internal
	 *
	 * Filesystem pathname this local UNIX socket is bound to
	 */
	struct sockaddr_un local;
};

/**
 * Validate a filesystem path to bind a service side UNIX socket to.
 *
 * @param[in] path Path to validate
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 * @retval 0             path is valid
 * @retval -EFAULT       path is `NULL`
 * @retval -EINVAL       path is empty
 * @retval -ENAMETOOLONG path length too long.
 *
 * @see @man{unix(7)}
 */
static inline __utils_pure __utils_nothrow
int
unsk_svc_is_path_ok(const char * __restrict path)
{
	return unsk_is_named_path_ok(path);
}

/**
 * Transmit a message from a service side UNIX datagram socket to specified peer
 * socket.
 *
 * @param[in] sock   local service side UNIX socket
 * @param[in] data   data to send
 * @param[in] size   number of bytes to send
 * @param[in] peer   address of peer abstract socket to send to
 * @param[in] flags  flags to send according to
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 * @retval 0             success
 * @retval -EAGAIN       socket is nonblocking and the send operation would
 *                       block
 * @retval -EINTR        signal occurred before any data was transmitted
 * @retval -ECONNREFUSED connection refused, i.e., peer (client) socket has
 *                       closed
 * @retval -ENOMEM       no memory available
 *
 * Note that @p flags support is limited to `MSG_DONTWAIT` and `MSG_MORE`.
 *
 * @see
 * - @man{sendmsg(2)}
 * - @man{unix(7)}
 */
extern int
unsk_dgram_svc_send(const struct unsk_svc * __restrict    sock,
                    const void * __restrict               data,
                    size_t                                size,
                    const struct sockaddr_un * __restrict peer,
                    int                                   flags)
	__utils_nonull(1, 2, 4) __warn_result __export_public;

/**
 * Fetch a datagram from a service side UNIX datagram named socket.
 *
 * @param[in]  sock  local service side UNIX socket
 * @param[out] data  buffer to store datagram into
 * @param[in]  size  number of bytes @p data may hold
 * @param[out] peer  address of peer (client) UNIX socket that sent @p data
 * @param[out] creds credentials of process that drives @p peer socket
 * @param[in]  flags flags according to which to receive
 *
 * @return `> 0` when successful, a negative errno-like return code otherwise.
 * @retval > 0            success, i.e., number of bytes received
 * @retval -EAGAIN        socket is nonblocking and the receive operation would
 *                        block, i.e. there is no available data to receive
 * @retval -EINTR         signal occurred before any data could be received
 * @retval -EADDRNOTAVAIL invalid peer (client) abstract socket address
 * @retval -EMSGSIZE      received datagram was too large to fit into @p data
 * @retval -EPROTO        missing credentials ancillary control message
 * @retval -ENOMEM        no memory available.
 *
 * Note that @p flags support is limited to `MSG_CMSG_CLOEXEC` and
 * `MSG_DONTWAIT`.
 *
 * @see
 * - @man{recvmsg(2)}
 * - @man{unix(7)}
 *
 */
extern ssize_t
unsk_dgram_svc_recv(const struct unsk_svc * __restrict sock,
                    void * __restrict                  data,
                    size_t                             size,
                    struct sockaddr_un *               peer,
                    struct ucred * __restrict          creds,
                    int                                flags)
	__utils_nonull(1, 2, 4, 5) __warn_result __export_public;

/**
 * Bind a UNIX service named socket to a local filesystem pathname.
 *
 * @param[inout] sock local service side UNIX socket
 * @param[in]    path pathname to bind @p sock to
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 * @retval 0           success
 * @retval -EACCES     the local address the service socket is bound to is
 *                     protected (and the user is not the superuser) search
 *                     permission denied on a component of @p path
 * @retval -EADDRINUSE given address (@p path) already in use protected (and the
 *                     user is not the superuser)
 * @retval -ELOOP      too many symbolic links encountered in resolving @p path
 * @retval -ENOENT     a component in the directory prefix of @p path does not
 *                     exist
 * @retval -EISDIR     @p path is an existing directory
 * @retval -ENOTDIR    a component of @p path prefix is not a directory
 * @retval -EROFS      socket inode would reside on a read-only filesystem
 * @retval -ENOMEM     no memory available
 *
 * Once bound successfully, the corresponding filesystem entry will have been
 * created according to pathname given by @p path.
 * This filesystem entry should be delete using unsk_svc_close() once no longer
 * needed.
 *
 * unsk_svc_bind() will try to unlink(2) @p path before binding to prevent from
 * failing with `-EADDRINUSE` error code. This allows a crashing application to
 * successfully bind once restarted.
 *
 * @warning
 * The way @p path is removed from filesystem is not safe against multiple
 * threads / processes binding to @p path:
 * - a process could silently remove the named socket created by a previous
 *   one ;
 * - there is a possible race condition betwwen @p path removal by
 *   @man{unlink(2)} and the binding operation performed using @man{bind(2)}
 *   internally.
 *
 * To overcome such a situation, the caller must rely upon external
 * synchronization mechanisms such as advisory / mandatory filesystem locking.
 * See @man{flock(2)}, @man{lockf(3)} and @man{fcntl(2)} man pages for more
 * infos.
 *
 * See
 * - @man{bind(2)}
 * - @man{unix(7)}
 */
extern int
unsk_svc_bind(struct unsk_svc * __restrict sock, const char * __restrict path)
	__utils_nonull(1, 2) __utils_nothrow __export_public;

/**
 * Open a service / server side UNIX datagram socket.
 *
 * @param[out] sock  local service side UNIX socket
 * @param[in]  flags flags to open socket with
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 * @retval 0        success
 * @retval -EACCES  socket creation permission denied
 * @retval -EMFILE  system-wide limit on the total number of open files has been
 *                  reached
 * @retval -ENOMEM  no memory available
 * @retval -ENOBUFS same as `-ENOMEM`
 *
 * Note that @p flags support is limited to `SOCK_NONBLOCK` and `SOCK_CLOEXEC`.
 *
 * @see
 * - @man{socket(2)}
 * - @man{unix(7)}
 */
extern int
unsk_dgram_svc_open(struct unsk_svc * __restrict sock, int flags)
	__utils_nonull(1) __utils_nothrow __export_public;

/**
 * Close all endpoints of a service / server side UNIX socket.
 *
 * @param[in] sock local service side UNIX socket
 *
 * @see
 * - @man{close(2)}
 * - @man{shutdown(2)}
 * - @man{unix(7)}
 */
extern int
unsk_svc_close(const struct unsk_svc * __restrict sock)
	__utils_nonull(1) __export_public;

/******************************************************************************
 * Client side UNIX socket handling
 ******************************************************************************/

/**
 * UNIX socket ancillary / control message holding process credentials.
 *
 * @see
 * - @man{cmsg(3)}
 * - @man{recvmsg(2)}
 * - @man{sendmsg(2)}
 * - @man{unix(7)}
 */
union unsk_creds {
	/**
	 * @internal
	 *
	 * Raw buffer where ancillary message content is stored.
	 */
	char           buff[CMSG_SPACE(sizeof(struct ucred))];

	/**
	 * @internal
	 *
	 * Ancillary message descriptor.
	 */
	struct cmsghdr head;
};

/**
 * Client side UNIX socket.
 */
struct unsk_clnt {
	/**
	 * @internal
	 *
	 * System socket file descriptor
	 */
	int                fd;

	/**
	 * @internal
	 *
	 * Address of filesystem pathname peer (service) UNIX socket
	 */
	struct sockaddr_un peer;

	/**
	 * @internal
	 *
	 * Size of the above address
	 */
	socklen_t          peer_sz;

	/**
	 * @internal
	 *
	 * Ancillary / control message where credentials of process owning this
	 * socket are stored.
	 */
	union unsk_creds   creds;
};

/**
 * Transmit a message from a client side UNIX datagram socket to specified peer
 * (service) socket.
 *
 * @param[in] sock  local client side UNIX socket
 * @param[in] data  data to send
 * @param[in] size  number of bytes to send
 * @param[in] flags flags to send according to
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 * @retval 0             success
 * @retval -EAGAIN       socket is nonblocking and the send operation would
 *                       block
 * @retval -EINTR        signal occurred before any data was transmitted
 * @retval -EACCES       write permission is denied on destination socket file,
 *                       or search permission is denied for one of prefix path
 *                       directories
 * @retval -ENOENT       peer (service) socket filesystem entry not found
 * @retval -ECONNREFUSED connection refused, i.e., peer (service) socket has
 *                       closed
 * @retval -ENOMEM       no memory available
 *
 * Note that @p flags support is limited to `MSG_DONTWAIT` and `MSG_MORE`.
 *
 * See
 * - @man{sendmsg(2)}
 * - @man{unix(7)}
 */
extern int
unsk_dgram_clnt_send(const struct unsk_clnt * __restrict sock,
                     const void * __restrict             data,
                     size_t                              size,
                     int                                 flags)
	__utils_nonull(1, 2) __warn_result __export_public;

/**
 * Fetch a datagram from a client side UNIX datagram unamed socket.
 *
 * @param[in]  sock  local client side UNIX socket
 * @param[out] data  buffer to store datagram into
 * @param[in]  size  number of bytes @p data may hold
 * @param[in]  flags flags according to which to receive
 *
 * @return `>0` when successful, a negative errno-like return code otherwise.
 * @retval >0             success, i.e., number of bytes received
 * @retval -EAGAIN        socket is nonblocking and the receive operation would
 *                        block, i.e. there is no available data to receive
 * @retval -EINTR         signal occurred before any data could be received
 * @retval -EADDRNOTAVAIL sender address does not match the service socket we
 *                        are connected to
 * @retval -EMSGSIZE      received datagram was too large to fit into @p data
 * @retval -ENOMEM        no memory available.
 *
 * Note that the @p flags support is limited to `MSG_CMSG_CLOEXEC` and
 * `MSG_DONTWAIT`.
 *
 * @see
 * - @man{recvmsg(2)}
 * - @man{unix(7)}
 */
extern ssize_t
unsk_dgram_clnt_recv(const struct unsk_clnt * __restrict sock,
                     void * __restrict                   data,
                     size_t                              size,
                     int                                 flags)
	__utils_nonull(1, 2) __warn_result __export_public;

/**
 * Connect a UNIX datagram client socket to specified peer (service) named
 * socket.
 *
 * @param[inout] sock local client side UNIX socket
 * @param[in]    path filesystem pathname to peer (service) UNIX datagram socket
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 * @retval 0       success
 * @retval -EACCES the local address the client socket is bound to is
 *                 protected (and the user is not the superuser)
 * @retval -ENOMEM no memory available.
 *
 * @see
 * - @man{bind(2)}
 * - @man{unix(7)}
 */
extern int
unsk_dgram_clnt_connect(struct unsk_clnt * __restrict sock,
                        const char * __restrict       path)
	__utils_nonull(1, 2) __utils_nothrow __export_public;

/**
 * Open a client side UNIX datagram socket.
 *
 * @param[out] sock   local client side UNIX socket
 * @param[in]  flags  flags to open socket with
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 * @retval 0        success
 * @retval -EACCES  socket creation permission denied
 * @retval -EMFILE  system-wide limit on the total number of open files has
 *                  been reached
 * @retval -ENOMEM  no memory available
 * @retval -ENOBUFS same as -ENOMEM
 *
 * Note that @p flags support is limited to `SOCK_NONBLOCK` and `SOCK_CLOEXEC`.
 *
 * @see
 * - @man{socket(2)}
 * - @man{unix(7)}
 */
extern int
unsk_dgram_clnt_open(struct unsk_clnt * __restrict sock, int flags)
	__utils_nonull(1) __utils_nothrow __export_public;

/**
 * Close all endpoints of a client side UNIX socket.
 *
 * @param[in] sock local client side UNIX socket
 *
 * @see
 * - @man{close(2)}
 * - @man{shutdown(2)}
 * - @man{unix(7)}
 */
extern void
unsk_clnt_close(const struct unsk_clnt * __restrict sock)
	__utils_nonull(1) __export_public;

/******************************************************************************
 * Asynchronous service / server side UNIX socket handling
 ******************************************************************************/

#if defined(CONFIG_UTILS_POLL_UNSK)

struct unsk_async_svc {
	struct upoll_worker work;
	struct unsk_svc     sock;
};

static inline __utils_nonull(1, 2) __utils_nothrow
void
unsk_async_svc_apply_watch(struct unsk_async_svc * __restrict svc,
                           const struct upoll * __restrict    poller)
{
	unsk_assert_api(svc);

	upoll_apply(poller, svc->sock.fd, &svc->work);
}

static inline __utils_nonull(1) __utils_pure __utils_nothrow __returns_nonull
struct unsk_async_svc *
unsk_async_svc_from_worker(const struct upoll_worker * __restrict worker)
{
	return containerof(worker, struct unsk_async_svc, work);
}

static inline __utils_nonull(1, 2) __warn_result
int
unsk_dgram_async_svc_send(const struct unsk_async_svc * __restrict  svc,
                          const struct unsk_dgram_buff * __restrict buff,
                          int                                       flags)
{
	unsk_assert_api(svc);

	return unsk_dgram_svc_send(&svc->sock,
	                           buff->data,
	                           buff->unsk.bytes,
	                           &buff->peer,
	                           flags);
}

extern int
unsk_dgram_async_svc_recv(const struct unsk_async_svc * __restrict svc,
                          struct unsk_dgram_buff * __restrict      buff,
                          size_t                                   size,
                          struct ucred * __restrict                creds,
                          int                                      flags)
	__utils_nonull(1, 2, 4) __warn_result __export_public;

extern int
unsk_dgram_async_svc_open(struct unsk_async_svc * __restrict svc,
                          const char * __restrict            path,
                          int                                sock_flags,
                          const struct upoll * __restrict    poller,
                          uint32_t                           poll_flags,
                          upoll_dispatch_fn *                dispatch)
	__utils_nonull(1, 2, 4, 6) __utils_nothrow __export_public;

extern int
unsk_dgram_async_svc_close(struct unsk_async_svc * __restrict svc,
                           const struct upoll * __restrict    poller)
	__utils_nonull(1, 2) __export_public;

#endif /* defined(CONFIG_UTILS_POLL_UNSK) */

#endif /* _UTILS_UNSK_H */
