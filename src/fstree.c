/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "utils/fstree.h"
#include <dirent.h>

#if !defined(_DIRENT_HAVE_D_TYPE)
#error dirent structure has no support for d_type field. Check your C library !
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

struct etux_fstree_iterator {
	int    options;
	DIR *  dir;
	char * path;
	size_t pathlen;
};

#define etux_fstree_assert_iter_api(_iter) \
	etux_fstree_assert_api(_iter); \
	etux_fstree_assert_api(!((_iter)->options & ~ETUX_FSTREE_VALID_OPTS)); \
	etux_fstree_assert_api((_iter)->dir); \
	etux_fstree_assert_api((_iter)->path)

#define etux_fstree_assert_iter_intern(_iter) \
	etux_fstree_assert_intern(_iter); \
	etux_fstree_assert_intern(!((_iter)->options & \
	                            ~ETUX_FSTREE_VALID_OPTS)); \
	etux_fstree_assert_intern((_iter)->dir); \
	etux_fstree_assert_intern((_iter)->path)

struct etux_fstree_entry {
	struct dirent dirent;
	size_t      namelen;
	int         flags;
	struct stat stat;
	char *      path;
	char *      slink;
};

#define etux_fstree_assert_entry_api(_ent, _iter) \
	etux_fstree_assert_api(_ent); \
	etux_fstree_assert_iter_api(_iter); \
	etux_fstree_assert_api( \
		etux_fstree_validate_dirent(&(_ent)->dirent, _iter) == \
		(ssize_t)(_ent)->namelen); \
	etux_fstree_assert_api(!((_ent)->flags & ~ETUX_FSTREE_VALID_FLAGS)); \
	etux_fstree_assert_api(!((_ent)->flags & ETUX_FSTREE_PATH_FLAG) || \
	                       (upath_validate_path_name((_ent)->path) > 0)); \
	etux_fstree_assert_api(!((_ent)->flags & ETUX_FSTREE_SLINK_FLAG) || \
	                       (upath_validate_path_name((_ent)->slink) > 0))

#define etux_fstree_assert_entry_intern(_ent, _iter) \
	etux_fstree_assert_intern(_ent); \
	etux_fstree_assert_iter_intern(_iter); \
	etux_fstree_assert_intern( \
		etux_fstree_validate_dirent(&(_ent)->dirent, _iter) == \
		(ssize_t)(_ent)->namelen); \
	etux_fstree_assert_intern(!((_ent)->flags & \
	                            ~ETUX_FSTREE_VALID_FLAGS)); \
	etux_fstree_assert_intern( \
		!((_ent)->flags & ETUX_FSTREE_PATH_FLAG) || \
		(upath_validate_path_name((_ent)->path) > 0)); \
	etux_fstree_assert_intern( \
		!((_ent)->flags & ETUX_FSTREE_SLINK_FLAG) || \
		(upath_validate_path_name((_ent)->slink) > 0))

static
bool
etux_fstree_path_isdot(const char * __restrict path, size_t len)
{
	etux_fstree_assert_intern(upath_validate_path_name(path) > 0);
	etux_fstree_assert_intern(len);

	if ((len <= 2) && (path[0] == '.') && (!path[1] || path[1] == '.'))
		return true;

	return false;
}

static
ssize_t
etux_fstree_validate_dirent(
	const struct dirent * __restrict               dirent,
	const struct etux_fstree_iterator * __restrict iter)
{
	etux_fstree_assert_intern(dirent);
	etux_fstree_assert_iter_intern(iter);

	size_t len;

	len = strnlen(dirent->d_name, NAME_MAX + 1);
	etux_fstree_assert_intern(len <= (NAME_MAX + 1));
	if (!len)
		return -ENODATA;
	else if ((len == (NAME_MAX + 1)) ||
		 ((iter->pathlen + 1 + len) >= PATH_MAX))
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
etux_fstree_entry_isdot(const struct etux_fstree_entry * __restrict    entry,
                        const struct etux_fstree_iterator * __restrict iter)
{
	etux_fstree_assert_entry_api(entry, iter);

	return etux_fstree_path_isdot(entry->dirent.d_name, entry->namelen);
}

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
const struct stat *
etux_fstree_entry_stat(struct etux_fstree_entry * __restrict          entry,
                       const struct etux_fstree_iterator * __restrict iter)
{
	etux_fstree_assert_entry_api(entry, iter);

	if (!(entry->flags & ETUX_FSTREE_STAT_FLAG)) {
		int fd;
		int flags = 0;
		int err;

		fd = dirfd(iter->dir);
		etux_fstree_assert_intern(fd >= 0);

		if (!(iter->options & ETUX_FSTREE_FOLLOW_OPT))
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

static
int
etux_fstree_setup_entry_type(
	struct etux_fstree_entry * __restrict          entry,
	const struct etux_fstree_iterator * __restrict iter)
{
	etux_fstree_assert_entry_intern(entry, iter);

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
int
etux_fstree_entry_type(struct etux_fstree_entry * __restrict          entry,
                       const struct etux_fstree_iterator * __restrict iter)
{
	etux_fstree_assert_entry_api(entry, iter);

	if (entry->dirent.d_type == DT_UNKNOWN) {
		int err;

		err = etux_fstree_setup_entry_type(entry, iter);
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
etux_fstree_entry_name(const struct etux_fstree_entry * __restrict    entry,
                       const struct etux_fstree_iterator * __restrict iter)
{
	etux_fstree_assert_entry_api(entry, iter);

	return entry->dirent.d_name;
}

const char *
etux_fstree_entry_path(struct etux_fstree_entry * __restrict          entry,
                       const struct etux_fstree_iterator * __restrict iter)
{
	etux_fstree_assert_entry_api(entry, iter);

	if (!(entry->flags & ETUX_FSTREE_PATH_FLAG)) {
		size_t plen = iter->pathlen;

		if (!entry->path) {
			entry->path = malloc(PATH_MAX);
			if (!entry->path)
				return NULL;
		}
#warning FIXME: handling of leading slash
		if (plen && (iter->path[plen - 1] == '/'))
			plen--;

		if (plen) {
			memcpy(entry->path, iter->path, plen);
			entry->path[plen++] = '/';
		}
		memcpy(&entry->path[plen],
		       entry->dirent.d_name,
		       entry->namelen);
		entry->path[plen + entry->namelen] = '\0';

		entry->flags |= ETUX_FSTREE_PATH_FLAG;
	}

	return entry->path;
}

const char *
etux_fstree_entry_slink(struct etux_fstree_entry * __restrict          entry,
                        const struct etux_fstree_iterator * __restrict iter)
{
	etux_fstree_assert_entry_api(entry, iter);

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

static
void
etux_fstree_init_entry(struct etux_fstree_entry * __restrict entry)
{
	etux_fstree_assert_intern(entry);

	entry->flags = 0;
	entry->path = NULL;
	entry->slink = NULL;
}

static
void
etux_fstree_fini_entry(struct etux_fstree_entry * __restrict entry)
{
	etux_fstree_assert_intern(entry);

	free(entry->path);
	free(entry->slink);
}

/*
 * When symlinks must be followed, retrieve properties about the current
 * directory entry that is pointed to by a symlink.
 */
static
int
etux_fstree_resolve_entry(const struct etux_fstree_iterator * __restrict iter,
                          struct etux_fstree_entry * __restrict          entry)

{
	etux_fstree_assert_iter_intern(iter);
	etux_fstree_assert_intern(entry);
	etux_fstree_assert_intern(
		etux_fstree_validate_dirent(&entry->dirent, iter) > 0);

	int ret;

	ret = etux_fstree_entry_type(entry, iter);
	if (ret < 0)
		return ret;

	if (ret == DT_LNK) {
		ret = etux_fstree_setup_entry_type(entry, iter);
		if (ret)
			return ret;
	}

	return 0;
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
static
int
etux_fstree_load_entry(const struct etux_fstree_iterator * __restrict iter,
                       struct etux_fstree_entry * __restrict          entry)
{
	etux_fstree_assert_iter_intern(iter);
	etux_fstree_assert_intern(entry);
	etux_fstree_assert_intern(!entry->flags);

	ssize_t ret;

	ret = etux_fstree_validate_dirent(&entry->dirent, iter);
	if (ret < 0)
		return (int)ret;

	entry->namelen = (size_t)ret;

	if (iter->options & ETUX_FSTREE_FOLLOW_OPT) {
		ret = etux_fstree_resolve_entry(iter, entry);
		if (ret)
			return (int)ret;
	}

	return 0;
}

const char *
etux_fstree_iter_path(const struct etux_fstree_iterator * __restrict iter)
{
	etux_fstree_assert_iter_api(iter);

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
static
int
etux_fstree_iter_next(struct etux_fstree_iterator * __restrict iter,
                      struct etux_fstree_entry * __restrict    entry)
{
	etux_fstree_assert_iter_intern(iter);
	etux_fstree_assert_intern(entry);

	struct dirent * ent;

	errno = 0;
	ent = readdir(iter->dir);
	if (ent) {
		entry->dirent = *ent;
		entry->flags = 0;

		return 0;
	}

	etux_fstree_assert_intern(errno != EBADF);

	if (errno)
		return -errno;

	return -ENOENT;
}

static
int
etux_fstree_init_iter(struct etux_fstree_iterator * __restrict iter,
                      const char * __restrict                  path,
                      int                                      options)
{
	etux_fstree_assert_api(iter);
	etux_fstree_assert_api(!path ||
	                       !path[0] ||
	                       upath_validate_path_name(path) > 0);
	etux_fstree_assert_api(!(options & ~ETUX_FSTREE_VALID_OPTS));

	iter->path = malloc(PATH_MAX);
	if (!iter->path)
		return -ENOMEM;

	if (path && path[0]) {
		size_t len;

		len = strnlen(path, PATH_MAX);
		etux_fstree_assert_intern(len < PATH_MAX);

		memcpy(iter->path, path, len);
		iter->path[len] = '\0';
		iter->pathlen = len;
	}
	else {
		iter->path[0] = '\0';
		iter->pathlen = 0;
	}

	iter->dir = opendir(iter->pathlen ? iter->path : ".");
	if (!iter->dir) {
		int err = errno;

		etux_fstree_assert_intern(err != EBADF);

		free(iter->path);

		return -err;
	}

	iter->options = options;

	return 0;
}

static
void
etux_fstree_fini_iter(struct etux_fstree_iterator * __restrict iter)
{
	etux_fstree_assert_iter_api(iter);

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
	struct etux_fstree_iterator iter;
	int                         ret;
	struct etux_fstree_entry    ent;
	enum etux_fstree_cmd        cmd;

	ret = etux_fstree_init_iter(&iter, path, options);
	if (ret)
		return ret;

	etux_fstree_init_entry(&ent);

	do {
		/* Advance to next directory entry. */
		ret = etux_fstree_iter_next(&iter, &ent);
		etux_fstree_assert_intern(ret <= 0);
		if (!ret) {
			/* Load and validate curent directory entry content. */
			ret = etux_fstree_load_entry(&iter, &ent);
			if (!ret)
				/* Entry has been properly loaded. */
				cmd = handle(&ent,
				             &iter,
				             ETUX_FSTREE_ENT_EVT,
				             &ret,
				             data);
			else if (ret != -ENOMEM)
				/* An error happened while loading entry. */
				cmd = handle(&ent,
				             &iter,
				             ETUX_FSTREE_LOAD_ERR_EVT,
				             &ret,
				             data);
		}
		else if (ret != -ENOENT) {
			/* Current directory iteration failed. */
			if (ret != -ENOMEM)
				handle(NULL,
				       &iter,
				       ETUX_FSTREE_NEXT_ERR_EVT,
				       &ret,
				       data);
			break;
		}
		else
			break;

		if (ret == -ENOMEM)
			/* No more memory: unrecoverable error. */
			break;

		etux_fstree_assert_api((cmd == ETUX_FSTREE_CONT_CMD) ||
		                (cmd == ETUX_FSTREE_STOP_CMD));
	} while (cmd == ETUX_FSTREE_CONT_CMD);

	etux_fstree_fini_entry(&ent);

	etux_fstree_fini_iter(&iter);

	return (ret == -ENOENT) ? 0 : ret;
}
