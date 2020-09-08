#include <utils/string.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

void
ustr_tolower(char *lower, const char *orig, size_t size)
{
	ustr_assert(lower);
	ustr_assert(orig);
	ustr_assert(size);

	unsigned int c;

	for (c = 0; (c < (size - 1)) && orig[c]; c++)
		lower[c] = tolower(orig[c]);

	lower[c] = '\0';
}

void
ustr_tolower_inp(char *string, size_t size)
{
	ustr_assert(string);
	ustr_assert(size);

	unsigned int c;

	for (c = 0; (c < (size - 1)) && string[c]; c++)
		string[c] = tolower(string[c]);

	string[c] = '\0';
}

void
ustr_toupper(char *upper, const char *orig, size_t size)
{
	ustr_assert(upper);
	ustr_assert(orig);
	ustr_assert(size);

	unsigned int c;

	for (c = 0; (c < (size - 1)) && orig[c]; c++)
		upper[c] = toupper(orig[c]);

	upper[c] = '\0';
}

void
ustr_toupper_inp(char *string, size_t size)
{
	ustr_assert(string);
	ustr_assert(size);

	unsigned int c;

	for (c = 0; (c < (size - 1)) && string[c]; c++)
		string[c] = toupper(string[c]);

	string[c] = '\0';
}

int
ustr_parse_bool(const char *string, bool *value)
{
	ustr_assert(string);
	ustr_assert(value);

	char str[5];
	int  ret = 0;

	ustr_tolower(str, string, sizeof(str));

	if (!strcmp(string, "yes") ||
	    !strcmp(string, "y") ||
	    !strcmp(string, "true") ||
	    !strcmp(string, "1"))
		*value = true;
	else if (!strcmp(string, "no") ||
	         !strcmp(string, "n") ||
	         !strcmp(string, "false") ||
	         !strcmp(string, "0"))
		*value = false;
	else
		ret = -EINVAL;

	return ret;
}

int
ustr_parse_ullong(const char *string, unsigned long long *value)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long long  val;
	char               *err;

	if (!*string)
		return -EINVAL;

	val = strtoull(string, &err, 0);
	if (*err)
		return -EINVAL;

	*value = val;

	return 0;
}

int
ustr_parse_ullong_range(const char         *string,
                        unsigned long long *value,
                        unsigned long long  min,
                        unsigned long long  max)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long long val;
	int                err;

	err = ustr_parse_ullong(string, &val);
	if (err)
		return err;

	if ((val < min) || (val > max))
		return -ERANGE;

	*value = val;

	return 0;
}

int
ustr_parse_llong(const char *string, long long *value)
{
	ustr_assert(string);
	ustr_assert(value);

	long long  val;
	char      *err;

	if (!*string)
		return -EINVAL;

	val = strtoll(string, &err, 0);
	if (*err)
		return -EINVAL;

	*value = val;

	return 0;
}

int
ustr_parse_llong_range(const char *string,
                       long long  *value,
                       long long   min,
                       long long   max)
{
	ustr_assert(string);
	ustr_assert(value);

	long long val;
	int       err;

	err = ustr_parse_llong(string, &val);
	if (err)
		return err;

	if ((val < min) || (val > max))
		return -ERANGE;

	*value = val;

	return 0;
}

int
ustr_parse_ulong(const char *string, unsigned long *value)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long  val;
	char          *err;

	if (!*string)
		return -EINVAL;

	val = strtoul(string, &err, 0);
	if (*err)
		return -EINVAL;

	*value = val;

	return 0;
}

int
ustr_parse_ulong_range(const char    *string,
                       unsigned long *value,
                       unsigned long  min,
                       unsigned long  max)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long val;
	int           err;

	err = ustr_parse_ulong(string, &val);
	if (err)
		return err;

	if ((val < min) || (val > max))
		return -ERANGE;

	*value = val;

	return 0;
}

int
ustr_parse_long(const char *string, long *value)
{
	ustr_assert(string);
	ustr_assert(value);

	long  val;
	char *err;

	if (!*string)
		return -EINVAL;

	val = strtol(string, &err, 0);
	if (*err)
		return -EINVAL;

	*value = val;

	return 0;
}

int
ustr_parse_long_range(const char *string, long *value, long min, long max)
{
	ustr_assert(string);
	ustr_assert(value);

	long val;
	int  err;

	err = ustr_parse_long(string, &val);
	if (err)
		return err;

	if ((val < min) || (val > max))
		return -ERANGE;

	*value = val;

	return 0;
}

int
ustr_parse_uint_range(const char   *string,
                      unsigned int *value,
                      unsigned int  min,
                      unsigned int  max)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long val;
	int           err;

	err = ustr_parse_ulong(string, &val);
	if (err)
		return err;

	if ((val < (unsigned long)min) || (val > (unsigned long)max))
		return -ERANGE;

	*value = (unsigned int)val;

	return 0;
}

int
ustr_parse_int_range(const char *string, int *value, int min, int max)
{
	ustr_assert(string);
	ustr_assert(value);

	long val;
	int  err;

	err = ustr_parse_long(string, &val);
	if (err)
		return err;

	if ((val < (long)min) || (val > (long)max))
		return -ERANGE;

	*value = (int)val;

	return 0;
}

int
ustr_parse_ushrt_range(const char     *string,
                       unsigned short *value,
                       unsigned short  min,
                       unsigned short  max)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long val;
	int           err;

	err = ustr_parse_ulong(string, &val);
	if (err)
		return err;

	if ((val < (unsigned long)min) || (val > (unsigned long)max))
		return -ERANGE;

	*value = (unsigned short)val;

	return 0;
}

int
ustr_parse_shrt_range(const char *string, short *value, short min, short max)
{
	ustr_assert(string);
	ustr_assert(value);

	long val;
	int  err;

	err = ustr_parse_long(string, &val);
	if (err)
		return err;

	if ((val < (long)min) || (val > (long)max))
		return -ERANGE;

	*value = (short)val;

	return 0;
}

int
ustr_parse_uchar_range(const char    *string,
                       unsigned char *value,
                       unsigned char  min,
                       unsigned char  max)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long val;
	int           err;

	err = ustr_parse_ulong(string, &val);
	if (err)
		return err;

	if ((val < (unsigned long)min) || (val > (unsigned long)max))
		return -ERANGE;

	*value = (unsigned char)val;

	return 0;
}

int
ustr_parse_char_range(const char  *string,
                      signed char *value,
                      signed char  min,
                      signed char  max)
{
	ustr_assert(string);
	ustr_assert(value);

	long val;
	int  err;

	err = ustr_parse_long(string, &val);
	if (err)
		return err;

	if ((val < (long)min) || (val > (long)max))
		return -ERANGE;

	*value = (signed char)val;

	return 0;
}

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
