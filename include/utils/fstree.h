/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * File system tree interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      13 Mar 2025
 * @copyright Copyright (C) 2017-2025 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _ETUX_FSTREE_H
#define _ETUX_FSTREE_H

#include <utils/dir.h>

struct etux_fstree_iter;

extern const char *
etux_fstree_iter_path(const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__returns_nonull
	__warn_result;

struct etux_fstree_entry;

extern bool
etux_fstree_entry_isdot(const struct etux_fstree_entry * __restrict entry,
                        const struct etux_fstree_iter * __restrict  iter)
	__utils_nonull(1, 2) __utils_pure __utils_nothrow __leaf __warn_result;

extern int
etux_fstree_entry_type(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

extern const struct stat *
etux_fstree_entry_stat(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

extern const char *
etux_fstree_entry_name(const struct etux_fstree_entry * __restrict entry,
                       const struct etux_fstree_iter * __restrict  iter)
	__utils_nonull(1, 2)
	__utils_pure
	__utils_nothrow
	__leaf
	__returns_nonull
	__warn_result;

extern const char *
etux_fstree_entry_path(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;


extern const char *
etux_fstree_entry_slink(struct etux_fstree_entry * __restrict      entry,
                        const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __warn_result;

extern unsigned int
etux_fstree_depth(const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

enum etux_fstree_event {
	/* etux_fstree_iter() and etux_fstree_scan() events */
	ETUX_FSTREE_ENT_EVT,
	ETUX_FSTREE_NEXT_ERR_EVT,
	ETUX_FSTREE_LOAD_ERR_EVT,

	/* etux_fstree_scan() only events */
	ETUX_FSTREE_LOOP_EVT,
	ETUX_FSTREE_PRE_EVT,
	ETUX_FSTREE_POST_EVT,
	ETUX_FSTREE_DIR_ERR_EVT,

	/* End of enumeration marker. */
	ETUX_FSTREE_EVT_NR
};

enum etux_fstree_cmd {
	ETUX_FSTREE_CONT_CMD = 0,
	ETUX_FSTREE_STOP_CMD = 1,
	ETUX_FSTREE_SKIP_CMD = 2,

	/* End of enumeration marker. */
	ETUX_FSTREE_CMD_NR
};

typedef enum etux_fstree_cmd
        (etux_fstree_handle_fn)(struct etux_fstree_entry *,
                                const struct etux_fstree_iter *,
                                enum etux_fstree_event,
                                int *,
                                void *);

enum etux_fstree_option {
	/* etux_fstree_iter() and etux_fstree_scan() options */
	ETUX_FSTREE_FOLLOW_OPT = 1 << 0,

	/* etux_fstree_scan() only options */
	ETUX_FSTREE_XDEV_OPT   = 1 << 1,
	ETUX_FSTREE_PRE_OPT    = 1 << 2,
	ETUX_FSTREE_POST_OPT   = 1 << 3,

	/* End of enumeration marker. */
	ETUX_FSTREE_OPT_NR
};

extern int
etux_fstree_iter(const char * __restrict path,
                 int                     options,
                 etux_fstree_handle_fn * handle,
                 void *                  data)
	__utils_nonull(3) __warn_result;

extern int
etux_fstree_scan(const char * __restrict path,
                 int                     options,
                 etux_fstree_handle_fn * handle,
                 void *                  data)
	__utils_nonull(3) __warn_result;

#endif /* _UTILS_FSTREE_H */
