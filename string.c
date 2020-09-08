#include <utils/string.h>
#include <stdlib.h>

size_t
ustr_prefix_len(const char *string,
                size_t      str_len,
                const char *prefix,
                size_t      pref_len)
{
	ustr_assert(string);
	ustr_assert(prefix);

	if (!str_len || ! pref_len || (pref_len > str_len))
		return 0;

	return memcmp(string, prefix, pref_len) ? 0 : pref_len;
}

size_t
ustr_suffix_len(const char *string,
                size_t      str_len,
                const char *suffix,
                size_t      suff_len)
{
	ustr_assert(string);
	ustr_assert(suffix);

	if (!str_len || ! suff_len || (suff_len > str_len))
		return 0;

	return memcmp(string + str_len - suff_len,
	              suffix,
	              suff_len) ? 0 : suff_len;
}

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
