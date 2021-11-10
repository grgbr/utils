#include "utils/fd.h"
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>

ssize_t
ufd_nointr_write(int fd, const char *data, size_t size)
{
	ssize_t ret;

	do {
		ret = ufd_write(fd, data, size);
	} while (ret == -EINTR);

	return ret;
}

#if !defined(__NR_close_range) || !defined(__USE_GNU)

int
ufd_close_fds(unsigned int first, unsigned int last)
{
	ufd_assert(first <= last);

	DIR * dir;
	int   ret;

	dir = opendir("/proc/self/fd");
	if (!dir) {
		ufd_assert(errno != EBADF);
		return -errno;
	}

	while (true) {
		struct dirent * ent;

		errno = 0;
		ent = readdir(dir);
		if (!ent) {
			ufd_assert(errno != EBADF);
			break;
		}

		if (ent->d_type == DT_LNK) {
			unsigned long fd;
			char *        err;

			fd = strtoul(ent->d_name, &err, 10);
			if (!*err &&
			    ((int)fd != dirfd(dir)) &&
			    (fd >= first) && (fd <= last))
				ufd_close(fd);
		}
	}

	ret = -errno;

	closedir(dir);

	return ret;
}

#endif /* !defined(__NR_close_range) || !defined(__USE_GNU) */
