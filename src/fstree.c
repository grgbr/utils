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

/******************************************************************************
 * Simple filesystem tree iterator
 ******************************************************************************/

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

		if (plen) {
			memcpy(entry->path, iter->path, plen);
			if (iter->path[plen - 1] != '/')
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

/******************************************************************************
 * Internal recursive filesystem tree walk path tracking.
 ******************************************************************************/

struct etux_fstree_point {
	dev_t           dev;
	ino_t           ino;
	DIR *           dir;
	size_t          len;
	struct dirent * ent;
};

struct etux_fstree_track {
	unsigned int               cnt;
	unsigned int               nr;
	struct etux_fstree_point * pts;
};

struct etux_fstree_scan {
	struct etux_fstree_iterator iter;
	dev_t                       dev;
	ino_t                       ino;
	struct etux_fstree_track    track;
};

#define ETUX_FSTREE_TRACK_MIN_NR (8U)

#define etux_fstree_assert_track_api(_trk) \
	etux_fstree_assert_api(_trk); \
	etux_fstree_assert_api((_trk)->nr >= ETUX_FSTREE_TRACK_MIN_NR); \
	etux_fstree_assert_api((_trk)->cnt <= (_trk)->nr); \
	etux_fstree_assert_api((_trk)->pts)

#define etux_fstree_assert_track_intern(_trk) \
	etux_fstree_assert_intern(_trk); \
	etux_fstree_assert_intern((_trk)->nr >= ETUX_FSTREE_TRACK_MIN_NR); \
	etux_fstree_assert_intern((_trk)->cnt <= (_trk)->nr); \
	etux_fstree_assert_intern((_trk)->pts)

#define etux_fstree_track_foreach(_trk, _pt) \
	for ((_pt) = (_trk)->pts; \
	     (_pt) < &(_trk)->pts[etux_fstree_track_count(_trk)]; \
	     (_pt)++)

static inline
unsigned int
etux_fstree_track_count(const struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_track_intern(track);

	return track->cnt;
}

static
int
etux_fstree_track_grow(struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_track_intern(track);

	struct etux_fstree_point * pts = track->pts;
	unsigned int               nr = 2 * track->nr;

	pts = realloc(pts, nr * sizeof(*pts));
	if (!pts)
		return -ENOMEM;

	track->pts = pts;
	track->nr = nr;

	return 0;
}

static
struct etux_fstree_point *
etux_fstree_track_push(struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_track_intern(track);

	if (track->cnt == track->nr) {
		int err;

		err = etux_fstree_track_grow(track);
		if (err) {
			errno = -ENOMEM;
			return NULL;
		}
	}

	return &track->pts[track->cnt++];
}

static __utils_nonull(1) __utils_nothrow __returns_nonull __warn_result
const struct etux_fstree_point *
etux_fstree_track_pop(struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_track_intern(track);
	etux_fstree_assert_intern(track->cnt);

	return &track->pts[--track->cnt];
}

static
int
etux_fstree_track_init(struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_intern(track);

	struct etux_fstree_point * pts;

	pts = malloc(ETUX_FSTREE_TRACK_MIN_NR * sizeof(*pts));
	if (!pts)
		return -ENOMEM;

	track->cnt = 0;
	track->nr = ETUX_FSTREE_TRACK_MIN_NR;
	track->pts = pts;

	return 0;
}

static
void
etux_fstree_track_fini(struct etux_fstree_track * __restrict track)
{
	etux_fstree_assert_track_intern(track);

	free(track->pts);
}

/******************************************************************************
 * Recursive filesystem tree scanner.
 ******************************************************************************/

struct etux_fstree_scanner {
	struct etux_fstree_iterator iter;
	dev_t                       dev;
	ino_t                       ino;
	struct etux_fstree_track    track;
};

#define etux_fstree_assert_scan_api(_scan) \
	etux_fstree_assert_iter_api(&(_scan)->iter); \
	etux_fstree_assert_track_api(&(_scan)->track)

#define etux_fstree_assert_scan_intern(_scan) \
	etux_fstree_assert_iter_intern(&(_scan)->iter); \
	etux_fstree_assert_track_intern(&(_scan)->track)

#define etux_fstree_assert_scan_entry_api(_ent, _scan) \
	etux_fstree_assert_entry_api(_ent, &(_scan)->iter); \
	etux_fstree_assert_track_api(&(_scan)->track)

#define etux_fstree_assert_scan_entry_intern(_ent, _scan) \
	etux_fstree_assert_entry_intern(_ent, &(_scan)->iter); \
	etux_fstree_assert_track_intern(&(_scan)->track)

static
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

static
int
etux_fstree_isxdev_scan_entry(
	struct etux_fstree_entry * __restrict         entry,
	const struct etux_fstree_scanner * __restrict scan)
{
	etux_fstree_assert_scan_entry_intern(entry, scan);

	const struct stat * st;

	st = etux_fstree_entry_stat(entry, &scan->iter);
	if (!st)
		return -errno;

	if (st->st_dev != scan->dev)
		return 1;

	return 0;
}

static
int
etux_fstree_isloop_scan_entry(
	struct etux_fstree_entry * __restrict         entry,
	const struct etux_fstree_scanner * __restrict scan)
{
	etux_fstree_assert_scan_entry_intern(entry, scan);

	if (scan->iter.options & ETUX_FSTREE_FOLLOW_OPT) {
		const struct stat *              st;
		const struct etux_fstree_point * pt;

		st = etux_fstree_entry_stat(entry, &scan->iter);
		if (!st)
			return -errno;

		if ((st->st_dev == scan->dev) && (st->st_ino == scan->ino))
			return 1;

		etux_fstree_track_foreach(&scan->track, pt)
			if ((st->st_dev == pt->dev) && (st->st_ino == pt->ino))
				return 1;
	}

	return 0;
}





















FINISH ME!!!!!

static
int
etux_fstree_enter_scan_dir(
	struct etux_fstree_entry * __restrict         entry,
	const struct etux_fstree_scanner * __restrict scan)
{
	etux_fstree_assert_scan_entry_intern(entry, scan);
	etux_fstree_assert_intern(
		etux_fstree_validate_dirent(&scan->iter.dirent,
		                            &scan->iter) > 0);

	const struct stat *        st;
	int                        flags;
	int                        fd;    /* file desc of current directory */
	DIR *                      dir;   /* child directory */
	struct etux_fstree_point * pt;

	if (scan->iter.options & ETUX_FSTREE_FOLLOW_OPT) {
		st = etux_fstree_entry_stat(entry, &scan->iter);
		if (!st)
			return -errno;

		flags = 0;
	}
	else {
		st = NULL;
		flags = O_NOFOLLOW;
	}

	fd = dirfd(scan->iter.dir);
	etux_fstree_assert_intern(fd >= 0);

	dir = etux_fstree_open_dir_at(fd, entry->d_name, flags);
	if (!dir)
		return -errno;

	pt = etux_fstree_track_push(&scan->track);
	if (!pt) {
		closedir(dir);
		return -ENOMEM;
	}

	if (st) {
		pt->dev = st->st_dev;
		pt->ino = st->st_ino;
	}
	pt->dir = scan->iter.dir;
	pt->len = scan->iter.plen;
	pt->ent = entry;

	scan->iter.dir = dir;
	scan->iter.plen = udir_iter_build_entry_path(&scan->iter, entry);
	etux_fstree_assert_intern(scan->iter.plen > pt->len);

	return 0;
}























#if 0

static
struct dirent *
udir_scan_exit_dir(struct udir_scan * __restrict scan)
{
	etux_fstree_assert_scan_intern(scan);

	if (udir_scan_stk_count(&scan->track)) {
		const struct etux_fstree_point * pt;

		/* Close current directory stream. */
		closedir(scan->iter.dir);

		pt = etux_fstree_track_pop(&scan->track);
		etux_fstree_assert_intern(pt);

		/* Restore parent directory stream and state. */
		scan->iter.flags &= ~UDIR_ITER_PATH_FLAG;
		scan->iter.dir = pt->dir;
		scan->iter.nlen = strnlen(pt->ent->d_name, NAME_MAX);
		etux_fstree_assert_intern(scan->iter.nlen);
		scan->iter.plen = pt->len;

		return pt->ent;
	}
	else
		return NULL;
}

int
udir_scan_next(struct udir_scan * __restrict scan,
               struct dirent ** __restrict   entry)
{
	udir_scan_assert_api(scan);
	etux_fstree_assert_api(entry);

	return udir_iter_next(&scan->iter, entry);
}

/*
 * Load current directory entry then check if it is suitable for recursing down
 * the filesystem tree.
 *
 * Return values:
 * >= 0     -- the type of entry as defined by <dirent.h> DT_ macros
 * 0        -- not suitable for recursing
 * < 0      -- negative errno like value indicating failure while retrieving
 *             entry's attributes
 */
int
udir_scan_load_entry(struct udir_scan * __restrict scan,
                     struct dirent * __restrict    entry)
{
	udir_scan_assert_api(scan);
	etux_fstree_assert_api(entry);

	int ret;

	ret = udir_iter_load_entry(&scan->iter, entry);
	if (ret < 0)
		return ret;

	ret = udir_iter_entry_type(&scan->iter, entry);
	if (ret < 0)
		return ret;
	else if (ret != DT_DIR)
		return 0;

	if (udir_iter_entry_isdot(&scan->iter, entry))
		return 0;

	if (!(scan->iter.options & UDIR_ITER_XDEV_OPT)) {
		ret = etux_fstree_isxdev_scan_entry(entry, scan);
		if (ret < 0)
			return ret;
		else if (ret)
			return 0;
	}

	return 1;
}

int
udir_scan_init(struct udir_scan * __restrict scan,
               const char * __restrict       path,
               unsigned int                  opts)
{
	etux_fstree_assert_api(scan);
	etux_fstree_assert_api(!path ||
	                !path[0] ||
	                upath_validate_path_name(path) > 0);
	etux_fstree_assert_api(!(opts & ~UDIR_ITER_VALID_OPTS));

	int         err;
	int         fd;
	struct stat st;

	err = udir_iter_init(&scan->iter, path, opts);
	if (err)
		return err;

	fd = dirfd(scan->iter.dir);
	etux_fstree_assert_intern(fd >= 0);

	err = ufd_fstat(fd, &st);
	if (err)
		goto fini;

	err = udir_scan_init_stk(&scan->track);
	if (err)
		goto fini;

	scan->dev = st.st_dev;
	scan->ino = st.st_ino;

	return 0;

fini:
	udir_iter_fini(&scan->iter);

	return err;
}

void
udir_scan_fini(struct udir_scan * __restrict scan)
{
	udir_scan_assert_api(scan);

	/*
	 * Close all directory streams left in case of premature interruption
	 * of recursive walk interruption.
	 */
	while (udir_scan_stk_count(&scan->track)) {
		const struct etux_fstree_point * pt;

		/* Close current directory stream. */
		closedir(scan->iter.dir);

		pt = etux_fstree_track_pop(&scan->track);
		etux_fstree_assert_intern(pt);

		scan->iter.dir = pt->dir;
	}

	udir_scan_fini_stk(&scan->track);

	udir_iter_fini(&scan->iter);
}

static
enum udir_iter_cmd
udir_scan_enter_entry(struct udir_scan * __restrict scan,
                      struct dirent * __restrict    entry,
                      int * __restrict              status,
                      udir_handle_entry_fn *        handle,
                      void *                        data)
{
	etux_fstree_assert_scan_intern(scan);
	etux_fstree_assert_intern(entry);
	etux_fstree_assert_intern(status);
	etux_fstree_assert_intern(*status == 1);
	etux_fstree_assert_intern(handle);

	int ret;

	ret = etux_fstree_isloop_scan_entry(scan, entry);
	if (ret) {
		if (ret == 1) {
			/* Symlink to directory loop detected: don't recurse. */
			*status = 0;

			return handle(&scan->iter,
			              entry,
			              UDIR_ITER_LOOP_EVT,
			              status,
			              data);
		}

		*status = ret;

		if (ret != -ENOMEM)
			/* Failure while retrieving entry attributes. */
			return handle(&scan->iter,
			              entry,
			              UDIR_ITER_LOAD_ERR_EVT,
			              status,
			              data);
		else
			/* No more memory: unrecoverable error. */
			return UDIR_ITER_STOP_CMD;
	}

	*status = 0;

	if (scan->iter.options & UDIR_ITER_PRE_OPT) {
		/*
		 * Make sure that the handler really wants to recurse into this
		 * subdirectory.
		 */
		switch (handle(&scan->iter,
		               entry,
		               UDIR_ITER_PRE_EVT,
		               status,
		               data)) {
		case UDIR_ITER_CONT_CMD:
			break;
		case UDIR_ITER_SKIP_CMD:
			return UDIR_ITER_CONT_CMD;
		case UDIR_ITER_STOP_CMD:
			return UDIR_ITER_STOP_CMD;
		default:
			etux_fstree_assert_api(0);
		}
	}

	ret = etux_fstree_enter_scan_dir(scan, entry);
	if (!ret)
		/* Entering the child directory succeeded. */
		return UDIR_ITER_CONT_CMD;

	*status = ret;

	if (ret != -ENOMEM)
		/* Entering the child directory failed. */
		return handle(&scan->iter,
		              entry,
		              UDIR_ITER_DIR_ERR_EVT,
		              status,
		              data);
	else
		/* No more memory: unrecoverable error. */
		return UDIR_ITER_STOP_CMD;
}

static
enum udir_iter_cmd
udir_scan_exit_entry(struct udir_scan * __restrict scan,
                     int * __restrict              status,
                     udir_handle_entry_fn *        handle,
                     void *                        data)
{
	etux_fstree_assert_scan_intern(scan);
	etux_fstree_assert_intern(status);
	etux_fstree_assert_intern(*status == -ENOENT);
	etux_fstree_assert_intern(handle);

	struct dirent * ent;

	*status = 0;

	ent = udir_scan_exit_dir(scan);
	if (!ent)
		return UDIR_ITER_STOP_CMD;

	if (!(scan->iter.options & UDIR_ITER_POST_OPT))
		return UDIR_ITER_CONT_CMD;

	return handle(&scan->iter, ent, UDIR_ITER_POST_EVT, status, data);
}

int
udir_scan(struct udir_scan * __restrict scan,
          const char * __restrict       path,
          unsigned int                  opts,
          udir_handle_entry_fn *        handle,
          void *                        data)

{
	int                ret;
	enum udir_iter_cmd cmd;

	ret = udir_scan_init(scan, path, opts);
	if (ret)
		return ret;

	do {
		struct dirent * ent;

		/* Advance to next directory entry. */
		ret = udir_scan_next(scan, &ent);
		if (!ret) {
			/* Load and validate curent directory entry content. */
			ret = udir_scan_load_entry(scan, ent);
			if (!ret)
				/* Entry is not suitable for recursion. */
				cmd = handle(&scan->iter,
				             ent,
				             UDIR_ITER_ENT_EVT,
				             &ret,
				             data);
			else if (ret == 1)
				/* Recursion required. */
				cmd = udir_scan_enter_entry(scan,
				                            ent,
				                            &ret,
				                            handle,
				                            data);
			else if (ret != -ENOMEM)
				/* An error happened while loading entry. */
				cmd = handle(&scan->iter,
				             ent,
				             UDIR_ITER_LOAD_ERR_EVT,
				             &ret,
				             data);
		}
		else if (ret != -ENOENT) {
			/* Current directory iteration failed. */
			etux_fstree_assert_intern(ret < 0);

			if (ret != -ENOMEM)
				handle(&scan->iter,
				       NULL,
				       UDIR_ITER_NEXT_ERR_EVT,
				       &ret,
				       data);
		}
		else
			/* End of current directory iteration. */
			cmd = udir_scan_exit_entry(scan, &ret, handle, data);

		if (ret == -ENOMEM)
			/* No more memory: unrecoverable error. */
			break;

		etux_fstree_assert_api(cmd >= 0);
		etux_fstree_assert_api(cmd < UDIR_ITER_CMD_NR);
	} while (cmd == UDIR_ITER_CONT_CMD);

	udir_scan_fini(scan);

	return (ret == -ENOENT) ? 0 : ret;
}

#endif
