#ifndef _UTILS_THREAD_H
#define _UTILS_THREAD_H

#include <utils/cdefs.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define __uthr_nonull(_arg_index, ...)

#define uthr_assert(_expr) \
	uassert("uthread", _expr)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define __uthr_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#define uthr_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

struct uthr_mutex {
	pthread_mutex_t pthread;
};

/* On Linux, defaults to «fast» mutex type. */
#define UTHR_INIT_MUTEX \
	{ .pthread = PTHREAD_MUTEX_INITIALIZER }

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __uthr_nonull(1) __nothrow
uthr_lock_mutex(struct uthr_mutex *mutex)
{
	uthr_assert(mutex);

	int err;

	err = pthread_mutex_lock(&mutex->pthread);

	uthr_assert(!err);
}

static inline int __uthr_nonull(1) __nothrow
uthr_trylock_mutex(struct uthr_mutex *mutex)
{
	uthr_assert(mutex);

	int err;

	err = pthread_mutex_trylock(&mutex->pthread);

	uthr_assert(err == EBUSY);

	return -err;
}

static inline void __uthr_nonull(1) __nothrow
uthr_unlock_mutex(struct uthr_mutex *mutex)
{
	uthr_assert(mutex);

	int err;

	err = pthread_mutex_unlock(&mutex->pthread);

	uthr_assert(!err);
}

static inline int __uthr_nonull(1) __nothrow
uthr_init_mutex(struct uthr_mutex *mutex)
{
	uthr_assert(mutex);

	int err;

	/* On Linux, defaults to «fast» mutex type. */
	err = pthread_mutex_init(&mutex->pthread, NULL);

	uthr_assert(err != EBUSY);
	uthr_assert(err != EINVAL);

	return -err;
}

static inline void __uthr_nonull(1) __nothrow
uthr_fini_mutex(struct uthr_mutex *mutex)
{
	uthr_assert(mutex);

	int err;

	err = pthread_mutex_destroy(&mutex->pthread);

	uthr_assert(!err);
}

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __uthr_nonull(1) __nothrow
uthr_lock_mutex(struct uthr_mutex *mutex)
{
	pthread_mutex_lock(&mutex->pthread);
}

static inline int __uthr_nonull(1) __nothrow
uthr_trylock_mutex(struct uthr_mutex *mutex)
{
	return 0 - pthread_mutex_trylock(&mutex->pthread);
}

static inline void __uthr_nonull(1) __nothrow
uthr_unlock_mutex(struct uthr_mutex *mutex)
{
	pthread_mutex_unlock(&mutex->pthread);
}

static inline int __uthr_nonull(1) __nothrow
uthr_init_mutex(struct uthr_mutex *mutex)
{
	/* On Linux, defaults to «fast» mutex type. */
	return 0 - pthread_mutex_init(&mutex->pthread, NULL);
}

static inline void __uthr_nonull(1) __nothrow
uthr_fini_mutex(struct uthr_mutex *mutex)
{
	pthread_mutex_destroy(&mutex->pthread);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

struct uthr_rdwr_lock {
	pthread_rwlock_t pthread;
};

/*
 * On Linux, defaults to recursive lock giving preference to readers.
 * See man (3) pthread_rwlockattr_setkind_np() and
 * PTHREAD_RWLOCK_PREFER_READER_NP.
 */
#define UTHR_INIT_RDWR_lock(_lock) \
	{ .pthread = PTHREAD_RWLOCK_INITIALIZER }

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline int __uthr_nonull(1) __nothrow
uthr_rdlock_rdwr(struct uthr_rdwr_lock *lock)
{
	uthr_assert(lock);

	int err;

	err = pthread_rwlock_rdlock(&lock->pthread);

	uthr_assert(err != EINVAL);
	uthr_assert(err != EDEADLK);

	return -err;
}

static inline int __uthr_nonull(1) __nothrow
uthr_tryrdlock_rdwr(struct uthr_rdwr_lock *lock)
{
	uthr_assert(lock);

	int err;

	err = pthread_rwlock_tryrdlock(&lock->pthread);

	uthr_assert(err != EINVAL);
	uthr_assert(err != EDEADLK);

	return -err;
}

static inline void __uthr_nonull(1) __nothrow
uthr_wrlock_rdwr(struct uthr_rdwr_lock *lock)
{
	uthr_assert(lock);

	int err;

	err = pthread_rwlock_wrlock(&lock->pthread);

	uthr_assert(!err);
}

static inline int __uthr_nonull(1) __nothrow
uthr_trywrlock_rdwr(struct uthr_rdwr_lock *lock)
{
	uthr_assert(lock);

	int err;

	err = pthread_rwlock_trywrlock(&lock->pthread);

	uthr_assert(err != EINVAL);
	uthr_assert(err != EDEADLK);

	return -err;
}

static inline void __uthr_nonull(1) __nothrow
uthr_unlock_rdwr(struct uthr_rdwr_lock *lock)
{
	uthr_assert(lock);

	int err;

	err = pthread_rwlock_unlock(&lock->pthread);

	uthr_assert(!err);
}

static inline int __uthr_nonull(1) __nothrow
uthr_init_rdwr_lock(struct uthr_rdwr_lock *lock)
{
	uthr_assert(lock);

	int err;

	/* On Linux, defaults to «fast» mutex type. */
	err = pthread_rwlock_init(&lock->pthread, NULL);

	uthr_assert(err != EBUSY);
	uthr_assert(err != EINVAL);

	return -err;
}

static inline void __uthr_nonull(1) __nothrow
uthr_fini_rdwr_lock(struct uthr_rdwr_lock *lock)
{
	uthr_assert(lock);

	int err;

	err = pthread_rwlock_destroy(&lock->pthread);

	uthr_assert(!err);
}

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline int __uthr_nonull(1) __nothrow
uthr_rdlock_rdwr(struct uthr_rdwr_lock *lock)
{
	return 0 - pthread_rwlock_rdlock(&lock->pthread);
}

static inline int __uthr_nonull(1) __nothrow
uthr_tryrdlock_rdwr(struct uthr_rdwr_lock *lock)
{
	return 0 - pthread_rwlock_tryrdlock(&lock->pthread);
}

static inline void __uthr_nonull(1) __nothrow
uthr_wrlock_rdwr(struct uthr_rdwr_lock *lock)
{
	pthread_rwlock_wrlock(&lock->pthread);
}

static inline int __uthr_nonull(1) __nothrow
uthr_trywrlock_rdwr(struct uthr_rdwr_lock *lock)
{
	return 0 - pthread_rwlock_trywrlock(&lock->pthread);
}

static inline void __uthr_nonull(1) __nothrow
uthr_unlock_rdwr(struct uthr_rdwr_lock *lock)
{
	pthread_rwlock_unlock(&lock->pthread);
}

static inline int __uthr_nonull(1) __nothrow
uthr_init_rdwr_lock(struct uthr_rdwr_lock *lock)
{
	/*
	 * On Linux, defaults to recursive lock giving preference to readers.
	 * See man (3) pthread_rwlockattr_setkind_np() and
	 * PTHREAD_RWLOCK_PREFER_READER_NP.
	 */
	return 0 - pthread_rwlock_init(&lock->pthread, NULL);
}

static inline void __uthr_nonull(1) __nothrow
uthr_fini_rdwr_lock(struct uthr_rdwr_lock *lock)
{
	pthread_rwlock_destroy(&lock->pthread);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

struct uthr_cond {
	pthread_cond_t pthread;
	clockid_t      clock;
};

#define UTHR_INIT_COND(_cond) \
	{ .pthread = PTHREAD_COND_INITIALIZER, .clock = CLOCK_REALTIME }

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __uthr_nonull(1, 2) __nothrow
uthr_cond_now(const struct uthr_cond *restrict cond,
              struct timespec        *restrict tspec)
{
	uthr_assert(!clock_gettime(cond->clock, tspec));
}

static inline void __uthr_nonull(1, 2)
uthr_wait_cond(struct uthr_cond  *restrict cond,
               struct uthr_mutex *restrict mutex)
{
	uthr_assert(cond);
	uthr_assert(mutex);

	int err;

	err = pthread_cond_wait(&cond->pthread, &mutex->pthread);

	uthr_assert(!err);
}

static inline int __uthr_nonull(1, 2, 3)
uthr_timed_wait_cond(struct uthr_cond      *restrict cond,
                     struct uthr_mutex     *restrict mutex,
                     const struct timespec *restrict tmout)
{
	uthr_assert(cond);
	uthr_assert(mutex);
	uthr_assert(tmout);

	int err;

	err = pthread_cond_timedwait(&cond->pthread, &mutex->pthread, tmout);

	uthr_assert(err == ETIMEDOUT);

	return -err;
}

static inline void __uthr_nonull(1) __nothrow
uthr_signal_cond(struct uthr_cond *cond)
{
	uthr_assert(cond);

	int err;

	err = pthread_cond_signal(&cond->pthread);

	uthr_assert(!err);
}

static inline void __uthr_nonull(1) __nothrow
uthr_broadcast_cond(struct uthr_cond *cond)
{
	uthr_assert(cond);

	int err;

	err = pthread_cond_broadcast(&cond->pthread);

	uthr_assert(!err);
}

static inline void __uthr_nonull(1) __nothrow
uthr_fini_cond(struct uthr_cond *cond)
{
	uthr_assert(cond);

	int err;

	err = pthread_cond_destroy(&cond->pthread);

	uthr_assert(!err);
}

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __uthr_nonull(1, 2) __nothrow
uthr_cond_now(const struct uthr_cond *restrict cond,
              struct timespec        *restrict tspec)
{
	clock_gettime(cond->clock, tspec);
}

static inline void __uthr_nonull(1, 2)
uthr_wait_cond(struct uthr_cond  *restrict cond,
               struct uthr_mutex *restrict mutex)
{
	pthread_cond_wait(&cond->pthread, &mutex->pthread);
}

static inline int __uthr_nonull(1, 2, 3)
uthr_timed_wait_cond(struct uthr_cond      *restrict cond,
                     struct uthr_mutex     *restrict mutex,
                     const struct timespec *restrict tmout)
{
	return 0 - pthread_cond_timedwait(&cond->pthread,
	                                  &mutex->pthread,
	                                  tmout);
}

static inline void __uthr_nonull(1) __nothrow
uthr_signal_cond(struct uthr_cond *cond)
{
	pthread_cond_signal(&cond->pthread);
}

static inline void __uthr_nonull(1) __nothrow
uthr_broadcast_cond(struct uthr_cond *cond)
{
	pthread_cond_broadcast(&cond->pthread);
}

static inline void __uthr_nonull(1) __nothrow
uthr_fini_cond(struct uthr_cond *cond)
{
	pthread_cond_destroy(&cond->pthread);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

extern int
uthr_timed_wait_cond_msec(struct uthr_cond  *restrict cond,
                          struct uthr_mutex *restrict mutex,
                          unsigned long      tmout_msec) __uthr_nonull(1, 2);

extern int
uthr_init_cond(struct uthr_cond *cond,
               clockid_t         clock) __uthr_nonull(1) __nothrow __leaf;

/*
 * As stated in man (3) pthread_kill, glibc implementation gives an error
 * (EINVAL) on attempts to send either of the real-time signals used internally
 * by the NPTL threading implementation, i.e. signal numbers 32 and 33.
 * See nptl(7) for details.
 *
 * POSIX.1-2008 recommends that if an implementation detects the use of
 * a thread ID after the end of its lifetime, pthread_kill() should
 * return the error ESRCH.  The glibc implementation returns this error
 * in the cases where an invalid thread ID can be detected.
 *
 * BUT !! Note also that POSIX says that an attempt to use a thread ID whose
 * lifetime has ended produces undefined behavior, and an attempt to use an
 * invalid thread ID in a call to pthread_kill() can, for example, cause a
 * segmentation fault.
 */
#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __nothrow
uthr_kill(pthread_t thread, int sig)
{
	uthr_assert(!pthread_kill(thread, sig));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __nothrow
uthr_kill(pthread_t thread, int sig)
{
	pthread_kill(thread, sig);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __nothrow
uthr_yield(void)
{
	uthr_assert(!sched_yield());
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __nothrow
uthr_yield(void)
{
	sched_yield();
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __nothrow
uthr_sigmask(int how, const sigset_t *set, sigset_t *oldset)
{
	uthr_assert(set || oldset);

	uthr_assert(!pthread_sigmask(how, set, oldset));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __nothrow
uthr_sigmask(int how, const sigset_t *set, sigset_t *oldset)
{
	pthread_sigmask(how, set, oldset);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline int __uthr_nonull(1, 3) __nothrow
uthr_create(pthread_t             *thread,
            const pthread_attr_t  *attr,
            void *               (*start_routine)(void *),
            void                  *arg)
{
	int err;

	err = pthread_create(thread, attr, start_routine, arg);
	uthr_assert(err != EINVAL);

	return -err;
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline int __uthr_nonull(1, 3) __nothrow
uthr_create(pthread_t             *thread,
            const pthread_attr_t  *attr,
            void *               (*start_routine)(void *),
            void                  *arg)
{
	return 0 - pthread_create(thread, attr, start_routine, arg);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define UTHR_NAME_MAX (16U)

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

static inline void __uthr_nonull(2) __nothrow
uthr_set_name(pthread_t thread, const char *name)
{
	uthr_assert(name);
	uthr_assert(*name);
	uthr_assert(strnlen(name, UTHR_NAME_MAX) < UTHR_NAME_MAX);

	uthr_assert(!pthread_setname_np(thread, name));
}

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline void __uthr_nonull(2) __nothrow
uthr_set_name(pthread_t thread, const char *name)
{
	pthread_setname_np(thread, name);
}

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#endif /* _UTILS_THREAD_H */
