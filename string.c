#include <utils/string.h>
#include <stdlib.h>

char *
ustr_clone(const char *orig, size_t len)
{
	ustr_assert(orig);

	char *str;

	str = malloc(len + 1);
	if (!str) {
		errno = ENOMEM;
		return NULL;
	}

	memcpy(str, orig, len);
	str[len] = '\0';

	return str;
}

char *
ustr_sized_clone(const char *orig, size_t max_size)
{
	ustr_assert(orig);
	ustr_assert(max_size);

	ssize_t len;

	len = ustr_parse(orig, max_size);
	if (len < 0) {
		errno = -len;
		return NULL;
	}

	return ustr_clone(orig, len);
}
