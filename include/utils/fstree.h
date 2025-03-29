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
#include <dirent.h>

/**
 * @struct etux_fstree_iter
 * Filesystem tree entry iterator.
 *
 * An opaque structure used to traverse filesystem hierarchies.
 *
 * @see
 * - etux_fstree_iter_depth()
 * - etux_fstree_iter_dir()
 * - etux_fstree_iter_dirfdf()
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
 * During a traversal, retrieve the depth, numbered from 1 to N, for the current
 * filesystem entry. As a consequence, filesystem tree starting point (or root)
 * of the traversal would be numbered 0.
 *
 * @see
 * - #etux_fstree_iter
 */
extern unsigned int
etux_fstree_iter_depth(const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

/**
 * Return pathname of current filesystem traversal root entry.
 *
 * @param[in] iter Filesystem tree entry iterator
 *
 * @return Pointer to the pathname of current filesystem hierarchies traversal
 *         root entry.
 *
 * This roughly corresponds to the @p path argument given to either
 * etux_fstree_walk(), etux_fstree_sort_walk(), etux_fstree_scan() or
 * etux_fstree_sort_scan() functions.
 *
 * @see
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
 * Return current filesystem traversal directory stream.
 *
 * @param[in] iter Filesystem tree entry iterator
 *
 * @return Pointer to the current filesystem traversal directory stream.
 *
 * While traversing a filesystem tree, return the directory stream related to
 * the current entry's parent directory.
 *
 * @see
 * - #etux_fstree_iter
 * - etux_fstree_iter_dirfd()
 */
extern DIR *
etux_fstree_iter_dir(const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__returns_nonull
	__warn_result;

/**
 * Return current filesystem traversal directory file descriptor.
 *
 * @param[in] iter Filesystem tree entry iterator
 *
 * @return Current filesystem traversal directory file descriptor.
 *
 * While traversing a filesystem tree, return the file descriptor related to
 * the current entry's parent directory.
 *
 * @see
 * - #etux_fstree_iter
 * - etux_fstree_iter_dir()
 */
extern int
etux_fstree_iter_dirfd(const struct etux_fstree_iter * __restrict iter)
	__utils_nonull(1)
	__utils_pure
	__utils_nothrow
	__leaf
	__warn_result;

/**
 * @struct etux_fstree_entry
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
 * - etux_fstree_entry_sized_path()
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
 * Retrieve pathname to filesystem tree traversal entry given in argument.
 *
 * @param[in]  entry Filesystem tree entry
 * @param[in]  iter  Filesystem tree entry iterator
 * @param[out] path  Output path
 * @param[in]  size  Size of @p path in bytes
 *
 * @return `>0` when successful, a negative errno-like return code otherwise.
 * @retval >0            Length of retrieved path
 * @retval -ENAMETOOLONG Retrieved path too larget to fit into @p path
 *
 * This function retrieves the pathname relative to the root of the traversal
 * for the filesystem tree entry specified as @p entry and places it into the
 * string specified as the @p path argument.
 *
 * The @p size argument should specify the available size of the @p path string.
 * It is guaranteed that no more than @p size bytes (including the terminating
 * `NULL` byte) are copied into @p path string.
 *
 * Length of the string returned (excluding the terminating `NULL` byte) is
 * guaranteed to be `> 0` and `< PATH_MAX` as defined by `<limits.h>`.
 * In other words, the maximum required size for the @p path string (including
 * the terminating `NULL` byte) is `<= PATH_MAX` as defined by `<limits.h>`.
 *
 * Content of the @p path string may differ according to @p iter filesystem
 * tree iterator configuration. See #ETUX_FSTREE_FOLLOW_OPT for more
 * informations.
 *
 * @see
 * - etux_fstree_entry_path()
 * - etux_fstree_entry_name()
 * - etux_fstree_entry_slink()
 * - #etux_fstree_entry
 * - #etux_fstree_iter
 * - #ETUX_FSTREE_FOLLOW_OPT
 * - @man{errno(3)}
 */
extern ssize_t
etux_fstree_entry_sized_path(const struct etux_fstree_entry * __restrict entry,
                             const struct etux_fstree_iter * __restrict  iter,
                             char * __restrict                           path,
                             size_t                                      size)
	__utils_nonull(1, 2, 3) __utils_nothrow __leaf __warn_result;

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
 *   returned by etux_fstree_entry_type(). A `NULL` pointer is returned
 *   otherwise and @man{errno(3)} is set to `EINVAL`.
 * - when @p iter filesystem tree iterator is configured with the
 *   #ETUX_FSTREE_FOLLOW_OPT option, no symbolic link is ever traversed ; in
 *   this case, etux_fstree_entry_slink() always returns `NULL`...
 *
 * @see
 * - etux_fstree_entry_sized_slink()
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
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

/**
 * Retrieve target of filesystem tree traversal symbolic link entry given in
 * argument.
 *
 * @param[in]  entry  Filesystem tree entry
 * @param[in]  iter   Filesystem tree entry iterator
 * @param[out] target Output symbolic link target path
 * @param[in]  size   Size of @p target in bytes
 *
 * @return `>0` when successful, a negative errno-like return code otherwise.
 * @retval >0            Length of retrieved symbolic link target path
 * @retval -ENODATA      Empty symbolic link target path found
 * @retval -ENAMETOOLONG Symbolic link target path too larget to fit into @p
 *                       target
 *
 * This function retrieves the symbolic link target pathname for the filesystem
 * tree entry specified as @p entry and places it into the string specified as
 * the @p target argument.
 *
 * The @p size argument should specify the available size of the @p target
 * string. It is guaranteed that no more than @p size bytes (including the
 * terminating `NULL` byte) are copied into @p target string.
 * In addition, the maximum required size for the @p target string (including
 * the terminating `NULL` byte) is `<= PATH_MAX` as defined by `<limits.h>`.
 *
 * @note
 * - @p entry *MUST* be a symbolic link, i.e., its type *MUST* be `DT_LNK` as
 *   returned by etux_fstree_entry_type(). A `NULL` pointer is returned
 *   otherwise and @man{errno(3)} is set to `EINVAL`.
 * - when @p iter filesystem tree iterator is configured with the
 *   #ETUX_FSTREE_FOLLOW_OPT option, no symbolic link is ever traversed ; in
 *   this case, etux_fstree_entry_slink() always returns `-EINVAL`...
 *
 * @see
 * - etux_fstree_entry_slink()
 * - etux_fstree_entry_type()
 * - etux_fstree_entry_name()
 * - etux_fstree_entry_path()
 * - #etux_fstree_entry
 * - #etux_fstree_iter
 * - #ETUX_FSTREE_FOLLOW_OPT
 * - @man{errno(3)}
 */
extern ssize_t
etux_fstree_entry_sized_slink(
	const struct etux_fstree_entry * __restrict entry,
	const struct etux_fstree_iter * __restrict  iter,
	char * __restrict                           target,
	size_t                                      size)
	__utils_nonull(1, 2, 3) __utils_nothrow __leaf __warn_result;

/**
 * @defgroup etux_fstree_cmds-group Filesystem tree traversal options
 * @{
 */

/**
 * Tell the filesystem tree traversal to keep going.
 *
 * When used as the return value of an #etux_fstree_handle_fn entry handler
 * function, this tells the filesystem traversal logic to keep iterating over
 * next entry.
 *
 * When used as the return value of a #etux_fstree_filter_fn filtering function,
 * this tells the filesystem traversal logic to include the current entry within
 * the list of entries to be sorted.
 *
 * @see
 * - #etux_fstree_handle_fn
 * - #etux_fstree_filter_fn
 * - @rstref{etux_fstree_cmds-group}
 */
#define ETUX_FSTREE_CONT_CMD (0)

/**
 * Tell the filesystem tree traversal to stop.
 *
 * When used as the return value of an #etux_fstree_handle_fn entry handler
 * function or a #etux_fstree_filter_fn filtering function, this tells the
 * filesystem traversal logic to abort the iteration process.
 *
 * @see
 * - #etux_fstree_handle_fn
 * - #etux_fstree_filter_fn
 * - @rstref{etux_fstree_cmds-group}
 */
#define ETUX_FSTREE_STOP_CMD (1)

/**
 * Tell the filesystem tree traversal to skip current entry.
 *
 * When used as the return value of an #etux_fstree_filter_fn filtering
 * function, this tells the filesystem traversal logic to exclude the current
 * entry from the set of directory entries to be sorted.
 *
 * @see
 * - #etux_fstree_filter_fn
 * - @rstref{etux_fstree_cmds-group}
 */
#define ETUX_FSTREE_SKIP_CMD (2)

/**
 * @}
 */

/**
 * Filesystem traversal entry filtering routine signature.
 *
 * Use this to implement a function that filters entries in or out during a
 * etux_fstree_sort_walk() or etux_fstree_sort_scan() filesystem tree ordered
 * traversal.
 *
 * etux_fstree_sort_walk() and etux_fstree_sort_scan() call this function before
 * sorting current iteration directory entries.
 *
 * The function is called with:
 * - the **first** argument pointing to the current iteration *entry*,
 * - the **second** argument pointing to the underlying filesystem traversal
 *   *iterator*,
 * - and the **third** argument pointing to the optional user provided @p data
 *   argument given to one of the top-level ordered traversal functions
 *   etux_fstree_sort_walk() and etux_fstree_sort_scan().
 *
 * This function is *EXPECTED* to return one of the following:
 * - a value described in the @rstref{etux_fstree_cmds-group} section to inform
 *   the traversal logic of the *filtering result* ;
 * - or a negative errno-like value to inform the traversal logic that an
 *   *unrecoverable error* happened and that iteration should be *aborted*
 *   immediately.
 *
 * @see
 * - @rstref{etux_fstree_cmds-group}
 * - etux_fstree_sort_walk()
 * - etux_fstree_sort_scan()
 */
typedef int
        etux_fstree_filter_fn(struct etux_fstree_entry *,
                              const struct etux_fstree_iter *,
                              void *);

/**
 * Filesystem traversal entry comparison routine signature.
 *
 * Use this to implement a function that compares 2 filesystem entries
 * given as arguments according to an arbitrary logic during one of the ordered
 * traversal operations etux_fstree_sort_walk() and etux_fstree_sort_scan().
 *
 * The function is called with:
 * - the **first** argument pointing to the *first entry* to be sorted,
 * - the **second** one pointing to the *second entry* to be sorted,
 * - the **third** one pointing to the underlying filesystem traversal
 *   *iterator*,
 * - and the *last* one, pointing to the optional user provided @p data argument
 *   given to one of the top-level ordered traversal functions
 *   etux_fstree_sort_walk() and etux_fstree_sort_scan().
 *
 * The function *MUST* return an integer less than, equal to, or greater than
 * zero if first entry is found, respectively, to be less than, to match, or
 * be greater than the second one.
 *
 * @see
 * - etux_fstree_sort_walk()
 * - etux_fstree_sort_scan()
 */
typedef int
        etux_fstree_cmp_fn(struct etux_fstree_entry *,
                           struct etux_fstree_entry *,
                           const struct etux_fstree_iter *,
                           void *);

/**
 * Filesystem tree traversal event.
 *
 * An event is passed as *third* argument to an #etux_fstree_handle_fn entry
 * handler function each time an entry is visited during a filesystem tree
 * traversal operation.
 *
 * It describes the circumstances under which an entry is visited.
 *
 * @note
 * The following events may only happen when a filesystem tree traversal
 * operation is started by a call to either etux_fstree_scan() or
 * etux_fstree_sort_scan():
 * - #ETUX_FSTREE_LOOP_EVT,
 * - #ETUX_FSTREE_PRE_EVT,
 * - #ETUX_FSTREE_POST_EVT,
 * - #ETUX_FSTREE_DIR_ERR_EVT.
 *
 * @see
 * #etux_fstree_handle_fn
 */
enum etux_fstree_event {
	/**
	 * A loaded and valid filesystem tree entry is being visited.
	 *
	 * Happens when a non directory entry has been successfully loaded
	 * during a filesystem tree traversal operation started thanks to a call
	 * to etux_fstree_walk(), etux_fstree_sort_walk(), etux_fstree_scan() or
	 * etux_fstree_sort_scan().
	 *
	 * In this case, the #etux_fstree_handle_fn callback is called with the
	 * following arguments:
	 * - the **first** one points to a valid #etux_fstree_entry *entry* ;
	 * - the **second** one points to a valid #etux_fstree_iter *iterator* ;
	 * - the **third** one, the #etux_fstree_event visiting *event*, equals
	 *   #ETUX_FSTREE_ENT_EVT ;
	 * - the **fourth** one, the visiting error *status*, equals `0`,
	 *   meaning success ;
	 * - and the **last** one points to the optional user provided @p data
	 *   argument given to the top-level traversal function.
	 */
	ETUX_FSTREE_ENT_EVT = 0,

	/**
	 * Failed to retrieve next filesystem tree entry.
	 *
	 * Happens when the next entry cannot be retrieved during a filesystem
	 * tree traversal operation started thanks to a call to
	 * etux_fstree_walk(), etux_fstree_sort_walk(), etux_fstree_scan() or
	 * etux_fstree_sort_scan().
	 *
	 * In this case, the #etux_fstree_handle_fn callback is called with the
	 * following arguments:
	 * - the **first** one points to a `NULL` *entry* ;
	 * - the **second** one points to a valid #etux_fstree_iter *iterator* ;
	 * - the **third** one, the #etux_fstree_event visiting *event*, equals
	 *   #ETUX_FSTREE_NEXT_ERR_EVT ;
	 * - the **fourth** one, the visiting error *status*, contains a
	 *   negative errno-like value describing the cause of failure ;
	 * - and the **last** one points to the optional user provided @p data
	 *   argument given to the top-level traversal function.
	 *
	 * @warning
	 * In this case, the value returned by the #etux_fstree_handle_fn
	 * callback function is treated in a special way. It is *REQUIRED* to be
	 * one of the following:
	 * - a negative errno-like value that aborts the ongoing iteration /
	 *   scanning process ;
	 * - or #ETUX_FSTREE_STOP_CMD to tell the caller to interrupt the
	 *   current directory iteration and try to keep scanning the ongoing
	 *   filesystem hierarchy in cases where the traversal operation was
	 *   started thanks to a call to etux_fstree_scan() or
	 *   etux_fstree_sort_scan().
	 *
	 * The traversal process behavior is *undefined* for any other value
	 * returned.
	 */
	ETUX_FSTREE_NEXT_ERR_EVT,

	/**
	 * Failed to load properties of current filesystem tree entry.
	 *
	 * Happens when properties of an entry could not be loaded during a
	 * filesystem tree traversal operation started thanks to a call to
	 * etux_fstree_walk(), etux_fstree_sort_walk(), etux_fstree_scan() or
	 * etux_fstree_sort_scan().
	 *
	 * In this case, the #etux_fstree_handle_fn callback is called with the
	 * following arguments:
	 * - the **first** one *MAY* point to a valid #etux_fstree_entry *entry*
	 *   (see the warning below) ;
	 * - the **second** one points to a valid #etux_fstree_iter *iterator* ;
	 * - the **third** one, the #etux_fstree_event visiting *event*, equals
	 *   #ETUX_FSTREE_LOAD_ERR_EVT ;
	 * - the **fourth** one, the visiting error *status*, contains a
	 *   negative errno-like value describing the cause of failure ;
	 * - and the **last** one points to the optional user provided @p data
	 *   argument given to the top-level traversal function.
	 *
	 * @warning
	 * The **first** argument points to a `NULL` entry when @p status value
	 * is different from `-ENODATA' and `-ENAMETOOLONG`. It points to a
	 * valid entry otherwise.
	 */
	ETUX_FSTREE_LOAD_ERR_EVT,

	/**
	 * Filesystem tree traversal loop detected.
	 *
	 * Happens when an entry is a link that points to an ancestor of the
	 * current entry during a filesystem tree traversal operation started
	 * thanks to a call to etux_fstree_scan() or etux_fstree_sort_scan().
	 *
	 * In this case, the #etux_fstree_handle_fn callback is called with the
	 * following arguments:
	 * - the **first** one points to a valid #etux_fstree_entry *entry* ;
	 * - the **second** one points to a valid #etux_fstree_iter *iterator* ;
	 * - the **third** one, the #etux_fstree_event visiting *event*, equals
	 *   #ETUX_FSTREE_LOOP_EVT ;
	 * - the **fourth** one, the visiting error *status*, equals `0`,
	 *   meaning success ;
	 * - and the **last** one points to the optional user provided @p data
	 *   argument given to the top-level traversal function.
	 */
	ETUX_FSTREE_LOOP_EVT,

	/**
	 * While descending down a filesystem tree, a directory entry is being
	 * visited.
	 *
	 * Happens when a `DT_DIR` directory entry has been successfully loaded
	 * during a filesystem tree traversal operation started thanks to a call
	 * to etux_fstree_scan() or etux_fstree_sort_scan() with the
	 * #ETUX_FSTREE_PRE_OPT @p options enabled.
	 *
	 * The #etux_fstree_handle_fn callback is then called a first time while
	 * descending down the hierarchy with the following arguments:
	 * - the **first** one points to a valid #etux_fstree_entry *entry* ;
	 * - the **second** one points to a valid #etux_fstree_iter *iterator* ;
	 * - the **third** one, the #etux_fstree_event visiting *event*, equals
	 *   #ETUX_FSTREE_PRE_EVT ;
	 * - the **fourth** one, the visiting error *status*, equals `0`,
	 *   meaning success ;
	 * - and the **last** one points to the optional user provided @p data
	 *   argument given to the top-level traversal function.
	 *
	 * Note that, when enabled thanks to the #ETUX_FSTREE_POST_OPT option,
	 * the same entry may also be visited with the #ETUX_FSTREE_POST_EVT
	 * event while travelling back up towards the initial root directory.
	 */
	ETUX_FSTREE_PRE_EVT,

	/**
	 * While ascending back up a filesystem tree, a directory entry is being
	 * visited.
	 *
	 * Happens when a `DT_DIR` directory entry has been successfully loaded
	 * during a filesystem tree traversal operation started thanks to a call
	 * to etux_fstree_scan() or etux_fstree_sort_scan() with the
	 * #ETUX_FSTREE_POST_OPT @p options enabled.
	 *
	 * The #etux_fstree_handle_fn callback is then called a first time while
	 * descending down the hierarchy with the following arguments:
	 * - the **first** one points to a valid #etux_fstree_entry *entry* ;
	 * - the **second** one points to a valid #etux_fstree_iter *iterator* ;
	 * - the **third** one, the #etux_fstree_event visiting *event*, equals
	 *   #ETUX_FSTREE_POST_EVT ;
	 * - the **fourth** one, the visiting error *status*, equals `0`,
	 *   meaning success ;
	 * - and the **last** one points to the optional user provided @p data
	 *   argument given to the top-level traversal function.
	 *
	 * Note that, when enabled thanks to the #ETUX_FSTREE_PRE_OPT option,
	 * the same entry may already have been visited  with the
	 * #ETUX_FSTREE_PRE_EVT event while descending down the filesystem tree
	 * hierarchy.
	 */
	ETUX_FSTREE_POST_EVT,

	/**
	 * Failed to descend down a filesystem directory entry.
	 *
	 * Happens when a `DT_DIR` directory entry, that has been successfully
	 * loaded, could not be entered into during a filesystem tree traversal
	 * operation started thanks to a call to etux_fstree_scan() or
	 * etux_fstree_sort_scan().
	 *
	 * The #etux_fstree_handle_fn callback is then called with the following
	 * arguments:
	 * - the **first** one points to a valid #etux_fstree_entry *entry* ;
	 * - the **second** one points to a valid #etux_fstree_iter *iterator* ;
	 * - the **third** one, the #etux_fstree_event visiting *event*, equals
	 *   #ETUX_FSTREE_DIR_ERR_EVT ;
	 * - the **fourth** one, the visiting error *status*, contains a
	 *   negative errno-like value describing the cause of failure ;
	 * - and the **last** one points to the optional user provided @p data
	 *   argument given to the top-level traversal function.
	 */
	ETUX_FSTREE_DIR_ERR_EVT,

	/**
	 * End of #etux_fstree_event event enumeration marker.
	 */
	ETUX_FSTREE_EVT_NR
};

/**
 * Filesystem traversal entry handler routine signature.
 *
 * Use this to implement a function that is called when a traversal operation
 * visits a filesystem tree entry.
 *
 * This function is called with:
 * - the **first** argument pointing to the current iteration *entry*,
 * - the **second** one pointing to the underlying filesystem traversal
 *   *iterator*,
 * - the **third** one being the #etux_fstree_event *event* describing the
 *   circumstances under which the entry is visited,
 * - the **fourth** one describing the error *status* related to the entry being
 *   visited,
 * - and the **last** one, pointing to the optional user provided @p data
 *   argument given to one of the top-level traversal functions
 *   etux_fstree_walk(), etux_fstree_sort_walk(), etux_fstree_scan() and
 *   etux_fstree_sort_scan().
 *
 * This function is *EXPECTED* to return one of the following:
 * - a value described in the @rstref{etux_fstree_cmds-group} section to
 *   control the traversal process,
 * - or a negative errno-like value to inform the traversal logic that an
 *   *unrecoverable error* happened and that iteration / scanning should be
 *   *aborted* immediately.
 *
 * See individual #etux_fstree_event events description for additional details
 * about #etux_fstree_handle_fn calling conventions.
 *
 * @see
 * - #etux_fstree_event
 * - #etux_fstree_entry
 * - #etux_fstree_iter
 * - etux_fstree_walk()
 * - etux_fstree_sort_walk()
 * - etux_fstree_scan()
 * - etux_fstree_sort_scan()
 */
typedef int
        etux_fstree_handle_fn(struct etux_fstree_entry *,
                              const struct etux_fstree_iter *,
                              enum etux_fstree_event,
                              int,
                              void *);
/**
 * @defgroup etux_fstree_opts-group Filesystem tree traversal options
 * @{
 */

/**
 * Follow symbolic links while traversing a filesystem tree.
 *
 * This option requests the filesystem tree traversal logic to follow symbolic
 * links when resolving directory entries.
 *
 * You may specify this option within the mask given as @p options argument to
 * etux_fstree_walk(), etux_fstree_sort_walk(), etux_fstree_scan() or
 * etux_fstree_sort_scan() functions.
 *
 * When this option is set, an entry passed to any of the callback pointer(s)
 * given to the functions mentioned above can never be a symbolic link.
 *
 * @see
 * - etux_fstree_walk()
 * - etux_fstree_sort_walk()
 * - etux_fstree_scan()
 * - etux_fstree_sort_scan()
 * - @rstref{etux_fstree_opts-group}
 * - #etux_fstree_handle_fn
 * - #etux_fstree_filter_fn
 * - #etux_fstree_cmp_fn
 */
#define ETUX_FSTREE_FOLLOW_OPT (1 << 0)

/**
 * Recurse across devices while traversing a filesystem tree.
 *
 * This option allows the filesystem tree traversal logic to recurse into
 * directories that have a different device number than the root entry from
 * which the descent began.
 *
 * Simply put, this option allows to cross filesystem mount point boundaries
 * during traversal.
 *
 * You may specify this option within the mask given as @p options argument to
 * etux_fstree_scan() or etux_fstree_sort_scan() functions.
 *
 * @see
 * - etux_fstree_scan()
 * - etux_fstree_sort_scan()
 * - @rstref{etux_fstree_opts-group}
 * - #etux_fstree_handle_fn
 * - #etux_fstree_filter_fn
 * - #etux_fstree_cmp_fn
 */
#define ETUX_FSTREE_XDEV_OPT   (1 << 1)

/**
 * Run entry handler when root of a filesystem traversal subtree is visited
 * first.
 *
 * This option requests the filesystem tree traversal logic to call the
 * #etux_fstree_handle_fn entry callback function when the root of a subtree is
 * visited first, i.e. while descending down a `DT_DIR` directory entry.
 *
 * You may specify this option within the mask given as @p options argument to
 * etux_fstree_scan() or etux_fstree_sort_scan() functions.
 *
 * This option may also be combined with the #ETUX_FSTREE_POST_OPT option to
 * call the #etux_fstree_handle_fn callback at both descending and ascending
 * time.
 *
 * @see
 * - etux_fstree_scan()
 * - etux_fstree_sort_scan()
 * - #etux_fstree_handle_fn
 * - #ETUX_FSTREE_POST_OPT
 * - @rstref{etux_fstree_opts-group}
 */
#define ETUX_FSTREE_PRE_OPT    (1 << 2)

/**
 * Run entry handler when root of a filesystem traversal subtree is visited
 * last.
 *
 * This option requests the filesystem tree traversal logic to call the
 * #etux_fstree_handle_fn entry callback function when the root of a subtree is
 * visited last, i.e., while travelling back up a `DT_DIR` directory entry once
 * all of its child entries have already been visited.
 *
 * You may specify this option within the mask given as @p options argument to
 * etux_fstree_scan() or etux_fstree_sort_scan() functions.
 *
 * This option may also be combined with the #ETUX_FSTREE_PRE_OPT option to
 * call the #etux_fstree_handle_fn callback at both descending and ascending
 * time.
 *
 * @see
 * - etux_fstree_scan()
 * - etux_fstree_sort_scan()
 * - #etux_fstree_handle_fn
 * - #ETUX_FSTREE_PRE_OPT
 * - @rstref{etux_fstree_opts-group}
 */
#define ETUX_FSTREE_POST_OPT   (1 << 3)

/**
 * @}
 */

/**
 * Iterate over filesystem directory entries.
 *
 * @param[in] path    Pathname to directory which entries to iterate over
 * @param[in] options Iteration option mask
 * @param[in] handle  Entry handler callback
 * @param[in] data    Optional arbitrary user data
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 *
 * This function iterates over filesystem entries found under the directory
 * which pathname is given as the @p path argument.
 *
 * Iteration is performed according to the @p options argument which should be
 * set as a mask of options as specified in the @rstref{etux_fstree_opts-group}
 * section.
 *
 * The @p handle entry handler callback is called once for every entry found
 * under the directory identified by @p path.
 * It is given the optional @p data pointer to an arbitrary user provided memory
 * area.
 * The #etux_fstree_handle_fn section describes the @p handle function pointer
 * calling conventions.
 *
 * @remark
 * - The @p path argument may be passed as `NULL` or an empty C string in which
 *   case this function iterates into the current working directory.
 * - etux_fstree_walk() supports the #ETUX_FSTREE_FOLLOW_OPT option only.
 * - The only possible #etux_fstree_event event that etux_fstree_walk() may pass
 *   to @p handle is #ETUX_FSTREE_ENT_EVT.
 *
 * @see
 * - #ETUX_FSTREE_FOLLOW_OPT
 * - #ETUX_FSTREE_ENT_EVT
 * - #etux_fstree_handle_fn
 * - etux_fstree_sort_walk()
 * - etux_fstree_scan()
 * - @man{errno(3)}
 * - @man{getcwd(3)}, @man{pwd(1)}
 */
extern int
etux_fstree_walk(const char * __restrict path,
                 int                     options,
                 etux_fstree_handle_fn * handle,
                 void *                  data)
	__utils_nonull(3);

/**
 * Sort and iterate over filesystem directory entries.
 *
 * @param[in] path    Pathname to directory which entries to iterate over
 * @param[in] options Iteration option mask
 * @param[in] filter  Optional entry filtering callback
 * @param[in] compare Entry order comparison callback
 * @param[in] handle  Entry handler callback
 * @param[in] data    Optional arbitrary user data
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 *
 * This function iterates over filesystem entries found under the directory
 * which pathname is given as the @p path argument in sorted order.
 *
 * Iteration is performed according to the @p options argument which should be
 * set as a mask of options as specified in the @rstref{etux_fstree_opts-group}
 * section.
 *
 * Entries are sorted according to the callback function given as argument @p
 * compare.
 * Prior to the sorting phase, entries may be filtered in or out thanks to the
 * optional @p filter callback function.
 *
 * Once filtered and sorted, the @p handle entry handler callback is called once
 * for every entry found under the directory identified by @p path.
 *
 * All callback functions specified to etux_fstree_sort_walk() are given
 * the @p data argument that should point to an optionall arbitrary user
 * provided memory area. Calling conventions of callback functions are described
 * in the following sections:
 * - @p handle : #etux_fstree_handle_fn,
 * - @p compare : #etux_fstree_cmp_fn,
 * - @p filter : #etux_fstree_filter_fn.
 *
 * @remark
 * - The @p path argument may be passed as `NULL` or an empty C string in which
 *   case this function iterates into the current working directory.
 * - etux_fstree_sort_walk() supports the #ETUX_FSTREE_FOLLOW_OPT option only.
 * - The only possible #etux_fstree_event event that etux_fstree_walk() may pass
 *   to @p handle is #ETUX_FSTREE_ENT_EVT.
 *
 * @see
 * - #ETUX_FSTREE_FOLLOW_OPT
 * - #ETUX_FSTREE_ENT_EVT
 * - #etux_fstree_handle_fn
 * - #etux_fstree_filter_fn
 * - #etux_fstree_cmp_fn
 * - etux_fstree_walk()
 * - etux_fstree_sort_scan()
 * - @man{errno(3)}
 * - @man{getcwd(3)}, @man{pwd(1)}
 */
extern int
etux_fstree_sort_walk(const char * __restrict path,
                      int                     options,
                      etux_fstree_filter_fn * filter,
                      etux_fstree_cmp_fn *    compare,
                      etux_fstree_handle_fn * handle,
                      void *                  data)
	__utils_nonull(4, 5);

/**
 * Scan a filesystem hierarchy.
 *
 * @param[in] path    Pathname to root directory of hierarchy to scan
 * @param[in] options Iteration option mask
 * @param[in] handle  Entry handler callback
 * @param[in] data    Optional arbitrary user data
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 *
 * This function allows to traverse the whole filesystem hierarchy found under
 * the directory which pathname is given as the @p path argument.
 *
 * Traversal is performed according to the @p options argument which should be
 * set as a mask of options as specified in the @rstref{etux_fstree_opts-group}
 * section.
 *
 * The @p handle entry handler callback is called once for every entry
 * found under the current directory (but see below for directory entries).
 * It is given the optional @p data pointer to an arbitrary user provided memory
 * area.
 * The #etux_fstree_handle_fn section describes the @p handle function pointer
 * calling conventions.
 *
 * Note that `DT_DIR` *directory* entries are processed in a specific way since,
 * in addition to calling the @p handle callback, the traversal logic needs to
 * change to directories.
 * Hence, directory entries are *always visited twice*: once while descending
 * down the hierarchy, and once while travelling back up. The @p handle callback
 * is then called in the following manner:
 * - when the #ETUX_FSTREE_PRE_OPT option is enabled into the @p options
 *   argument, @p handle is called once while descending down, just before
 *   entering a directory entry ;
 * - when the #ETUX_FSTREE_POST_OPT option is enabled into the @p options
 *   argument, @p handle is called once while travelling back up, just after
 *   changing to the parent directory of the visited directory entry ;
 *
 * Also note that when neither of these options are enabled, the @p handle
 * callback function is not called at all for directory entries.
 *
 * @remark
 * - The @p path argument may be passed as `NULL` or an empty C string in which
 *   case this function iterates into the current working directory.
 *
 * @see
 * - @rstref{etux_fstree_opts-group}
 * - #etux_fstree_handle_fn
 * - etux_fstree_sort_scan()
 * - etux_fstree_walk()
 * - @man{errno(3)}
 * - @man{getcwd(3)}, @man{pwd(1)}
 */
extern int
etux_fstree_scan(const char * __restrict path,
                 int                     options,
                 etux_fstree_handle_fn * handle,
                 void *                  data)
	__utils_nonull(3);

/**
 * Sort and scan a filesystem hierarchy.
 *
 * @param[in] path    Pathname to root directory of hierarchy to scan
 * @param[in] options Scanning option mask
 * @param[in] filter  Optional entry filtering callback
 * @param[in] compare Entry order comparison callback
 * @param[in] handle  Entry handler callback
 * @param[in] data    Optional arbitrary user data
 *
 * @return `0` when successful, a negative errno-like return code otherwise.
 *
 * This function allows to traverse the whole filesystem hierarchy found under
 * the directory which pathname is given as the @p path argument.
 *
 * Traversal is performed according to the @p options argument which should be
 * set as a mask of options as specified in the @rstref{etux_fstree_opts-group}
 * section.
 *
 * For each directory, entries are sorted according to the callback function
 * given as argument @p compare.
 * Prior to the sorting phase, entries may be filtered in or out thanks to the
 * optional @p filter callback function.
 *
 * The @p handle entry handler callback is called once for every entry
 * found under the current directory (but see below for directory entries).
 * It is given the optional @p data pointer to an arbitrary user provided memory
 * area.
 * The #etux_fstree_handle_fn section describes the @p handle function pointer
 * calling conventions.
 *
 * Note that `DT_DIR` *directory* entries are processed in a specific way since,
 * in addition to calling the @p handle callback, the traversal logic needs to
 * change to directories.
 * Hence, directory entries are *always visited twice*: once while descending
 * down the hierarchy, and once while travelling back up. The @p handle callback
 * is then called in the following manner:
 * - when the #ETUX_FSTREE_PRE_OPT option is enabled into the @p options
 *   argument, @p handle is called once while descending down, just before
 *   entering a directory entry ;
 * - when the #ETUX_FSTREE_POST_OPT option is enabled into the @p options
 *   argument, @p handle is called once while travelling back up, just after
 *   changing to the parent directory of the visited directory entry ;
 *
 * Also note that when neither of these options are enabled, the @p handle
 * callback function is not called at all for directory entries.
 *
 * @remark
 * - The @p path argument may be passed as `NULL` or an empty C string in which
 *   case this function iterates into the current working directory.
 *
 * @see
 * - @rstref{etux_fstree_opts-group}
 * - #etux_fstree_handle_fn
 * - #etux_fstree_filter_fn
 * - #etux_fstree_cmp_fn
 * - etux_fstree_scan()
 * - etux_fstree_sort_walk()
 * - @man{errno(3)}
 * - @man{getcwd(3)}, @man{pwd(1)}
 */
extern int
etux_fstree_sort_scan(const char * __restrict path,
                      int                     options,
                      etux_fstree_filter_fn * filter,
                      etux_fstree_cmp_fn *    compare,
                      etux_fstree_handle_fn * handle,
                      void *                  data)
	__utils_nonull(4, 5);

#endif /* _UTILS_FSTREE_H */
