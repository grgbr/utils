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

/**
 * Filesystem tree entry iterator.
 *
 * An opaque structure used traverse filesystem hierarchies.
 *
 * @see
 * - etux_fstree_iter_depth()
 * - etux_fstree_iter_path()
 * - etux_fstree_filter_fn
 * - etux_fstree_cmp_fn
 * - etux_fstree_handle_fn
 */
struct etux_fstree_iter;

/**
 * Return depth of current filesystem hierarchy traversal entry.
 *
 * @param[in] iter Filesysem tree entry iterator
 *
 * The depth of the traversal, numbered from 1 to N, where the current
 * filesystem entry was found. As a consequence, filesystem tree starting point
 * (or root) of the traversal would be numbered 0.
 *
 * @see
 * - etux_fstree_iter_path()
 * - #etux_fstree_iter
 */
extern unsigned int
etux_fstree_iter_depth(const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

/**
 * Return pathname of current filesystem hierarchy traversal root entry.
 *
 * @param[in] iter Filesystem tree entry iterator
 *
 * @eturn Pointer to the pathname of current filesystem hierarchies traversal
 *        root entry.
 *
 * This roughly corresponds to the @p path argument given to either
 * etux_fstree_iter(), etux_fstree_sort_iter(), etux_fstree_scan() or
 * etux_fstree_sort_scan() functions.
 *
 * @see
 * - etux_fstree_iter_depth()
 * - #etux_fstree_iter
 */
extern const char *
etux_fstree_iter_path(const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__returns_nonull
	__warn_result;

/**
 * Filesystem tree traversal entry.
 *
 * An opaque structure describing a filesystem tree traversal entry.
 *
 * @see
 * - etux_fstree_entry_type()
 * - etux_fstree_entry_isdot()
 * - etux_fstree_entry_stat()
 * - etux_fstree_entry_name()
 * - etux_fstree_entry_path()
 * - etux_fstree_entry_slink()
 * - etux_fstree_filter_fn
 * - etux_fstree_cmp_fn
 * - etux_fstree_handle_fn
 */
struct etux_fstree_entry;

/**
 * Retrieve type of filesystem tree traversal entry.
 *
 * @param[inout] entry Filesystem tree entry
 * @param[in]    iter  Filesystem tree entry iterator
 *
 * @return `> 0` when successful, a negative errno-like return code otherwise.
 * @retval DT_DIR  directory entry type
 * @retval DT_BLK  block device entry type
 * @retval DT_CHR  character device entry type
 * @retval DT_FIFO FIFO / named @man{pipe(7)} entry type
 * @retval DT_LNK  symbolic link entry type
 * @retval DT_REG  regular file entry type
 * @retval DT_SOCK named @man{unix(7)} socket entry type
 * 
 * This function returns the type of filesystem tree entry given as @p entry
 * argument.
 *
 * Value returned may differ according to @p iter filesystem tree iterator
 * configuration. See #ETUX_FSTREE_FOLLOW_OPT for more informations.
 * 
 * @see
 * - #etux_fstree_entry
 * - #etux_fstree_iter
 * - #ETUX_FSTREE_FOLLOW_OPT
 * - @man{readdir(3)}
 */
extern int
etux_fstree_entry_type(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

/**
 * Return wether a filesystem tree traversal entry is a special dot-file or not.
 *
 * @param[inout] entry Filesystem tree entry
 * @param[in]    iter  Filesystem tree entry iterator
 *
 * @retval 0   not a special dot-file entry
 * @retval 1   is a special dot-file entry
 * @retval < 0 a negative errno-like return code otherwise
 * 
 * This function returns wether the filesystem tree entry given as @p entry
 * argument is one of the 2 special entries that are present in each directory
 * and which are:
 * - `.` (a dot) denoting the current directory itself ;
 * - and `..` (two dots) denoting the parent of current directory.
 *
 * @see
 * - etux_fstree_entry_type()
 * - #etux_fstree_entry
 * - #etux_fstree_iter
 */
extern int
etux_fstree_entry_isdot(struct etux_fstree_entry * __restrict      entry,
                        const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

/**
 * Retrieve filesystem tree traversal entry system properties.
 *
 * @param[inout] entry Filesystem tree entry
 * @param[in]    iter  Filesystem tree entry iterator
 *
 * @return A pointer to a @man{stat(2)} structure in case of success, `NULL`
 *         otherwise, in which case @man{errno(3)} is set appropriately.
 *
 * This function returns informations about the filesystem tree entry specified
 * as @p entry.
 *
 * Content of the returned @man{stat(2)} structure may differ according to
 * @p iter filesystem tree iterator configuration. See #ETUX_FSTREE_FOLLOW_OPT
 * for more informations.
 *
 * @see
 * - etux_fstree_entry_type()
 * - #etux_fstree_entry
 * - #etux_fstree_iter
 * - #ETUX_FSTREE_FOLLOW_OPT
 * - @man{stat(2)}
 * - @man{errno(3)}
 */
extern const struct stat *
etux_fstree_entry_stat(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

/**
 * Return basename of filesystem tree traversal entry given in argument.
 *
 * @param[in] entry Filesystem tree entry
 * @param[in] iter  Filesystem tree entry iterator
 *
 * @return A pointer to C string in case of success, `NULL` otherwise, in
 *         which case @man{errno(3)} is set appropriately.
 *
 * This function returns the basename of the filesystem tree entry specified as
 * @p entry.
 *
 * Length of the string returned (excluding the terminating `NULL` byte) is
 * guaranteed to be `> 0` and `<= NAME_MAX` as defined by `<limits.h>`.
 *
 * Content of the returned string may differ according to @p iter filesystem
 * tree iterator configuration. See #ETUX_FSTREE_FOLLOW_OPT for more
 * informations.
 *
 * @see
 * - etux_fstree_entry_path()
 * - etux_fstree_entry_slink()
 * - #etux_fstree_entry
 * - #etux_fstree_iter
 * - #ETUX_FSTREE_FOLLOW_OPT
 * - @man{stat(2)}
 * - @man{errno(3)}
 */
extern const char *
etux_fstree_entry_name(
	const struct etux_fstree_entry * __restrict entry,
	const struct etux_fstree_iter * __restrict  iter __unused)
	__utils_nonull(1, 2) __utils_pure
	__utils_nothrow
	__leaf
	__returns_nonull
	__warn_result;

/**
 * Return pathname to filesystem tree traversal entry given in argument.
 *
 * @param[inout] entry Filesystem tree entry
 * @param[in]    iter  Filesystem tree entry iterator
 *
 * @return A pointer to C string in case of success, `NULL` otherwise, in which
 * case @man{errno(3)} is set appropriately.
 *
 * This function returns the pathname relative to the root of the traversal for
 * the filesystem tree entry specified as @p entry.
 *
 * Length of the string returned (excluding the terminating `NULL` byte) is
 * guaranteed to be `> 0` and `< PATH_MAX` as defined by `<limits.h>`.
 *
 * Content of the returned string may differ according to @p iter filesystem
 * tree iterator configuration. See #ETUX_FSTREE_FOLLOW_OPT for more
 * informations.
 *
 * @see
 * - etux_fstree_entry_name()
 * - etux_fstree_entry_slink()
 * - #etux_fstree_entry
 * - #etux_fstree_iter
 * - #ETUX_FSTREE_FOLLOW_OPT
 * - @man{errno(3)}
 */
extern const char *
etux_fstree_entry_path(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

/**
 * Return target of filesystem tree traversal symbolic link entry given in
 * argument.
 *
 * @param[inout] entry Filesystem tree entry
 * @param[in]    iter  Filesystem tree entry iterator
 *
 * @return A pointer to C string in case of success, `NULL` otherwise, in which
 *         case @man{errno(3)} is set appropriately.
 *
 * This function returns the symbolic link target pathname for the filesystem
 * tree entry specified as @p entry.
 *
 * Length of the string returned (excluding the terminating `NULL` byte) is
 * guaranteed to be `> 0` and `< PATH_MAX` as defined by `<limits.h>`.
 *
 * @note
 * - @p entry *MUST* be a symbolic link, i.e., its type *MUST* be `DT_LNK` as
 *   returned by etux_fstree_entry_type(). A `NULL` pointer is returned otherwise
 *   and @man{errno(3)} is set to `EINVAL`.
 * - when @p iter filesystem tree iterator is configured with the
 *   #ETUX_FSTREE_FOLLOW_OPT option, no symbolic link is ever traversed ; in
 *   this case, etux_fstree_entry_slink() always returns `NULL`...
 *
 * @see
 * - etux_fstree_entry_type()
 * - etux_fstree_entry_name()
 * - etux_fstree_entry_path()
 * - #etux_fstree_entry
 * - #etux_fstree_iter
 * - #ETUX_FSTREE_FOLLOW_OPT
 * - @man{errno(3)}
 */
extern const char *
etux_fstree_entry_slink(struct etux_fstree_entry * __restrict      entry,
                        const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1, 2) __warn_result;

typedef int
        (etux_fstree_filter_fn)(struct etux_fstree_entry *,
                                const struct etux_fstree_iter *,
                                void *);

typedef int
        (etux_fstree_cmp_fn)(struct etux_fstree_entry *,
                             struct etux_fstree_entry *,
                             const struct etux_fstree_iter *,
                             void *);

/**
 * Filesystem tree traversal event.
 */
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

#define ETUX_FSTREE_CONT_CMD (0)
#define ETUX_FSTREE_STOP_CMD (1)
#define ETUX_FSTREE_SKIP_CMD (2)

typedef int
        (etux_fstree_handle_fn)(struct etux_fstree_entry *,
                                const struct etux_fstree_iter *,
                                enum etux_fstree_event,
                                int,
                                void *);

/* etux_fstree_iter() and etux_fstree_scan() options */
#define ETUX_FSTREE_FOLLOW_OPT (1 << 0)
/* etux_fstree_scan() only options */
#define ETUX_FSTREE_XDEV_OPT   (1 << 1)
#define ETUX_FSTREE_PRE_OPT    (1 << 2)
#define ETUX_FSTREE_POST_OPT   (1 << 3)

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
