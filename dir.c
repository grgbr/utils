#include "utils/dir.h"

int
udir_nointr_open(const char *path, int flags)
{
	int fd;

	do {
		fd = udir_open(path, flags);
	} while (fd == -EINTR);

	return fd;
}
