#ifndef _UTILS_UNSK_H
#define _UTILS_UNSK_H

#include <utils/slist.h>
#include <utils/poll.h>
#include <utils/fd.h>
#include <sys/socket.h>
#include <sys/un.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __unsk_nonull(_arg_index, ...)

#define __unsk_pure

#define unsk_assert(_expr) \
	uassert("unsk", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __unsk_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define __unsk_pure \
	__pure

#define unsk_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

/******************************************************************************
 * low-level UNIX socket wrappers
 ******************************************************************************/

#define UNSK_INIT_NAMED_ADDR(_path) \
	{ .sun_family = AF_UNIX, .sun_path = _path }

#define UNSK_INIT_NAMED_ADDR_LEN(_path) \
	(offsetof(struct sockaddr_un, sun_path) + sizeof(_path))

extern int
unsk_is_named_path_ok(const char * path) __unsk_pure __nothrow __leaf;

extern int
unsk_connect_dgram(int                             fd,
                   const char * __restrict         peer_path,
                   struct sockaddr_un * __restrict peer_addr,
                   socklen_t * __restrict          addr_len)
	__unsk_nonull(2, 3, 4) __nothrow __leaf;

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __unsk_nonull(3) __nothrow
unsk_setsockopt(int fd, int option, const void * value, socklen_t size)
{
	unsk_assert(fd >= 0);
	unsk_assert(option);
	unsk_assert(value);
	unsk_assert(size);

	unsk_assert(!setsockopt(fd, SOL_SOCKET, option, value, size));
}

extern ssize_t
unsk_send_dgram_msg(int fd, const struct msghdr * msg, int flags)
	__unsk_nonull(2) __warn_result;

extern ssize_t
unsk_recv_dgram_msg(int fd, struct msghdr * msg, int flags)
	__unsk_nonull(2) __warn_result;

extern int
unsk_bind(int fd, const struct sockaddr_un * addr, socklen_t size)
	__unsk_nonull(2) __nothrow __leaf;

extern int
unsk_open(int type, int flags) __nothrow __leaf;

extern int
unsk_close(int fd);

extern int
unsk_unlink(const char * path) __unsk_nonull(1) __nothrow __leaf;

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __unsk_nonull(3) __nothrow
unsk_setsockopt(int fd, int option, const void * value, socklen_t size)
{
	setsockopt(fd, SOL_SOCKET, option, value, size);
}

static inline ssize_t __unsk_nonull(2) __warn_result
unsk_send_dgram_msg(int fd, const struct msghdr * msg, int flags)
{
	ssize_t ret;

	ret = sendmsg(fd, msg, flags);
	if (ret > 0)
		return ret;
	else if (!ret)
		return -EAGAIN;

	return -errno;
}

static inline ssize_t __unsk_nonull(2) __warn_result
unsk_recv_dgram_msg(int fd, struct msghdr * msg, int flags)
{
	ssize_t ret;

	ret = recvmsg(fd, msg, flags);
	if (ret > 0)
		return ret;
	else if (!ret)
		return -EAGAIN;

	return -errno;
}

static inline int __unsk_nonull(2) __nothrow
unsk_bind(int fd, const struct sockaddr_un * addr, socklen_t size)
{
	if (!bind(fd, (struct sockaddr *)addr, size))
		return 0;

	return -errno;
}

static inline int __nothrow
unsk_open(int type, int flags)
{
	int fd;

	fd = socket(AF_UNIX, type | flags, 0);
	if (fd >= 0)
		return fd;

	return -errno;
}

static inline int
unsk_close(int fd)
{
	return ufd_close(fd);
}

static inline int __unsk_nonull(1) __nothrow
unsk_unlink(const char * path)
{
	if (!upath_unlink(path) || (errno == ENOENT))
		return 0;

	return -errno;
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

/******************************************************************************
 * UNIX socket buffer and queue handling
 ******************************************************************************/

#define UNSK_BUFF_SIZE_MAX (256U * 1024U)

struct unsk_buff {
	struct slist_node node;
	size_t            bytes;
};

#define UNSK_BUFF_COUNT_MAX (128U)

struct unsk_buffq {
	struct slist busy;
	struct slist free;
};

static inline bool __unsk_nonull(1) __unsk_pure __nothrow
unsk_buffq_has_busy(const struct unsk_buffq * buffq)
{
	unsk_assert(buffq);

	return !slist_empty(&buffq->busy);
}

static inline bool __unsk_nonull(1) __unsk_pure __nothrow
unsk_buffq_has_free(const struct unsk_buffq * buffq)
{
	unsk_assert(buffq);

	return !slist_empty(&buffq->free);
}

extern void
unsk_buffq_nqueue_busy(struct unsk_buffq * __restrict buffq,
                       struct unsk_buff * __restrict  buff)
	__unsk_nonull(1, 2) __leaf __nothrow;

extern void
unsk_buffq_requeue_busy(struct unsk_buffq * __restrict buffq,
                        struct unsk_buff * __restrict  buff)
	__unsk_nonull(1, 2) __nothrow;

extern struct unsk_buff *
unsk_buffq_peek_busy(const struct unsk_buffq * buffq)
	__unsk_nonull(1) __unsk_pure __nothrow __returns_nonull;

extern struct unsk_buff *
unsk_buffq_dqueue_busy(struct unsk_buffq * buffq)
	__unsk_nonull(1) __nothrow __returns_nonull;

extern struct unsk_buff *
unsk_buffq_peek_free(const struct unsk_buffq * buffq)
	__unsk_nonull(1) __unsk_pure __nothrow __returns_nonull;

extern struct unsk_buff *
unsk_buffq_dqueue_free(struct unsk_buffq * buffq)
	__unsk_nonull(1) __nothrow __returns_nonull;

extern void
unsk_buffq_release(struct unsk_buffq * __restrict buffq,
                   struct unsk_buff * __restrict  buff)
	__unsk_nonull(1, 2) __nothrow;

extern int
unsk_buffq_init(struct unsk_buffq * buffq,
                size_t              buff_desc_sz,
                size_t              max_data_sz,
                unsigned int        max_buff_nr) __unsk_nonull(1) __nothrow;

extern void
unsk_buffq_fini(struct unsk_buffq * buffq) __unsk_nonull(1) __nothrow;

struct unsk_dgram_buff {
	struct unsk_buff   unsk;
	struct sockaddr_un peer;
	char               data[];
};

static inline
struct unsk_dgram_buff * __unsk_nonull(1) __unsk_pure __nothrow __returns_nonull
unsk_dgram_from_buff(const struct unsk_buff * buff)
{
	unsk_assert(buff);

	return containerof(buff, struct unsk_dgram_buff, unsk);
}

static inline void __unsk_nonull(1, 2) __nothrow
unsk_dgram_buffq_nqueue_busy(struct unsk_buffq * __restrict      buffq,
                             struct unsk_dgram_buff * __restrict buff)
{
	unsk_assert(buff);

	unsk_buffq_nqueue_busy(buffq, &buff->unsk);
}

static inline void __unsk_nonull(1, 2) __nothrow
unsk_dgram_buffq_requeue_busy(struct unsk_buffq * __restrict      buffq,
                              struct unsk_dgram_buff * __restrict buff)
{
	unsk_assert(buff);

	unsk_buffq_requeue_busy(buffq, &buff->unsk);
}

static inline
struct unsk_dgram_buff * __unsk_nonull(1) __unsk_pure __nothrow __returns_nonull
unsk_dgram_buffq_peek_busy(const struct unsk_buffq * buffq)
{
	return unsk_dgram_from_buff(unsk_buffq_peek_busy(buffq));
}

static inline
struct unsk_dgram_buff * __unsk_nonull(1) __nothrow __returns_nonull
unsk_dgram_buffq_dqueue_busy(struct unsk_buffq * buffq)
{
	return unsk_dgram_from_buff(unsk_buffq_dqueue_busy(buffq));
}

static inline
struct unsk_dgram_buff * __unsk_nonull(1) __unsk_pure __nothrow __returns_nonull
unsk_dgram_buffq_peek_free(const struct unsk_buffq * buffq)
{
	return unsk_dgram_from_buff(unsk_buffq_peek_free(buffq));
}

static inline
struct unsk_dgram_buff * __unsk_nonull(1) __nothrow __returns_nonull
unsk_dgram_buffq_dqueue_free(struct unsk_buffq * buffq)
{
	return unsk_dgram_from_buff(unsk_buffq_dqueue_free(buffq));
}

static inline void __unsk_nonull(1, 2) __nothrow
unsk_dgram_buffq_release(struct unsk_buffq * __restrict      buffq,
                         struct unsk_dgram_buff * __restrict buff)
{
	unsk_assert(buff);

	unsk_buffq_release(buffq, &buff->unsk);
}

static inline int __unsk_nonull(1) __nothrow
unsk_dgram_buffq_init(struct unsk_buffq * buffq,
                      size_t              max_data_sz,
                      unsigned int        max_buff_nr)
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
 * struct unsk_svc - Service / server side UNIX socket.
 */
struct unsk_svc {
	/* private: system socket file descriptor */
	int                fd;

	/* private: filesystem pathname this local UNIX socket is bound to */
	struct sockaddr_un local;
};

/**
 * unsk_svc_is_path_ok() - Validate a filesystem path to bind a service side
 *                         UNIX socket to.
 *
 * @path: path to validate
 *
 * See: unix(7) man pages.
 *
 * Return:
 * * 0             - path is valid,
 * * -EFAULT       - path is NULL,
 * * -EINVAL       - path is empty,
 * * -ENAMETOOLONG - path length too long.
 */
static inline int __unsk_pure __nothrow
unsk_svc_is_path_ok(const char * path)
{
	return unsk_is_named_path_ok(path);
}

/**
 * unsk_dgram_svc_send() - Transmit a message from a service side UNIX datagram
 *                         socket to specified peer socket.
 *
 * @sock:  local service side UNIX socket
 * @data:  data to send
 * @size:  number of bytes to send
 * @peer:  address of peer abstract socket to send to
 * @flags: flags to send according to
 *
 * @flags support limited to MSG_DONTWAIT.
 *
 * See: sendmsg(2) and unix(7) man pages.
 *
 * Return:
 * * 0             - success,
 * * -EAGAIN       - socket is nonblocking and the send operation would block,
 * * -EINTR        - signal occurred before any data was transmitted,
 * * -ECONNREFUSED - connection refused, i.e., peer (client) socket has closed,
 * * -ENOMEM       - no memory available.
 */
extern int
unsk_dgram_svc_send(const struct unsk_svc * __restrict    sock,
                    const void *                          data,
                    size_t                                size,
                    const struct sockaddr_un * __restrict peer,
                    int                                   flags)
	__unsk_nonull(1, 2, 4) __warn_result;

/**
 * unsk_dgram_svc_recv() - Fetch a datagram from a service side UNIX
 *                         datagram named socket.
 *
 * @sock:  local service side UNIX socket
 * @data:  buffer to store datagram into
 * @size:  number of bytes @data may hold
 * @peer:  address of peer (client) UNIX socket that sent @data
 * @creds: credentials of process that drives @peer socket
 * @flags: flags according to which to receive
 *
 * @flags support limited to MSG_CMSG_CLOEXEC and MSG_DONTWAIT.
 *
 * See: recvmsg(2) and unix(7) man pages.
 *
 * Return:
 * * >0             - success, i.e., number of bytes received,
 * * -EAGAIN        - socket is nonblocking and the receive operation would
 *                    block, i.e. there is no available data to receive,
 * * -EINTR         - signal occurred before any data could be received,
 * * -EADDRNOTAVAIL - invalid peer (client) abstract socket address,
 * * -EMSGSIZE      - received datagram was too large to fit into @data,
 * * -EPROTO        - missing credentials ancillary control message,
 * * -ENOMEM        - no memory available.
 */
extern ssize_t
unsk_dgram_svc_recv(const struct unsk_svc * __restrict sock,
                    void *                             data,
                    size_t                             size,
                    struct sockaddr_un *               peer,
                    struct ucred * __restrict          creds,
                    int                                flags)
	__unsk_nonull(1, 2, 4, 5) __warn_result;

/**
 * unsk_svc_bind() - Bind a UNIX service named socket to a local filesystem
 *                   pathname.
 *
 * @sock: local service side UNIX socket
 * @path: filesystem pathname to bind @sock to
 *
 * Once bound successfully, the corresponding filesystem entry will have been
 * created according to pathname given by @path.
 * This filesystem entry should be delete using unsk_svc_close() once no longer
 * needed.
 *
 * unsk_svc_bind() will try to unlink(2) @path before binding to prevent from
 * failing with -EADDRINUSE error code. This allows a crashing application to
 * successfully bind once restarted.
 *
 * Warning:
 * The way @path is removed from filesystem is not safe against multiple threads
 * / processes binding to @path:
 * - a process could silently remove the named socket created by a previous
 *   one ;
 * - there is a possible race condition betwwen @path removal by unlink(2) and
 *   the binding operation performed using bind(2) internally.
 * To overcome such a situation, the caller must rely upon external
 * synchronization mechanisms such as advisory / mandatory filesystem locking.
 * See flock(2), lockf(3) and fcntl(2) man pages for more infos.
 *
 * See: bind(2) and unix(7) man pages.
 *
 * Return:
 * * 0             - success,
 * * -EACCES       - the local address the service socket is bound to is
 *                   protected (and the user is not the superuser),
 *                   search permission denied on a component of @path,
 * * -EADDRINUSE   - given address (@path) already in use
 *                   protected (and the user is not the superuser)
 * * -ELOOP        - too many symbolic links encountered in resolving @path,
 * * -ENOENT       - a component in the directory prefix of @path does not
 *                   exist,
 * * -EISDIR       - @path is an existing directory,
 * * -ENOTDIR      - a component of @path prefix is not a directory
 * * -EROFS        - socket inode would reside on a read-only filesystem,
 * * -ENOMEM       - no memory available.
 */
extern int
unsk_svc_bind(struct unsk_svc * __restrict sock, const char * __restrict path)
	__unsk_nonull(1, 2) __nothrow;

/**
 * unsk_dgram_svc_open() - Open a service / server side UNIX datagram socket.
 *
 * @sock:  local service side UNIX socket
 * @flags: flags to open socket with
 *
 * @flags support limited to SOCK_NONBLOCK and SOCK_CLOEXEC.
 *
 * See: socket(2) and unix(7) man pages.
 *
 * Return:
 * * 0             - success,
 * * -EACCES       - socket creation permission denied,
 * * -EMFILE       - system-wide limit on the total number of open files has
 *                   been reached,
 * * -ENOMEM       - no memory available,
 * * -ENOBUFS      - same as -ENOMEM.
 */
extern int
unsk_dgram_svc_open(struct unsk_svc * sock, int flags)
	__unsk_nonull(1) __nothrow;

/**
 * unsk_svc_close() - Close all endpoints of a service / server side UNIX
 *                    socket.
 *
 * @sock: local service side UNIX socket
 *
 * See: close(2), shutdown(2) and unix(7) man pages.
 */
extern int
unsk_svc_close(const struct unsk_svc * sock) __unsk_nonull(1);

/******************************************************************************
 * Client side UNIX socket handling
 ******************************************************************************/

/**
 * union unsk_creds - UNIX socket ancillary / control message holding process
 *                    credentials.
 *
 * See: cmsg(3), recvmsg(2), sendmsg(2) and unix(7) man pages
 */
union unsk_creds {
	/* private: raw buffer where ancillary message content is stored. */
	char           buff[CMSG_SPACE(sizeof(struct ucred))];
	/* private: ancillary message descriptor. */
	struct cmsghdr head;
};

/**
 * struct unsk_clnt - Client side UNIX socket.
 */
struct unsk_clnt {
	/* private: system socket file descriptor */
	int                fd;

	/* private: address of filesystem pathname peer (service) UNIX socket */
	struct sockaddr_un peer;

	/* private: size of the above address */
	socklen_t          peer_sz;

	/*
	 * private: ancillary / control message where credentials of process
	 *          owning this socket are stored.
	 */
	union unsk_creds   creds;
};

/**
 * unsk_dgram_clnt_send() - Transmit a message from a client side UNIX datagram
 *                          socket to specified peer (service) socket.
 *
 * @sock:  local client side UNIX socket
 * @data:  data to send
 * @size:  number of bytes to send
 * @flags: flags to send according to
 *
 * @flags support limited to MSG_DONTWAIT.
 *
 * See: sendmsg(2) and unix(7) man pages.
 *
 * Return:
 * * 0             - success,
 * * -EAGAIN       - socket is nonblocking and the send operation would block,
 * * -EINTR        - signal occurred before any data was transmitted,
 * * -EACCES       - write permission is denied on destination socket file, or
 * *                 search permission is denied for one of prefix path
 * *                 directories,
 * * -ENOENT       - peer (service) socket filesystem entry not found,
 * * -ECONNREFUSED - connection refused, i.e., peer (service) socket has closed,
 * * -ENOMEM       - no memory available.
 */
extern int
unsk_dgram_clnt_send(const struct unsk_clnt * sock,
                     const void *             data,
                     size_t                   size,
                     int                      flags)
	__unsk_nonull(1, 2) __warn_result;

/**
 * unsk_dgram_clnt_recv() - Fetch a datagram from a client side UNIX
 *                          datagram unamed socket.
 *
 * @sock:  local client side UNIX socket
 * @data:  buffer to store datagram into
 * @size:  number of bytes @data may hold
 * @flags: flags according to which to receive
 *
 * @flags support limited to MSG_CMSG_CLOEXEC and MSG_DONTWAIT.
 *
 * See: recvmsg(2) and unix(7) man pages.
 *
 * Return:
 * * >0             - success, i.e., number of bytes received,
 * * -EAGAIN        - socket is nonblocking and the receive operation would
 *                    block, i.e. there is no available data to receive,
 * * -EINTR         - signal occurred before any data could be received,
 * * -EADDRNOTAVAIL - sender address does not match the service socket we are
 *                    connected to,
 * * -EMSGSIZE      - received datagram was too large to fit into @data,
 * * -ENOMEM        - no memory available.
 */
extern ssize_t
unsk_dgram_clnt_recv(const struct unsk_clnt * __restrict sock,
                     void * __restrict                   data,
                     size_t                              size,
                     int                                 flags)
	__unsk_nonull(1, 2) __warn_result;

/**
 * unsk_dgram_clnt_connect() - Connect a UNIX datagram client socket to
 *                             specified peer (service) named socket.
 *
 * @sock: local client side UNIX socket
 * @path: filesystem pathname to peer (service) UNIX datagram socket
 *
 * See: bind(2) and unix(7) man pages.
 *
 * Return:
 * * 0             - success,
 * * -EACCES       - the local address the client socket is bound to is
 *                   protected (and the user is not the superuser)
 * * -ENOMEM       - no memory available.
 */
extern int
unsk_dgram_clnt_connect(struct unsk_clnt * __restrict sock,
                        const char * __restrict       path)
	__unsk_nonull(1, 2) __nothrow;

/**
 * unsk_dgram_clnt_open() - Open a client side UNIX datagram socket.
 *
 * @sock:  local client side UNIX socket
 * @flags: flags to open socket with
 *
 * @flags support limited to SOCK_NONBLOCK and SOCK_CLOEXEC.
 *
 * See: socket(2) and unix(7) man pages.
 *
 * Return:
 * * 0             - success,
 * * -EACCES       - socket creation permission denied,
 * * -EMFILE       - system-wide limit on the total number of open files has
 *                   been reached,
 * * -ENOMEM       - no memory available,
 * * -ENOBUFS      - same as -ENOMEM.
 */
extern int
unsk_dgram_clnt_open(struct unsk_clnt * sock, int flags)
	__unsk_nonull(1) __nothrow;

/**
 * unsk_clnt_close() - Close all endpoints of a client side UNIX socket.
 *
 * @sock: local client side UNIX socket
 *
 * See: close(2), shutdown(2) and unix(7) man pages.
 */
extern void
unsk_clnt_close(const struct unsk_clnt * sock) __unsk_nonull(1);

/******************************************************************************
 * Asynchronous service / server side UNIX socket handling
 ******************************************************************************/

#if defined(CONFIG_UTILS_POLL_UNSK)

struct unsk_async_svc {
	struct upoll_worker work;
	struct unsk_svc     sock;
};

static inline void __upoll_nonull(1, 2) __nothrow
unsk_async_svc_apply_watch(struct unsk_async_svc * __restrict svc,
                           const struct upoll * __restrict    poller)
{
	unsk_assert(svc);

	upoll_apply(poller, svc->sock.fd, &svc->work);
}

static inline
struct unsk_async_svc * __unsk_nonull(1) __unsk_pure __nothrow __returns_nonull
unsk_async_svc_from_worker(const struct upoll_worker * worker)
{
	return containerof(worker, struct unsk_async_svc, work);
}

static inline int __unsk_nonull(1, 2) __warn_result
unsk_dgram_async_svc_send(const struct unsk_async_svc * __restrict  svc,
                          const struct unsk_dgram_buff * __restrict buff,
                          int                                       flags)
{
	unsk_assert(svc);

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
	__unsk_nonull(1, 2, 4) __warn_result;

extern int
unsk_dgram_async_svc_open(struct unsk_async_svc *         svc,
                          const char * __restrict         path,
                          int                             sock_flags,
                          const struct upoll * __restrict poller,
                          uint32_t                        poll_flags,
                          upoll_dispatch_fn *             dispatch)
	__unsk_nonull(1, 2, 4, 6) __nothrow;

extern int
unsk_dgram_async_svc_close(struct unsk_async_svc * __restrict svc,
                           const struct upoll * __restrict    poller)
	__unsk_nonull(1, 2);

#endif /* defined(CONFIG_UTILS_POLL_UNSK) */

#endif /* _UTILS_UNSK_H */
