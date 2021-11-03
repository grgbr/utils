#include <utils/file.h>

ssize_t __ufile_nonull(2) __warn_result
ufile_nointr_read(int fd, char *data, size_t size)
{
	ssize_t ret;

	do {
		ret = ufile_read(fd, data, size);
	} while (ret == -EINTR);

	return ret;
}

int __ufile_nonull(2) __warn_result
ufile_nointr_full_read(int fd, char *data, size_t size)
{
	ufile_assert(fd >= 0);
	ufile_assert(!(!!data ^ !!size));

	unsigned int off = 0;

	while (size) {
		ssize_t ret;

		ret = ufile_nointr_read(fd, &data[off], size);
		if (ret > 0) {
			off += ret;
			size -= ret;
		}
		else if (ret != -EAGAIN)
			return !ret ? -ENODATA : ret;
	}

	return 0;
}

ssize_t __ufile_nonull(2) __warn_result
ufile_nointr_write(int fd, const char *data, size_t size)
{
	ssize_t ret;

	do {
		ret = ufile_write(fd, data, size);
	} while (ret == -EINTR);

	return ret;
}

int __ufile_nonull(2) __warn_result
ufile_nointr_full_write(int fd, const char *data, size_t size)
{
	ufile_assert(fd >= 0);
	ufile_assert(!(!!data ^ !!size));

	unsigned int off = 0;

	while (size) {
		ssize_t ret;

		ret = ufile_nointr_write(fd, &data[off], size);
		if (ret > 0) {
			off += ret;
			size -= ret;
		}
		else if (ret != -EAGAIN)
			return ret;
	}

	return 0;
}

int __ufile_nonull(1)
ufile_nointr_open(const char *path, int flags)
{
	int fd;

	do {
		fd = ufile_open(path, flags);
	} while (fd == -EINTR);

	return fd;
}

int __ufile_nonull(2)
ufile_nointr_open_at(int dir, const char *path, int flags)
{
	int fd;

	do {
		fd = ufile_open_at(dir, path, flags);
	} while (fd == -EINTR);

	return fd;
}

int __ufile_nonull(1)
ufile_nointr_new(const char *path, int flags, mode_t mode)
{
	int fd;

	do {
		fd = ufile_new(path, flags, mode);
	} while (fd == -EINTR);

	return fd;
}

int __ufile_nonull(2)
ufile_nointr_new_at(int dir, const char *path, int flags, mode_t mode)
{
	int fd;

	do {
		fd = ufile_new_at(dir, path, flags, mode);
	} while (fd == -EINTR);

	return fd;
}
