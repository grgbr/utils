#ifndef _UTILS_ATOMIC_H
#define _UTILS_ATOMIC_H

#define atomic_load(_atom) \
	__atomic_load_n(_atom, __ATOMIC_ACQUIRE)

#define atomic_store(_atom, _val) \
	__atomic_store_n(_atom, _val, __ATOMIC_RELEASE)

#define atomic_add(_atom, _val) \
	__atomic_add_fetch(_atom, _val, __ATOMIC_ACQ_REL)

#define atomic_inc(_atom) \
	atomic_add(_atom, 1)

#define atomic_sub(_atom, _val) \
	__atomic_sub_fetch(_atom, _val, __ATOMIC_ACQ_REL)

#define atomic_dec(_atom) \
	atomic_sub(_atom, 1)

#define atomic_dec_and_fetch(_atom) \
	atomic_dec(_atom)

#endif /* _UTILS_ATOMIC_H */
