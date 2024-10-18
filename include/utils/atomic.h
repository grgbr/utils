/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * Atomic operations interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      25 May 2020
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_ATOMIC_H
#define _UTILS_ATOMIC_H

#include <utils/cdefs.h>

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
