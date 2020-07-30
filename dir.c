#include <utils/dir.h>

int __udir_nonull(1)
udir_nointr_open(const char *path, int flags)
{
	int fd;

	do {
		fd = udir_open(path, flags);
	} while (fd == -EINTR);

	return fd;
}

int
udir_nointr_close(int fd)
{
	int ret;

	do {
		ret = udir_close(fd);
	} while (ret == -EINTR);

	return ret;
}
