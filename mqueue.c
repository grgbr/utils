#include <utils/mqueue.h>
#include <utils/path.h>

ssize_t
umq_validate_name(const char * __restrict name)
{
	umq_assert(name);

	if (name[0] == '/') {
		ssize_t len;

		len = upath_validate_file_name(&name[1]);
		if (len < 0)
			return len;

		if (memchr(&name[1], '/', len))
			return -EISDIR;

		return len + 1;
	}
	else if (name[0] == '\0')
		return -ENODATA;
	else
		return -EINVAL;
}
