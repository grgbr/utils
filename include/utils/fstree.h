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

extern unsigned int
etux_fstree_iter_depth(const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

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
etux_fstree_entry_isdot(
	const struct etux_fstree_entry * __restrict entry,
	const struct etux_fstree_iter * __restrict  iter __unused)
	__utils_nonull(1, 2) __utils_pure __utils_nothrow __leaf __warn_result;

/*
 * Return type of current directory entry.
 *
 * Return values:
 * >= 0     -- the type of entry as defined by <dirent.h> DT_ macros
 * -EACCES  -- search permission denied for one of the directories in the path
 *             prefix of entry
 * -ELOOP   -- too many symbolic links encountered while traversing the entry
 *             path
 * -ENOENT  -- a component of entry path does not exist or is a dangling
 *             symbolic link
 * -ENOMEM  -- out of memory
 * -ENOTDIR -- a component of the entry path prefix is not a directory
 * -ENOTSUP -- unexpected type found for entry
 */
extern int
etux_fstree_entry_type(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

/*
 * Retrieve current directory entry system properties.
 *
 * Return values:
 * >0   -- a pointer to a stat(2) structure.
 * NULL -- an error occured, errno has been set accordingly
 *
 * Possible errno values:
 * -EACCES  -- search permission denied for one of the directories in the path
 *             prefix of entry
 * -ELOOP   -- too many symbolic links encountered while traversing the entry
 *             path
 * -ENOENT  -- a component of entry path does not exist or is a dangling
 *             symbolic link
 * -ENOMEM  -- out of memory
 * -ENOTDIR -- a component of the entry path prefix is not a directory
 */
extern const struct stat *
etux_fstree_entry_stat(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

extern const char *
etux_fstree_entry_name(
	const struct etux_fstree_entry * __restrict entry,
	const struct etux_fstree_iter * __restrict  iter __unused)
	__utils_nonull(1, 2) __utils_pure
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

typedef int
        (etux_fstree_filter_fn)(struct etux_fstree_entry *,
                                const struct etux_fstree_iter *,
                                void *);

typedef int
        (etux_fstree_cmp_fn)(struct etux_fstree_entry *,
                             struct etux_fstree_entry *,
                             const struct etux_fstree_iter *,
                             void *);

#define ETUX_FSTREE_CONT_CMD (0)
#define ETUX_FSTREE_STOP_CMD (1)
#define ETUX_FSTREE_SKIP_CMD (2)

typedef int
        (etux_fstree_handle_fn)(struct etux_fstree_entry *,
                                const struct etux_fstree_iter *,
                                enum etux_fstree_event,
                                int,
                                void *);

extern int
etux_fstree_iter(const char * __restrict path,
                 int                     options,
                 etux_fstree_handle_fn * handle,
                 void *                  data)
	__utils_nonull(3);

extern int
etux_fstree_sort_iter(const char * __restrict path,
                      int                     options,
                      etux_fstree_filter_fn * filter,
                      etux_fstree_cmp_fn *    compare,
                      etux_fstree_handle_fn * handle,
                      void *                  data)
	__utils_nonull(4, 5);

extern int
etux_fstree_scan(const char * __restrict path,
                 int                     options,
                 etux_fstree_handle_fn * handle,
                 void *                  data)
	__utils_nonull(3);

extern int
etux_fstree_sort_scan(const char * __restrict path,
                      int                     options,
                      etux_fstree_filter_fn * filter,
                      etux_fstree_cmp_fn *    compare,
                      etux_fstree_handle_fn * handle,
                      void *                  data)
	__utils_nonull(4, 5);

#endif /* _UTILS_FSTREE_H */
