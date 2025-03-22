/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/fstree.h"
#include <stroll/array.h>
#include <stroll/falloc.h>
#include <dirent.h>

#if !defined(_DIRENT_HAVE_D_TYPE)
#error dirent structure is missing support for d_type field. \
       Check your C library !
#endif
#if !defined(_DIRENT_HAVE_D_RECLEN)
#error dirent structure is missing support for d_reclen field. \
       Check your C library !
#endif

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define etux_fstree_assert_api(_expr) \
	stroll_assert("etux:fstree", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define etux_fstree_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#include <stroll/assert.h>

#define etux_fstree_assert_intern(_expr) \
	stroll_assert("etux:fstree", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define etux_fstree_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

#define ETUX_FSTREE_VALID_OPTS \
	(ETUX_FSTREE_FOLLOW_OPT | \
	 ETUX_FSTREE_XDEV_OPT | \
	 ETUX_FSTREE_PRE_OPT | \
	 ETUX_FSTREE_POST_OPT)

enum etux_fstree_flag {
	ETUX_FSTREE_STAT_FLAG  = 1 << 0,
	ETUX_FSTREE_PATH_FLAG  = 1 << 1,
	ETUX_FSTREE_SLINK_FLAG = 1 << 2
};

#define ETUX_FSTREE_VALID_FLAGS \
	(ETUX_FSTREE_STAT_FLAG | ETUX_FSTREE_PATH_FLAG | ETUX_FSTREE_SLINK_FLAG)

struct etux_fstree_iter {
	int          opts;
	DIR *        dir;
	size_t       plen;
	char *       path;
	unsigned int depth;
};

#define etux_fstree_iter_assert_api(_iter) \
	etux_fstree_assert_api(_iter); \
	etux_fstree_assert_api(!((_iter)->opts & ~ETUX_FSTREE_VALID_OPTS)); \
	etux_fstree_assert_api((_iter)->dir); \
	etux_fstree_assert_api((_iter)->plen < PATH_MAX); \
	etux_fstree_assert_api((_iter)->path); \
	etux_fstree_assert_api(strnlen((_iter)->path, PATH_MAX) == \
	                       (_iter)->plen); \
	etux_fstree_assert_api((_iter)->depth)

#define etux_fstree_iter_assert_intern(_iter) \
	etux_fstree_assert_intern(_iter); \
	etux_fstree_assert_intern(!((_iter)->opts & ~ETUX_FSTREE_VALID_OPTS)); \
	etux_fstree_assert_intern((_iter)->dir); \
	etux_fstree_assert_intern((_iter)->plen < PATH_MAX); \
	etux_fstree_assert_intern((_iter)->path); \
	etux_fstree_assert_intern(strnlen((_iter)->path, PATH_MAX) == \
	                          (_iter)->plen); \
	etux_fstree_assert_intern((_iter)->depth)

/******************************************************************************
 * Filesystem tree entry handling.
 ******************************************************************************/

struct etux_fstree_entry {
	struct dirent dirent;
	size_t        nlen;
	int           flags;
	struct stat   stat;
	char *        path;
	char *        slink;
};

#define etux_fstree_entry_assert_api(_ent, _iter) \
	etux_fstree_assert_api(_ent); \
	etux_fstree_iter_assert_api(_iter); \
	etux_fstree_assert_api( \
		etux_fstree_validate_dirent(&(_ent)->dirent, _iter) == \
		(ssize_t)(_ent)->nlen); \
	etux_fstree_assert_api(!((_ent)->flags & ~ETUX_FSTREE_VALID_FLAGS)); \
	etux_fstree_assert_api(!((_ent)->flags & ETUX_FSTREE_PATH_FLAG) || \
	                       (upath_validate_path_name((_ent)->path) > 0)); \
	etux_fstree_assert_api(!((_ent)->flags & ETUX_FSTREE_SLINK_FLAG) || \
	                       (upath_validate_path_name((_ent)->slink) > 0))

#define etux_fstree_entry_assert_intern(_ent, _iter) \
	etux_fstree_assert_intern(_ent); \
	etux_fstree_iter_assert_intern(_iter); \
	etux_fstree_assert_intern( \
		etux_fstree_validate_dirent(&(_ent)->dirent, _iter) == \
		(ssize_t)(_ent)->nlen); \
	etux_fstree_assert_intern(!((_ent)->flags & \
	                            ~ETUX_FSTREE_VALID_FLAGS)); \
	etux_fstree_assert_intern( \
		!((_ent)->flags & ETUX_FSTREE_PATH_FLAG) || \
		(upath_validate_path_name((_ent)->path) > 0)); \
	etux_fstree_assert_intern( \
		!((_ent)->flags & ETUX_FSTREE_SLINK_FLAG) || \
		(upath_validate_path_name((_ent)->slink) > 0))

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
etux_fstree_path_isdot(const char * __restrict path, size_t len)
{
	etux_fstree_assert_intern(upath_validate_path_name(path) > 0);
	etux_fstree_assert_intern(len);

	if ((len <= 2) && (path[0] == '.') && (!path[1] || path[1] == '.'))
		return true;

	return false;
}

static __utils_nonull(1, 2) __utils_pure __utils_nothrow __warn_result
ssize_t
etux_fstree_validate_dirent(const struct dirent * __restrict           dirent,
                            const struct etux_fstree_iter * __restrict iter)
{
	etux_fstree_assert_intern(dirent);
	etux_fstree_iter_assert_intern(iter);

	size_t len;

	len = strnlen(dirent->d_name, NAME_MAX + 1);
	etux_fstree_assert_intern(len <= (NAME_MAX + 1));
	if (!len)
		return -ENODATA;
	else if ((len == (NAME_MAX + 1)) ||
		 ((iter->plen + 1 + len) >= PATH_MAX))
		return -ENAMETOOLONG;

	switch (dirent->d_type) {
	case DT_DIR:
	case DT_UNKNOWN:
		break;

	case DT_BLK:
	case DT_CHR:
	case DT_FIFO:
	case DT_LNK:
	case DT_REG:
	case DT_SOCK:
	case DT_WHT:
		if (etux_fstree_path_isdot(dirent->d_name, len))
			return -EISDIR;
		break;

	default:
		return -ENOTSUP;
	}

	return (ssize_t)len;
}

bool
etux_fstree_entry_isdot(
	const struct etux_fstree_entry * __restrict entry,
	const struct etux_fstree_iter * __restrict  iter __unused)
{
	etux_fstree_entry_assert_api(entry, iter);

	return etux_fstree_path_isdot(entry->dirent.d_name, entry->nlen);
}

const struct stat *
etux_fstree_entry_stat(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
{
	etux_fstree_entry_assert_api(entry, iter);

	if (!(entry->flags & ETUX_FSTREE_STAT_FLAG)) {
		int fd;
		int flags = 0;
		int err;

		fd = dirfd(iter->dir);
		etux_fstree_assert_intern(fd >= 0);

		if (!(iter->opts & ETUX_FSTREE_FOLLOW_OPT))
			flags |= AT_SYMLINK_NOFOLLOW;

		err = ufd_fstat_at(fd,
		                   entry->dirent.d_name,
		                   &entry->stat,
		                   flags);
		if (err) {
			errno = -err;
			return NULL;
		}

		entry->flags |= ETUX_FSTREE_STAT_FLAG;
	}

	return &entry->stat;
}

static __utils_nonull(1, 2) __utils_nothrow __warn_result
int
etux_fstree_entry_probe(struct etux_fstree_entry * __restrict      entry,
                        const struct etux_fstree_iter * __restrict iter)
{
	etux_fstree_entry_assert_intern(entry, iter);

	const struct stat * st;

	st = etux_fstree_entry_stat(entry, iter);
	if (!st)
		return -errno;

	if (S_ISREG(st->st_mode))
		entry->dirent.d_type = DT_REG;
	else if (S_ISDIR(st->st_mode))
		entry->dirent.d_type = DT_DIR;
	else if (S_ISLNK(st->st_mode))
		entry->dirent.d_type = DT_LNK;
	else if (S_ISFIFO(st->st_mode))
		entry->dirent.d_type = DT_FIFO;
	else if (S_ISSOCK(st->st_mode))
		entry->dirent.d_type = DT_SOCK;
	else if (S_ISCHR(st->st_mode))
		entry->dirent.d_type = DT_CHR;
	else if (S_ISBLK(st->st_mode))
		entry->dirent.d_type = DT_BLK;
	else
		/*
		 * For some reason, this may happen in strange circumstances.
		 * Especially when performing ufd_fstat_at() over the content of
		 * `/proc/self/task/<tid>/fd' directory with
		 * AT_SYMLINK_NOFOLLOW flag disabled... Maybe some kind of
		 * system race conditions ???
		 */
		return -ENOTSUP;

	return 0;
}

int
etux_fstree_entry_type(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
{
	etux_fstree_entry_assert_api(entry, iter);

	if (entry->dirent.d_type == DT_UNKNOWN) {
		int err;

		err = etux_fstree_entry_probe(entry, iter);
		if (err)
			return err;

		return entry->dirent.d_type;
	}

	switch (entry->dirent.d_type) {
	case DT_REG:
	case DT_DIR:
	case DT_LNK:
	case DT_FIFO:
	case DT_SOCK:
	case DT_CHR:
	case DT_BLK:
	case DT_WHT:
		break;

	case DT_UNKNOWN:
	default:
		etux_fstree_assert_intern(0);
	}

	return entry->dirent.d_type;
}

const char *
etux_fstree_entry_name(
	const struct etux_fstree_entry * __restrict entry,
	const struct etux_fstree_iter * __restrict  iter __unused)
{
	etux_fstree_entry_assert_api(entry, iter);

	return entry->dirent.d_name;
}

static __utils_nonull(1, 3) __utils_nothrow
size_t
etux_fstree_join_path(char         path[__restrict_arr PATH_MAX],
                      size_t       pathlen,
                      const char * basename,
                      size_t       baselen)
{
	etux_fstree_assert_intern(path);
	etux_fstree_assert_intern(basename);
	etux_fstree_assert_intern(baselen);
	etux_fstree_assert_intern(baselen <= NAME_MAX);
	etux_fstree_assert_intern(strnlen(basename, NAME_MAX + 1) == baselen);

	if (pathlen) {
		if (path[pathlen - 1] != '/')
			path[pathlen++] = '/';
	}

	memcpy(&path[pathlen], basename, baselen);
	path[pathlen + baselen] = '\0';

	return pathlen + baselen;
}

const char *
etux_fstree_entry_path(struct etux_fstree_entry * __restrict      entry,
                       const struct etux_fstree_iter * __restrict iter)
{
	etux_fstree_entry_assert_api(entry, iter);

	if (!(entry->flags & ETUX_FSTREE_PATH_FLAG)) {
		size_t plen = iter->plen;

		if (!entry->path) {
			entry->path = malloc(PATH_MAX);
			if (!entry->path)
				return NULL;
		}

		if (plen) {
			memcpy(entry->path, iter->path, plen);
			if (iter->path[plen - 1] != '/')
				entry->path[plen++] = '/';
		}

		memcpy(&entry->path[plen],
		       entry->dirent.d_name,
		       entry->nlen);
		entry->path[plen + entry->nlen] = '\0';

		entry->flags |= ETUX_FSTREE_PATH_FLAG;
	}

	return entry->path;
}

const char *
etux_fstree_entry_slink(struct etux_fstree_entry * __restrict      entry,
                        const struct etux_fstree_iter * __restrict iter)
{
	etux_fstree_entry_assert_api(entry, iter);

	if (!(entry->flags & ETUX_FSTREE_SLINK_FLAG)) {
		int     fd;
		ssize_t ret;

		if (!entry->slink) {
			entry->slink = malloc(PATH_MAX);
			if (!entry->slink)
				return NULL;
		}

		fd = dirfd(iter->dir);
		etux_fstree_assert_intern(fd >= 0);

		ret = readlinkat(fd,
		                 entry->dirent.d_name,
		                 entry->slink,
		                 PATH_MAX);
		etux_fstree_assert_intern(ret <= PATH_MAX);
		if (ret < 0) {
			etux_fstree_assert_intern(ret != EFAULT);
			etux_fstree_assert_intern(ret != ENAMETOOLONG);
			etux_fstree_assert_intern(ret != EBADF);

			return NULL;
		}
		else if (!ret) {
			errno = -ENODATA;
			return NULL;
		}
		else if (ret == PATH_MAX) {
			errno = -ENAMETOOLONG;
			return NULL;
		}

		entry->flags |= ETUX_FSTREE_SLINK_FLAG;
		entry->slink[ret] = '\0';
	}

	return entry->slink;
}

static __utils_nonull(1) __utils_nothrow
void
etux_fstree_entry_init(struct etux_fstree_entry * __restrict entry)
{
	etux_fstree_assert_intern(entry);

	entry->flags = 0;
	entry->path = NULL;
	entry->slink = NULL;
}

static __utils_nonull(1) __utils_nothrow
void
etux_fstree_entry_fini(struct etux_fstree_entry * __restrict entry)
{
	etux_fstree_assert_intern(entry);

	free(entry->path);
	free(entry->slink);
}

static __utils_nonull(1) __utils_nothrow
void
etux_fstree_entry_free(struct stroll_falloc * __restrict     alloc,
                       struct etux_fstree_entry * __restrict entry)
{
	etux_fstree_assert_intern(alloc);

	stroll_falloc_free(alloc, entry);
}

static __utils_nonull(1)
       __malloc(etux_fstree_entry_free, 2)
       __utils_nothrow
       __warn_result
struct etux_fstree_entry *
etux_fstree_entry_create(struct stroll_falloc * __restrict alloc)
{
	etux_fstree_assert_intern(alloc);

	struct etux_fstree_entry * ent;

	ent = stroll_falloc_alloc(alloc);
	if (!ent)
		return NULL;

	etux_fstree_entry_init(ent);

	return ent;
}

static __utils_nonull(1, 2) __utils_nothrow
void
etux_fstree_entry_destroy(struct stroll_falloc * __restrict     alloc,
                          struct etux_fstree_entry * __restrict entry)
{
	etux_fstree_assert_intern(alloc);
	etux_fstree_assert_intern(entry);

	etux_fstree_entry_fini(entry);

	etux_fstree_entry_free(alloc, entry);
}

/******************************************************************************
 * Simple filesystem tree iterator
 ******************************************************************************/

unsigned int
etux_fstree_iter_depth(const struct etux_fstree_iter * __restrict iter)
{
	etux_fstree_iter_assert_api(iter);

	return iter->depth;
}

const char *
etux_fstree_iter_path(const struct etux_fstree_iter * __restrict iter)
{
	etux_fstree_iter_assert_api(iter);

	return iter->path;
}

/*
 * Jump to next directory entry.
 *
 * Return values:
 * -ENOENT -- end of iteration of current directory stream, i.e., no more
 *            entries.
 * < 0     -- negative errno like value indicating an unrecoverable error.
 */
static __utils_nonull(1, 2, 3) __warn_result
int
etux_fstree_iter_next(struct etux_fstree_iter * __restrict iter,
                      const struct dirent ** __restrict    dirent,
                      etux_fstree_handle_fn *              handle,
                      void *                               data)
{
	etux_fstree_iter_assert_intern(iter);
	etux_fstree_assert_intern(dirent);
	etux_fstree_assert_intern(handle);

	struct dirent * ent;

	errno = 0;
	ent = readdir(iter->dir);
	if (ent) {
		/* Current directory iteration succeeded. */
		*dirent = ent;

		return ETUX_FSTREE_CONT_CMD;
	}

	etux_fstree_assert_intern(errno != EBADF);
	if (errno) {
		/* An unrecoverable error happened. */
		int err = -errno;

		if (err != -ENOMEM)
			handle(NULL, iter, ETUX_FSTREE_NEXT_ERR_EVT, err, data);

		return err;
	}

	/* End of current directory iteration. */
	return ETUX_FSTREE_STOP_CMD;
}

/*
 * Load and validate the content of the current directory entry.
 *
 * Return values:
 * 0             -- success
 * -ENODATA      -- entry file name is empty
 * -ENAMETOOLONG -- entry file name too long
 * -EISDIR       -- an entry which file name is `.' or `..' has invalid type
 * -EACCES       -- search permission denied for one of the directories in the
 *                  path prefix of entry
 * -ELOOP        -- too many symbolic links encountered while traversing the
 *                  entry path
 * -ENOENT       -- a component of entry path does not exist or is a dangling
 *                  symbolic link
 * -ENOMEM       -- out of memory
 * -ENOTDIR      -- a component of the entry path prefix is not a directory
 * -ENOTSUP      -- unexpected type found for entry
 */
static __utils_nonull(1, 2, 3) __utils_nothrow __warn_result
int
etux_fstree_iter_load(const struct etux_fstree_iter * __restrict iter,
                      struct etux_fstree_entry * __restrict      entry,
                      const struct dirent * __restrict           dirent)
{
	etux_fstree_iter_assert_intern(iter);
	etux_fstree_assert_intern(entry);
	etux_fstree_assert_intern(dirent);

	int ret;

	entry->flags = 0;

	ret = (int)etux_fstree_validate_dirent(dirent, iter);
	if (ret > 0) {
		/*
		 * Watch out ! Size of available data does not match the size of
		 * the dirent structure definition ! Use the `d_reclen' field to
		 * prevent from out of boundaries accesses. See readdir(3) for
		 * more infos...
		 */
		memcpy(&entry->dirent, dirent, dirent->d_reclen);

		entry->nlen = (size_t)ret;

		if (iter->opts & ETUX_FSTREE_FOLLOW_OPT) {
			/*
			 * When symlinks must be followed, retrieve properties
			 * about the current directory entry that is pointed to
			 * by a symlink.
			 */
			ret = etux_fstree_entry_type(entry, iter);
			if (ret == DT_LNK)
				return etux_fstree_entry_probe(entry, iter);
		}

		return (ret >= 0) ? 0 : ret;
	}
	else
		return ret;
}

static __utils_nonull(1, 2) __warn_result
int
etux_fstree_iter_process(struct etux_fstree_iter * __restrict iter,
                         etux_fstree_handle_fn *              handle,
                         void *                               data)
{
	etux_fstree_iter_assert_intern(iter);
	etux_fstree_assert_intern(handle);

	struct etux_fstree_entry ent;
	int                      ret;

	etux_fstree_entry_init(&ent);

	while (true) {
		const struct dirent * dent;

		ret = etux_fstree_iter_next(iter, &dent, handle, data);
		if (ret != ETUX_FSTREE_CONT_CMD)
			break;

		/* Load and validate current directory entry content. */
		ret = etux_fstree_iter_load(iter, &ent, dent);
		if (!ret)
			/* Entry has been properly loaded. */
			ret = handle(&ent, iter, ETUX_FSTREE_ENT_EVT, 0, data);
		else if (ret != -ENOMEM)
			/* An error happened while loading entry. */
			ret = handle(&ent,
			             iter,
			             ETUX_FSTREE_LOAD_ERR_EVT,
			             ret,
			             data);
		etux_fstree_assert_api((ret < 0) ||
		                       (ret == ETUX_FSTREE_CONT_CMD) ||
		                       (ret == ETUX_FSTREE_STOP_CMD));
		if (ret != ETUX_FSTREE_CONT_CMD)
			break;
	}

	etux_fstree_entry_fini(&ent);

	return (ret >= 0) ? 0 : ret;
}

static __utils_nonull(1) __warn_result
int
etux_fstree_iter_init(struct etux_fstree_iter * __restrict iter,
                      const char * __restrict              path,
                      int                                  options)
{
	etux_fstree_assert_intern(iter);
	etux_fstree_assert_intern(!path ||
	                          !path[0] ||
	                          upath_validate_path_name(path) > 0);
	etux_fstree_assert_intern(!(options & ~ETUX_FSTREE_VALID_OPTS));

	iter->path = malloc(PATH_MAX);
	if (!iter->path)
		return -ENOMEM;

	if (path && path[0]) {
		size_t len;

		len = strnlen(path, PATH_MAX);
		etux_fstree_assert_intern(len < PATH_MAX);

		memcpy(iter->path, path, len);
		iter->plen = len;
		iter->path[len] = '\0';
	}
	else {
		iter->plen = 0;
		iter->path[0] = '\0';
	}

	iter->dir = opendir(iter->plen ? iter->path : ".");
	if (!iter->dir) {
		int err = errno;

		etux_fstree_assert_intern(err != EBADF);

		free(iter->path);

		return -err;
	}

	iter->opts = options;
	iter->depth = 1;

	return 0;
}

static __utils_nonull(1)
void
etux_fstree_iter_fini(struct etux_fstree_iter * __restrict iter)
{
	etux_fstree_iter_assert_intern(iter);

	int err __unused;

	err = closedir(iter->dir);
	etux_fstree_assert_api(!err);

	free(iter->path);
}

int
etux_fstree_iter(const char * __restrict path,
                 int                     options,
                 etux_fstree_handle_fn * handle,
                 void *                  data)
{
	etux_fstree_assert_api(!path ||
	                       !path[0] ||
	                       upath_validate_path_name(path) > 0);
	etux_fstree_assert_api(!(options & ~ETUX_FSTREE_FOLLOW_OPT));
	etux_fstree_assert_api(handle);

	struct etux_fstree_iter iter;
	int                     ret;

	ret = etux_fstree_iter_init(&iter, path, options);
	if (ret)
		return ret;

	ret = etux_fstree_iter_process(&iter, handle, data);

	etux_fstree_iter_fini(&iter);

	return ret;
}

/******************************************************************************
 * Vector of filesystem tree entries with sorting ability.
 ******************************************************************************/

struct etux_fstree_contxt {
	etux_fstree_cmp_fn *            cmp;
	const struct etux_fstree_iter * iter;
	void *                          data;
};

#define etux_fstree_contxt_assert(_ctx) \
	etux_fstree_assert_intern(_ctx); \
	etux_fstree_assert_intern((_ctx)->cmp); \
	etux_fstree_assert_intern((_ctx)->iter)

struct etux_fstree_vect {
	unsigned int                cnt;
	unsigned int                nr;
	struct etux_fstree_entry ** ents;
};

#define ETUX_FSTREE_VECT_MIN_NR (8U)

#define etux_fstree_vect_assert(_vect) \
	etux_fstree_assert_intern(_vect); \
	etux_fstree_assert_intern((_vect)->nr >= ETUX_FSTREE_VECT_MIN_NR); \
	etux_fstree_assert_intern((_vect)->cnt <= (_vect)->nr); \
	etux_fstree_assert_intern((_vect)->ents)

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
unsigned int
etux_fstree_vect_count(const struct etux_fstree_vect * __restrict vector)
{
	etux_fstree_vect_assert(vector);

	return vector->cnt;
}

static __utils_nonull(1)
       __utils_pure
       __utils_nothrow
       __returns_nonnull
       __warn_result
struct etux_fstree_entry *
etux_fstree_vect_get(const struct etux_fstree_vect * __restrict vector,
                     unsigned int                               index)
{
	etux_fstree_vect_assert(vector);
	etux_fstree_assert_intern(index < vector->cnt);

	return vector->ents[index];
}

static __utils_nonull(1) __utils_nothrow __warn_result
int
etux_fstree_vect_grow(struct etux_fstree_vect * __restrict vector)
{
	etux_fstree_vect_assert(vector);

	struct etux_fstree_entry ** ents = vector->ents;
	unsigned int                nr = 2 * vector->nr;

	ents = realloc(ents, nr * sizeof(*ents));
	if (!ents)
		return -ENOMEM;

	vector->ents = ents;
	vector->nr = nr;

	return 0;
}

static __utils_nonull(1) __utils_nothrow __warn_result
int
etux_fstree_vect_add(struct etux_fstree_vect * __restrict  vector,
                     struct etux_fstree_entry * __restrict entry)
{
	etux_fstree_vect_assert(vector);

	if (vector->cnt == vector->nr) {
		int err;

		err = etux_fstree_vect_grow(vector);
		if (err)
			return -ENOMEM;
	}

	vector->ents[vector->cnt++] = entry;

	return 0;
}

static __stroll_nonull(1, 2, 3) __warn_result
int
etux_fstree_vect_cmp(const void * __restrict first,
                     const void * __restrict second,
                     void *                  data)
{
	etux_fstree_assert_intern(first);
	etux_fstree_assert_intern(second);
	etux_fstree_assert_intern(data);

	struct etux_fstree_entry *           fst =
		*(struct etux_fstree_entry * const *)first;
	struct etux_fstree_entry *           snd =
		*(struct etux_fstree_entry * const *)second;
	const struct etux_fstree_contxt *    ctx =
		(const struct etux_fstree_contxt *)data;

	etux_fstree_contxt_assert(ctx);
	etux_fstree_entry_assert_intern(fst, ctx->iter);
	etux_fstree_entry_assert_intern(snd, ctx->iter);

	return ctx->cmp(fst, snd, ctx->iter, ctx->data);
}

static __utils_nonull(1, 2) __warn_result
int
etux_fstree_vect_sort(
	struct etux_fstree_vect * __restrict         vector,
	const struct etux_fstree_contxt * __restrict contxt)
{
	etux_fstree_vect_assert(vector);
	etux_fstree_contxt_assert(contxt);

STROLL_IGNORE_WARN("-Wcast-qual")
	return stroll_array_merge_sort(vector->ents,
	                               vector->cnt,
	                               sizeof(vector->ents[0]),
	                               etux_fstree_vect_cmp,
	                               (void *)contxt);
STROLL_RESTORE_WARN
}

static __utils_nonull(1) __utils_nothrow __warn_result
int
etux_fstree_vect_init(struct etux_fstree_vect * __restrict vector)
{
	etux_fstree_assert_intern(vector);

	struct etux_fstree_entry ** ents;

	ents = malloc(ETUX_FSTREE_VECT_MIN_NR * sizeof(*ents));
	if (!ents)
		return -ENOMEM;

	vector->cnt = 0;
	vector->nr = ETUX_FSTREE_VECT_MIN_NR;
	vector->ents = ents;

	return 0;
}

static __utils_nonull(1) __utils_nothrow
void
etux_fstree_vect_fini(struct etux_fstree_vect * __restrict vector)
{
	etux_fstree_vect_assert(vector);

	unsigned int e;

	for (e = 0; e < etux_fstree_vect_count(vector); e++)
		etux_fstree_entry_fini(etux_fstree_vect_get(vector, e));

	free(vector->ents);
}

static __utils_nonull(1) __utils_nothrow
void
etux_fstree_vect_free(struct stroll_falloc * __restrict    alloc,
                      struct etux_fstree_vect * __restrict vector)
{
	etux_fstree_assert_intern(alloc);

	stroll_falloc_free(alloc, vector);
}

static __utils_nonull(1)
       __malloc(etux_fstree_vect_free, 2)
       __utils_nothrow
       __warn_result
struct etux_fstree_vect *
etux_fstree_vect_create(struct stroll_falloc * __restrict alloc)
{
	etux_fstree_assert_intern(alloc);

	struct etux_fstree_vect * vect;
	int                       err;

	vect = stroll_falloc_alloc(alloc);
	if (!vect)
		return NULL;

	err = etux_fstree_vect_init(vect);
	if (!err)
		return vect;

	stroll_falloc_free(alloc, vect);

	errno = -err;

	return NULL;
}

static __utils_nonull(1, 2) __utils_nothrow
void
etux_fstree_vect_destroy(struct stroll_falloc * __restrict    alloc,
                         struct etux_fstree_vect * __restrict vector)
{
	etux_fstree_assert_intern(alloc);
	etux_fstree_assert_intern(vector);

	etux_fstree_vect_fini(vector);

	etux_fstree_vect_free(alloc, vector);
}

/******************************************************************************
 * Simple filesystem tree iterator with sorting ability.
 ******************************************************************************/

struct etux_fstree_sort {
	struct etux_fstree_iter iter;
	struct stroll_falloc    alloc;
};

#define etux_fstree_sort_assert(_iter) \
	etux_fstree_assert_intern(_iter); \
	etux_fstree_iter_assert_intern(&(_iter)->iter)

static __utils_nonull(1)
       __malloc(etux_fstree_entry_free, 2)
       __utils_nothrow
       __warn_result
struct etux_fstree_entry *
etux_fstree_sort_create_entry(struct etux_fstree_sort * __restrict sort)
{
	etux_fstree_sort_assert(sort);

	return etux_fstree_entry_create(&sort->alloc);
}

static __utils_nonull(1) __utils_nothrow
void
etux_fstree_sort_free_entry(struct etux_fstree_sort * __restrict  sort,
                            struct etux_fstree_entry * __restrict entry)
{
	etux_fstree_sort_assert(sort);

	return etux_fstree_entry_free(&sort->alloc, entry);
}

static __utils_nonull(1, 2) __utils_nothrow
void
etux_fstree_sort_destroy_entry(struct etux_fstree_sort * __restrict  sort,
                               struct etux_fstree_entry * __restrict entry)
{
	etux_fstree_sort_assert(sort);
	etux_fstree_assert_intern(entry);

	return etux_fstree_entry_destroy(&sort->alloc, entry);
}

static __utils_nonull(1, 2, 3, 5) __warn_result
int
etux_fstree_sort_load_entry(
	struct etux_fstree_sort * __restrict sort,
	struct etux_fstree_vect * __restrict vect,
	const struct dirent * __restrict     dirent,
	etux_fstree_filter_fn *              filter,
	etux_fstree_handle_fn *              handle,
	void *                               data)
{
	etux_fstree_sort_assert(sort);
	etux_fstree_vect_assert(vect);
	etux_fstree_assert_intern(dirent);
	etux_fstree_assert_intern(handle);

	struct etux_fstree_entry * ent;
	int                        ret;

	ent = etux_fstree_sort_create_entry(sort);
	if (!ent)
		return -ENOMEM;

	/* Load and validate current directory entry content. */
	ret = etux_fstree_iter_load(&sort->iter, ent, dirent);
	if (!ret) {
		/* Entry has been properly loaded. */
		if (filter)
			ret = filter(ent, &sort->iter, data);

		if (ret == ETUX_FSTREE_CONT_CMD) {
			ret = etux_fstree_vect_add(vect, ent);
			etux_fstree_assert_intern(!ret || (ret == -ENOMEM));
			if (!ret)
				return ETUX_FSTREE_CONT_CMD;
		}
	}
	else if (ret != -ENOMEM)
		/* An error happened while loading entry. */
		ret = handle(ent,
		             &sort->iter,
		             ETUX_FSTREE_LOAD_ERR_EVT,
		             ret,
		             data);

	etux_fstree_assert_api((ret < 0) ||
	                       (ret == ETUX_FSTREE_CONT_CMD) ||
	                       (ret == ETUX_FSTREE_STOP_CMD) ||
	                       (ret == ETUX_FSTREE_SKIP_CMD));

	etux_fstree_sort_destroy_entry(sort, ent);

	return (ret != ETUX_FSTREE_SKIP_CMD) ? ret : ETUX_FSTREE_CONT_CMD;
}

static __utils_nonull(1, 2, 4, 5) __warn_result
int
etux_fstree_sort_load(struct etux_fstree_sort * __restrict         sort,
                      struct etux_fstree_vect * __restrict         vect,
                      etux_fstree_filter_fn *                      filter,
                      etux_fstree_handle_fn *                      handle,
                      const struct etux_fstree_contxt * __restrict contxt)
{
	etux_fstree_sort_assert(sort);
	etux_fstree_vect_assert(vect);
	etux_fstree_assert_intern(handle);
	etux_fstree_contxt_assert(contxt);

	int ret;

	while (true) {
		const struct dirent * dent;

		ret = etux_fstree_iter_next(&sort->iter,
		                            &dent,
		                            handle,
		                            contxt->data);
		if (ret != ETUX_FSTREE_CONT_CMD)
			break;

		ret = etux_fstree_sort_load_entry(sort,
		                                  vect,
		                                  dent,
		                                  filter,
		                                  handle,
		                                  contxt->data);
		if (ret != ETUX_FSTREE_CONT_CMD)
			break;
	}

	if (ret < 0)
		return ret;

	ret = etux_fstree_vect_sort(vect, contxt);
	if (ret)
		return ret;

	return 0;
}

static __utils_nonull(1, 2, 3) __warn_result
int
etux_fstree_sort_process(struct etux_fstree_sort * __restrict sort,
                         struct etux_fstree_vect * __restrict vector,
                         etux_fstree_handle_fn *              handle,
                         void *                               data)
{
	etux_fstree_sort_assert(sort);
	etux_fstree_vect_assert(vector);
	etux_fstree_assert_intern(handle);

	unsigned int e;
	int          ret = 0;

	for (e = 0; e < etux_fstree_vect_count(vector); e++) {
		ret = handle(etux_fstree_vect_get(vector, e),
		             &sort->iter,
		             ETUX_FSTREE_ENT_EVT,
		             0,
		             data);
		if (ret)
			break;
	}

	etux_fstree_assert_api((ret < 0) ||
	                       (ret == ETUX_FSTREE_CONT_CMD) ||
	                       (ret == ETUX_FSTREE_STOP_CMD));
	return (ret >= 0) ? 0 : ret;
}

static __utils_nonull(1) __warn_result
int
etux_fstree_sort_init(struct etux_fstree_sort * __restrict sort,
                      const char * __restrict              path,
                      int                                  options)
{
	etux_fstree_assert_intern(sort);
	etux_fstree_assert_intern(!path ||
	                          !path[0] ||
	                          upath_validate_path_name(path) > 0);
	etux_fstree_assert_intern(!(options & ~ETUX_FSTREE_VALID_OPTS));

	int err;

	err = etux_fstree_iter_init(&sort->iter, path, options);
	if (err)
		return err;

#define ETUX_FSTREE_ENTRY_ALLOC_NR (16U)
	stroll_falloc_init(&sort->alloc,
	                   ETUX_FSTREE_ENTRY_ALLOC_NR,
	                   sizeof(struct etux_fstree_entry));

	return 0;
}

static __utils_nonull(1)
void
etux_fstree_sort_fini(struct etux_fstree_sort * __restrict sort)
{
	etux_fstree_sort_assert(sort);

	stroll_falloc_fini(&sort->alloc);
	etux_fstree_iter_fini(&sort->iter);
}

int
etux_fstree_sort_iter(const char * __restrict path,
                      int                     options,
                      etux_fstree_filter_fn * filter,
                      etux_fstree_cmp_fn *    compare,
                      etux_fstree_handle_fn * handle,
                      void *                  data)
{
	etux_fstree_assert_api(!path ||
	                       !path[0] ||
	                       upath_validate_path_name(path) > 0);
	etux_fstree_assert_api(!(options & ~ETUX_FSTREE_FOLLOW_OPT));
	etux_fstree_assert_api(compare);
	etux_fstree_assert_api(handle);

	struct etux_fstree_sort         sort;
	struct etux_fstree_vect         vect;
	int                             ret;
	const struct etux_fstree_contxt ctx = {
		.cmp  = compare,
		.iter = &sort.iter,
		.data = data
	};

	ret = etux_fstree_sort_init(&sort, path, options);
	if (ret)
		return ret;

	ret = etux_fstree_vect_init(&vect);
	if (ret)
		goto fini_iter;

	ret = etux_fstree_sort_load(&sort, &vect, filter, handle, &ctx);
	if (ret)
		goto fini_vect;

	ret = etux_fstree_sort_process(&sort, &vect, handle, data);

fini_vect:
	etux_fstree_vect_fini(&vect);
fini_iter:
	etux_fstree_sort_fini(&sort);

	return ret;
}

/******************************************************************************
 * Internal recursive filesystem tree walk path tracking.
 ******************************************************************************/

struct etux_fstree_point {
	struct etux_fstree_entry * ent;
	DIR *                      dir;
	size_t                     len;
	struct etux_fstree_vect *  vect;
	unsigned int               idx;
};

struct etux_fstree_track {
	unsigned int               cnt;
	unsigned int               nr;
	dev_t                      dev;
	ino_t                      ino;
	struct etux_fstree_point * pts;
};

#define ETUX_FSTREE_TRACK_MIN_NR (8U)

#define etux_fstree_assert_track(_trk) \
	etux_fstree_assert_intern(_trk); \
	etux_fstree_assert_intern((_trk)->nr >= ETUX_FSTREE_TRACK_MIN_NR); \
	etux_fstree_assert_intern((_trk)->cnt <= (_trk)->nr); \
	etux_fstree_assert_intern((_trk)->pts)

#define etux_fstree_track_foreach(_trk, _pt) \
	for ((_pt) = (_trk)->pts; \
	     (_pt) < &(_trk)->pts[etux_fstree_track_count(_trk)]; \
	     (_pt)++)

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
unsigned int
etux_fstree_track_count(const struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_track(track);

	return track->cnt;
}

static __utils_nonull(1) __utils_nothrow __warn_result
int
etux_fstree_track_grow(struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_track(track);

	struct etux_fstree_point * pts = track->pts;
	unsigned int               nr = 2 * track->nr;

	pts = realloc(pts, nr * sizeof(*pts));
	if (!pts)
		return -ENOMEM;

	track->pts = pts;
	track->nr = nr;

	return 0;
}

static __utils_nonull(1) __utils_nothrow __warn_result
struct etux_fstree_point *
etux_fstree_track_push(struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_track(track);

	if (track->cnt == track->nr) {
		int err;

		err = etux_fstree_track_grow(track);
		if (err) {
			errno = ENOMEM;
			return NULL;
		}
	}

	return &track->pts[track->cnt++];
}

static __utils_nonull(1) __utils_nothrow __returns_nonull __warn_result
const struct etux_fstree_point *
etux_fstree_track_pop(struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_track(track);
	etux_fstree_assert_intern(track->cnt);

	return &track->pts[--track->cnt];
}

static __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
etux_fstree_track_loop(const struct etux_fstree_track * __restrict track,
                       dev_t                                       dev,
                       ino_t                                       ino)
{
	etux_fstree_assert_track(track);

	const struct  etux_fstree_point * pt;

	if ((dev == track->dev) && (ino == track->ino))
		return true;

	etux_fstree_track_foreach(track, pt) {
		etux_fstree_assert_intern(pt->ent->flags &
		                          ETUX_FSTREE_STAT_FLAG);

		if ((dev == pt->ent->stat.st_dev) &&
		    (ino == pt->ent->stat.st_ino))
			return true;
	}

	return false;
}

static __utils_nonull(1) __utils_nothrow __warn_result
int
etux_fstree_track_init(struct etux_fstree_track * __restrict track,
                       dev_t                                 dev,
                       ino_t                                 ino)
{
	etux_fstree_assert_intern(track);

	struct etux_fstree_point * pts;

	pts = malloc(ETUX_FSTREE_TRACK_MIN_NR * sizeof(*pts));
	if (!pts)
		return -ENOMEM;

	track->cnt = 0;
	track->nr = ETUX_FSTREE_TRACK_MIN_NR;
	track->dev = dev;
	track->ino = ino;
	track->pts = pts;

	return 0;
}

static __utils_nonull(1) __utils_nothrow
void
etux_fstree_track_fini(struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_track(track);

	free(track->pts);
}

/******************************************************************************
 * Recursive filesystem tree scanner.
 ******************************************************************************/

struct etux_fstree_scan {
	struct etux_fstree_sort  sort;
	struct etux_fstree_track track;
};

#define etux_fstree_assert_scan(_scan) \
	etux_fstree_sort_assert(&(_scan)->sort); \
	etux_fstree_assert_track(&(_scan)->track)

#define etux_fstree_assert_scan_entry(_ent, _scan) \
	etux_fstree_entry_assert_intern(_ent, &(_scan)->sort.iter); \
	etux_fstree_assert_track(&(_scan)->track)

static inline __utils_nonull(1)
              __utils_pure
              __nothrow
              __returns_nonull
              __warn_result
struct etux_fstree_iter *
etux_fstree_scan_iter(const struct etux_fstree_scan * __restrict scan)
{
STROLL_IGNORE_WARN("-Wcast-qual")
	return (struct etux_fstree_iter *)&scan->sort.iter;
STROLL_RESTORE_WARN
}

static __utils_nonull(2) __warn_result
DIR *
etux_fstree_open_dir_at(int fd, const char * __restrict path, int flags)
{
	etux_fstree_assert_intern(fd >= 0);
	etux_fstree_assert_intern(upath_validate_path_name(path) > 0);
	etux_fstree_assert_intern(!(flags & ~O_NOFOLLOW));

	int   cfd; /* file descriptor of child directory */
	DIR * dir;

	cfd = udir_open_at(fd, path, flags | O_NONBLOCK | O_CLOEXEC);
	if (cfd < 0) {
		etux_fstree_assert_intern(cfd != -EINVAL);
		errno = -cfd;

		return NULL;
	}

	dir = fdopendir(cfd);
	if (!dir) {
		int err = errno;

		etux_fstree_assert_intern(err != EMFILE);
		etux_fstree_assert_intern(err != ENFILE);
		etux_fstree_assert_intern(err != ENOENT);
		etux_fstree_assert_intern(err != EINVAL);

		ufd_close(cfd);
		errno = err;

		return NULL;
	}

	return dir;
}

static __utils_nonull(1, 2) __utils_nothrow __warn_result
int
etux_fstree_scan_isxdev_entry(struct etux_fstree_entry * __restrict      entry,
                              const struct etux_fstree_scan * __restrict scan)
{
	etux_fstree_assert_scan_entry(entry, scan);

	const struct stat * st;

	st = etux_fstree_entry_stat(entry, &scan->sort.iter);
	if (!st)
		return -errno;

	if (st->st_dev != scan->track.dev)
		return 1;

	return 0;
}

static __utils_nonull(1, 2) __utils_nothrow __warn_result
int
etux_fstree_scan_isloop_entry(struct etux_fstree_entry * __restrict      entry,
                              const struct etux_fstree_scan * __restrict scan)
{
	etux_fstree_assert_scan_entry(entry, scan);

	const struct etux_fstree_iter * iter = etux_fstree_scan_iter(scan);

	if (iter->opts & ETUX_FSTREE_FOLLOW_OPT) {
		const struct stat * st;

		st = etux_fstree_entry_stat(entry, iter);
		if (!st)
			return -errno;

		return (int)etux_fstree_track_loop(&scan->track,
		                                   st->st_dev,
		                                   st->st_ino);
	}

	return 0;
}

static __utils_nonull(1, 2) __utils_nothrow __warn_result
int
etux_fstree_scan_may_enter(const struct etux_fstree_scan * __restrict scan,
                           struct etux_fstree_entry * __restrict      entry)
{
	etux_fstree_assert_scan_entry(entry, scan);

	const struct etux_fstree_iter * iter = etux_fstree_scan_iter(scan);
	int                             ret;

	ret = etux_fstree_entry_type(entry, iter);
	if (ret < 0)
		return ret;
	else if (ret != DT_DIR)
		return 0;

	if (etux_fstree_entry_isdot(entry, iter))
		return 0;

	if (!(iter->opts & ETUX_FSTREE_XDEV_OPT)) {
		ret = etux_fstree_scan_isxdev_entry(entry, scan);
		if (ret < 0)
			return ret;
		else if (ret)
			return 0;
	}

	return 1;
}

#define ETUX_FSTREE_ENTER_CMD (3)

static __utils_nonull(1, 2, 3) __warn_result
int
etux_fstree_scan_recurs(struct etux_fstree_scan * __restrict  scan,
                        struct etux_fstree_entry * __restrict entry,
                        etux_fstree_handle_fn *               handle,
                        void *                                data)
{
	etux_fstree_assert_scan(scan);
	etux_fstree_assert_intern(entry);
	etux_fstree_assert_intern(handle);

	const struct etux_fstree_iter * iter = etux_fstree_scan_iter(scan);
	int                             ret;

	ret = etux_fstree_scan_isloop_entry(entry, scan);
	etux_fstree_assert_intern(ret <= 1);
	if (!ret) {
		if (iter->opts & ETUX_FSTREE_PRE_OPT) {
			/*
			 * Make sure that the handler really wants to recurse
			 * into this subdirectory.
			 */
			ret = handle(entry, iter, ETUX_FSTREE_PRE_EVT, 0, data);
			switch (ret) {
			case ETUX_FSTREE_CONT_CMD:
				break;
			case ETUX_FSTREE_STOP_CMD:
				return ETUX_FSTREE_STOP_CMD;
			case ETUX_FSTREE_SKIP_CMD:
				return ETUX_FSTREE_CONT_CMD;
			default:
				etux_fstree_assert_api(ret < 0);
				return ret;
			}
		}

		return ETUX_FSTREE_ENTER_CMD;
	}
	else if (ret == 1)
			/* Symlink to directory loop detected: don't recurse. */
			ret = handle(entry,
			             iter,
			             ETUX_FSTREE_LOOP_EVT,
			             0,
			             data);
	else if (ret != -ENOMEM)
			/* Failure while retrieving entry attributes. */
			ret = handle(entry,
			             iter,
			             ETUX_FSTREE_LOAD_ERR_EVT,
			             ret,
			             data);

	etux_fstree_assert_api((ret < 0) ||
	                       (ret == ETUX_FSTREE_CONT_CMD) ||
	                       (ret == ETUX_FSTREE_STOP_CMD));
	return ret;
}

static __utils_nonull(1, 2) __warn_result
struct etux_fstree_point *
etux_fstree_scan_enter_dir(struct etux_fstree_scan * __restrict  scan,
                           struct etux_fstree_entry * __restrict entry)
{
	etux_fstree_assert_scan_entry(entry, scan);

	struct etux_fstree_iter *  iter = etux_fstree_scan_iter(scan);
	int                        fd;
	DIR *                      dir;
	struct etux_fstree_point * pt;

	fd = dirfd(iter->dir);
	etux_fstree_assert_intern(fd >= 0);

	dir = etux_fstree_open_dir_at(
		fd,
		entry->dirent.d_name,
		!(iter->opts & ETUX_FSTREE_FOLLOW_OPT) ? O_NOFOLLOW : 0);
	if (!dir)
		return NULL;

	pt = etux_fstree_track_push(&scan->track);
	if (!pt) {
		closedir(dir);
		return NULL;
	}

	pt->ent = entry;
	pt->dir = iter->dir;
	pt->len = iter->plen;

	iter->dir = dir;
	iter->plen = etux_fstree_join_path(iter->path,
	                                   iter->plen,
	                                   entry->dirent.d_name,
	                                   entry->nlen);
	etux_fstree_assert_intern(iter->plen > pt->len);
	iter->depth++;

	return pt;
}

static __utils_nonull(1, 2, 3) __warn_result
int
etux_fstree_scan_enter(struct etux_fstree_scan * __restrict   scan,
                       struct etux_fstree_entry ** __restrict entry,
                       etux_fstree_handle_fn *                handle,
                       void *                                 data)
{
	etux_fstree_assert_intern(entry);
	etux_fstree_assert_scan_entry(*entry, scan);
	etux_fstree_assert_intern(handle);

	struct etux_fstree_entry * old = *entry;
	int                        ret;

	ret = etux_fstree_scan_recurs(scan, old, handle, data);
	if (ret == ETUX_FSTREE_ENTER_CMD) {
		struct etux_fstree_entry * nevv;
		struct etux_fstree_point * pt;

		nevv = etux_fstree_sort_create_entry(&scan->sort);
		if (!nevv)
			return -ENOMEM;

		pt = etux_fstree_scan_enter_dir(scan, old);
		if (pt) {
			/* Entering the child directory succeeded. */
			*entry = nevv;
			return ETUX_FSTREE_CONT_CMD;
		}
		else
			ret = -errno;

		etux_fstree_sort_free_entry(&scan->sort, nevv);

		if (ret != -ENOMEM)
			/* Entering the child directory failed. */
			ret = handle(old,
			             etux_fstree_scan_iter(scan),
			             ETUX_FSTREE_DIR_ERR_EVT,
			             ret,
			             data);
	}

	return ret;
}

static __utils_nonull(1) __warn_result
const struct etux_fstree_point *
etux_fstree_scan_exit_dir(struct etux_fstree_scan * __restrict scan)
{
	etux_fstree_assert_scan(scan);

	if (etux_fstree_track_count(&scan->track)) {
		struct etux_fstree_iter *        iter =
			etux_fstree_scan_iter(scan);
		const struct etux_fstree_point * pt;

		/* Close current directory stream. */
		closedir(iter->dir);

		pt = etux_fstree_track_pop(&scan->track);
		etux_fstree_assert_intern(pt);
		etux_fstree_assert_intern(pt->dir);
		etux_fstree_assert_intern(pt->len < PATH_MAX);

		/* Restore parent directory stream state. */
		iter->dir = pt->dir;
		iter->plen = pt->len;
		iter->path[pt->len] = '\0';
		etux_fstree_assert_intern(iter->depth);
		iter->depth--;

		return pt;
	}
	else
		return NULL;
}

static __utils_nonull(1, 2, 3) __warn_result
int
etux_fstree_scan_exit(struct etux_fstree_scan * __restrict   scan,
                      struct etux_fstree_entry ** __restrict entry,
                      etux_fstree_handle_fn *                handle,
                      void *                                 data)
{
	etux_fstree_assert_intern(entry);
	etux_fstree_assert_scan_entry(*entry, scan);
	etux_fstree_assert_intern(handle);

	const struct etux_fstree_point * pt;
	const struct etux_fstree_iter *  iter = etux_fstree_scan_iter(scan);

	pt = etux_fstree_scan_exit_dir(scan);
	if (!pt)
		return ETUX_FSTREE_STOP_CMD;

	etux_fstree_assert_scan_entry(pt->ent, scan);

	etux_fstree_sort_destroy_entry(&scan->sort, *entry);
	*entry = pt->ent;

	if (!(iter->opts & ETUX_FSTREE_POST_OPT))
		return ETUX_FSTREE_CONT_CMD;

	return handle(pt->ent, iter, ETUX_FSTREE_POST_EVT, 0, data);
}

static __utils_nonull(1, 2) __warn_result
int
etux_fstree_scan_process(struct etux_fstree_scan * __restrict scan,
                         etux_fstree_handle_fn *              handle,
                         void *                               data)
{
	etux_fstree_assert_scan(scan);
	etux_fstree_assert_intern(handle);

	int                        ret;
	struct etux_fstree_entry * ent;

	ent = etux_fstree_sort_create_entry(&scan->sort);
	if (!ent)
		return -ENOMEM;

	do {
		struct etux_fstree_iter * iter = etux_fstree_scan_iter(scan);
		const struct dirent *     dent;

		ret = etux_fstree_iter_next(iter, &dent, handle, data);
		if (ret == ETUX_FSTREE_CONT_CMD) {
			/* Load and validate current directory entry content. */
			ret = etux_fstree_iter_load(iter, ent, dent);
			if (!ret)
				ret = etux_fstree_scan_may_enter(scan, ent);

			etux_fstree_assert_intern(ret <= 1);
			if (!ret)
				/* Entry has been properly loaded. */
				ret = handle(ent,
				             iter,
				             ETUX_FSTREE_ENT_EVT,
				             0,
				             data);
			else if (ret == 1)
				/* Recursion required. */
				ret = etux_fstree_scan_enter(scan,
				                             &ent,
				                             handle,
				                             data);
			else if (ret != -ENOMEM)
				/* An error happened while loading entry. */
				ret = handle(ent,
				             iter,
				             ETUX_FSTREE_LOAD_ERR_EVT,
				             ret,
				             data);
		}
		else if (ret == ETUX_FSTREE_STOP_CMD)
			ret = etux_fstree_scan_exit(scan, &ent, handle, data);
		etux_fstree_assert_api((ret < 0) ||
		                       (ret == ETUX_FSTREE_CONT_CMD) ||
		                       (ret == ETUX_FSTREE_STOP_CMD));
	} while (ret == ETUX_FSTREE_CONT_CMD);

	etux_fstree_sort_destroy_entry(&scan->sort, ent);

	return (ret >= 0) ? 0 : ret;
}

static __utils_nonull(1) __warn_result
int
etux_fstree_scan_init(struct etux_fstree_scan * __restrict scan,
                      const char * __restrict              path,
                      int                                  options)
{
	etux_fstree_assert_intern(scan);
	etux_fstree_assert_intern(!path ||
	                          !path[0] ||
	                          upath_validate_path_name(path) > 0);
	etux_fstree_assert_intern(!(options & ~ETUX_FSTREE_VALID_OPTS));

	int         err;
	int         fd;
	struct stat st;

	err = etux_fstree_sort_init(&scan->sort, path, options);
	if (err)
		return err;

	fd = dirfd(etux_fstree_scan_iter(scan)->dir);
	etux_fstree_assert_intern(fd >= 0);

	err = ufd_fstat(fd, &st);
	if (err)
		goto fini;

	err = etux_fstree_track_init(&scan->track, st.st_dev, st.st_ino);
	if (err)
		goto fini;

	return 0;

fini:
	etux_fstree_sort_fini(&scan->sort);

	return err;
}

static __utils_nonull(1)
void
etux_fstree_scan_fini(struct etux_fstree_scan * __restrict scan)
{
	etux_fstree_assert_scan(scan);

	/*
	 * Close all directory streams left in case of premature interruption
	 * of recursive walk interruption.
	 */
	while (etux_fstree_track_count(&scan->track)) {
		struct etux_fstree_iter *        iter =
			etux_fstree_scan_iter(scan);
		const struct etux_fstree_point * pt;

		/* Close current directory stream. */
		closedir(iter->dir);

		pt = etux_fstree_track_pop(&scan->track);
		etux_fstree_assert_intern(pt);

		iter->dir = pt->dir;
	}

	etux_fstree_track_fini(&scan->track);

	etux_fstree_sort_fini(&scan->sort);
}

int
etux_fstree_scan(const char * __restrict path,
                 int                     options,
                 etux_fstree_handle_fn * handle,
                 void *                  data)
{
	etux_fstree_assert_api(!path ||
	                       !path[0] ||
	                       upath_validate_path_name(path) > 0);
	etux_fstree_assert_api(!(options & ~ETUX_FSTREE_VALID_OPTS));
	etux_fstree_assert_api(handle);

	struct etux_fstree_scan    scan;
	int                        ret;

	ret = etux_fstree_scan_init(&scan, path, options);
	if (ret)
		return ret;

	ret = etux_fstree_scan_process(&scan, handle, data);

	etux_fstree_scan_fini(&scan);

	return ret;
}

/******************************************************************************
 * Recursive filesystem tree scanner with sorting ability.
 ******************************************************************************/

struct etux_fstree_sort_scan {
	struct etux_fstree_scan   base;
	struct etux_fstree_vect * vect;
	struct stroll_falloc      valloc;
};

#define etux_fstree_assert_sort_scan(_scan) \
	etux_fstree_assert_scan(&(_scan)->base); \
	etux_fstree_vect_assert((_scan)->vect)

static __utils_nonull(1, 2, 3, 5, 6) __warn_result
int
etux_fstree_sort_scan_enter(
	struct etux_fstree_sort_scan * __restrict    scan,
	struct etux_fstree_entry * __restrict        entry,
	unsigned int * __restrict                    index,
	etux_fstree_filter_fn *                      filter,
	etux_fstree_handle_fn *                      handle,
	const struct etux_fstree_contxt * __restrict contxt)
{
	etux_fstree_assert_sort_scan(scan);
	etux_fstree_assert_scan_entry(entry, &scan->base);
	etux_fstree_assert_intern(index);
	etux_fstree_assert_intern(handle);
	etux_fstree_contxt_assert(contxt);

	int ret;

	ret = etux_fstree_scan_recurs(&scan->base, entry, handle, contxt->data);
	if (ret == ETUX_FSTREE_ENTER_CMD) {
		struct etux_fstree_vect *  nevv;
		struct etux_fstree_point * pt;

		nevv = etux_fstree_vect_create(&scan->valloc);
		if (!nevv)
			return -ENOMEM;

		pt = etux_fstree_scan_enter_dir(&scan->base, entry);
		if (pt) {
			/* Entering the child directory succeeded. */
			pt->vect = scan->vect;
			pt->idx = *index;

			scan->vect = nevv;
			*index = UINT_MAX;

			ret = etux_fstree_sort_load(&scan->base.sort,
			                            nevv,
			                            filter,
			                            handle,
			                            contxt);
		}
		else {
			ret = -errno;

			etux_fstree_vect_destroy(&scan->valloc, nevv);

			if (ret != -ENOMEM)
				/* Entering the child directory failed. */
				ret = handle(entry,
				             etux_fstree_scan_iter(&scan->base),
				             ETUX_FSTREE_DIR_ERR_EVT,
				             ret,
				             contxt->data);
		}
	}

	return ret;
}

static __utils_nonull(1, 2, 3, 4) __warn_result
int
etux_fstree_sort_scan_exit(
	struct etux_fstree_sort_scan * __restrict    scan,
	unsigned int * __restrict                    index,
	etux_fstree_handle_fn *                      handle,
	const struct etux_fstree_contxt * __restrict contxt)
{
	etux_fstree_assert_sort_scan(scan);
	etux_fstree_assert_intern(index);
	etux_fstree_assert_intern(handle);
	etux_fstree_contxt_assert(contxt);

	const struct etux_fstree_point * pt;
	struct etux_fstree_entry *       ent;
	const struct etux_fstree_iter *  iter =
		etux_fstree_scan_iter(&scan->base);

	pt = etux_fstree_scan_exit_dir(&scan->base);
	if (!pt)
		return ETUX_FSTREE_STOP_CMD;

	etux_fstree_vect_destroy(&scan->valloc, scan->vect);

	ent = etux_fstree_vect_get(pt->vect, pt->idx);
	etux_fstree_entry_assert_intern(ent, iter);

	scan->vect = pt->vect;
	*index = pt->idx + 1;

	if (!(iter->opts & ETUX_FSTREE_POST_OPT))
		return ETUX_FSTREE_CONT_CMD;

	return handle(ent, iter, ETUX_FSTREE_POST_EVT, 0, contxt->data);
}

static __utils_nonull(1, 3, 4) __warn_result
int
etux_fstree_sort_scan_process(
	struct etux_fstree_sort_scan * __restrict    scan,
	etux_fstree_filter_fn *                      filter,
	etux_fstree_handle_fn *                      handle,
	const struct etux_fstree_contxt * __restrict contxt)
{
	etux_fstree_assert_sort_scan(scan);
	etux_fstree_assert_intern(handle);
	etux_fstree_contxt_assert(contxt);

	unsigned int e = 0;
	int          ret;

	do {
		const struct etux_fstree_iter * iter =
			etux_fstree_scan_iter(&scan->base);

		while (e < etux_fstree_vect_count(scan->vect)) {
			struct etux_fstree_entry * ent;

			ent = etux_fstree_vect_get(scan->vect, e);
			etux_fstree_assert_intern(ent);

			ret = etux_fstree_scan_may_enter(&scan->base, ent);
			etux_fstree_assert_intern(ret <= 1);
			if (!ret)
				/* Entry has been properly loaded. */
				ret = handle(ent,
				             iter,
				             ETUX_FSTREE_ENT_EVT,
				             0,
				             contxt->data);
			else if (ret == 1)
				/* Recursion required. */
				ret = etux_fstree_sort_scan_enter(scan,
				                                  ent,
				                                  &e,
				                                  filter,
				                                  handle,
				                                  contxt);
			else if (ret != -ENOMEM)
				/* An error happened while loading entry. */
				ret = handle(ent,
				             iter,
				             ETUX_FSTREE_LOAD_ERR_EVT,
				             ret,
				             contxt->data);

			etux_fstree_assert_api((ret < 0) ||
			                       (ret == ETUX_FSTREE_CONT_CMD) ||
			                       (ret == ETUX_FSTREE_STOP_CMD));
			if (ret != ETUX_FSTREE_CONT_CMD)
				/*
				 * A fatal error occured or we were requested to
				 * stop.
				 */
				return ret;

			e++;
		}

		ret = etux_fstree_sort_scan_exit(scan, &e, handle, contxt);
	} while (ret == ETUX_FSTREE_CONT_CMD);

	etux_fstree_assert_api((ret < 0) ||
	                       (ret == ETUX_FSTREE_CONT_CMD) ||
	                       (ret == ETUX_FSTREE_STOP_CMD));
	return (ret >= 0) ? 0 : ret;
}

static __utils_nonull(1, 5, 6) __warn_result
int
etux_fstree_sort_scan_init(
	struct etux_fstree_sort_scan * __restrict    scan,
	const char * __restrict                      path,
	int                                          options,
	etux_fstree_filter_fn *                      filter,
	etux_fstree_handle_fn *                      handle,
	const struct etux_fstree_contxt * __restrict contxt)
{
	etux_fstree_assert_intern(scan);
	etux_fstree_assert_intern(!path ||
	                          !path[0] ||
	                          upath_validate_path_name(path) > 0);
	etux_fstree_assert_intern(!(options & ~ETUX_FSTREE_VALID_OPTS));
	etux_fstree_assert_intern(handle);
	etux_fstree_contxt_assert(contxt);

	int ret;

	ret = etux_fstree_scan_init(&scan->base, path, options);
	if (ret)
		return ret;

#define ETUX_FSTREE_VECT_ALLOC_NR (16U)
	stroll_falloc_init(&scan->valloc,
	                   ETUX_FSTREE_VECT_ALLOC_NR,
	                   sizeof(struct etux_fstree_vect));
	scan->vect = etux_fstree_vect_create(&scan->valloc);
	if (!scan->vect) {
		ret = -ENOMEM;
		goto fini;
	}

	ret = etux_fstree_sort_load(&scan->base.sort,
	                            scan->vect,
	                            filter,
	                            handle,
	                            contxt);
	etux_fstree_assert_intern(ret <= 0);
	if (ret)
		goto destroy;

	return 0;

destroy:
	etux_fstree_vect_destroy(&scan->valloc, scan->vect);
fini:
	stroll_falloc_fini(&scan->valloc);
	etux_fstree_scan_fini(&scan->base);

	return ret;
}

static __utils_nonull(1)
void
etux_fstree_sort_scan_fini(struct etux_fstree_sort_scan * __restrict scan)
{
	etux_fstree_assert_sort_scan(scan);

	etux_fstree_vect_destroy(&scan->valloc, scan->vect);
	stroll_falloc_fini(&scan->valloc);
	etux_fstree_scan_fini(&scan->base);
}

int
etux_fstree_sort_scan(const char * __restrict path,
                      int                     options,
                      etux_fstree_filter_fn * filter,
                      etux_fstree_cmp_fn *    compare,
                      etux_fstree_handle_fn * handle,
                      void *                  data)
{
	etux_fstree_assert_api(!path ||
	                       !path[0] ||
	                       upath_validate_path_name(path) > 0);
	etux_fstree_assert_api(!(options & ~ETUX_FSTREE_VALID_OPTS));
	etux_fstree_assert_api(compare);
	etux_fstree_assert_api(handle);

	struct etux_fstree_sort_scan    scan;
	int                             ret;
	const struct etux_fstree_contxt ctx = {
		.cmp  = compare,
		.iter = etux_fstree_scan_iter(&scan.base),
		.data = data
	};

	ret = etux_fstree_sort_scan_init(&scan,
	                                 path,
	                                 options,
	                                 filter,
	                                 handle,
	                                 &ctx);
	etux_fstree_assert_intern(ret <= 0);
	if (ret)
		return ret;

	ret = etux_fstree_sort_scan_process(&scan,
	                                    filter,
	                                    handle,
	                                    &ctx);
	etux_fstree_assert_api((ret < 0) ||
	                       (ret == ETUX_FSTREE_CONT_CMD) ||
	                       (ret == ETUX_FSTREE_STOP_CMD));

	etux_fstree_sort_scan_fini(&scan);

	return ret;
}
