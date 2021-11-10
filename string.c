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

static int
_ustr_parse_ullong(const char *string, unsigned long long *value, int base)
{
	ustr_assert(string);
	ustr_assert(value);
	ustr_assert(!base || (base >= 2 && base <= 36));

	unsigned long long  val;
	char               *err;

	if (!*string)
		return -EINVAL;

	val = strtoull(string, &err, base);
	if (*err)
		return -EINVAL;

	*value = val;

	return 0;
}

int
ustr_parse_ullong(const char *string, unsigned long long *value)
{
	return _ustr_parse_ullong(string, value, 0);
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
ustr_parse_xllong(const char *string, unsigned long long *value)
{
	return _ustr_parse_ullong(string, value, 16);
}

int
ustr_parse_xllong_range(const char         *string,
                        unsigned long long *value,
                        unsigned long long  min,
                        unsigned long long  max)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long long val;
	int                err;

	err = ustr_parse_xllong(string, &val);
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

static int
_ustr_parse_ulong(const char *string, unsigned long *value, int base)
{
	ustr_assert(string);
	ustr_assert(value);
	ustr_assert(!base || (base >= 2 && base <= 36));

	unsigned long  val;
	char          *err;

	if (!*string)
		return -EINVAL;

	val = strtoul(string, &err, base);
	if (*err)
		return -EINVAL;

	*value = val;

	return 0;
}

int
ustr_parse_ulong(const char *string, unsigned long *value)
{
	return _ustr_parse_ulong(string, value, 0);
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
ustr_parse_xlong(const char *string, unsigned long *value)
{
	return _ustr_parse_ulong(string, value, 16);
}

int
ustr_parse_xlong_range(const char    *string,
                       unsigned long *value,
                       unsigned long  min,
                       unsigned long  max)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long val;
	int           err;

	err = ustr_parse_xlong(string, &val);
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
ustr_parse_xint_range(const char   *string,
                      unsigned int *value,
                      unsigned int  min,
                      unsigned int  max)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long val;
	int           err;

	err = ustr_parse_xlong(string, &val);
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
ustr_parse_xshrt_range(const char     *string,
                       unsigned short *value,
                       unsigned short  min,
                       unsigned short  max)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long val;
	int           err;

	err = ustr_parse_xlong(string, &val);
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
ustr_parse_xchar_range(const char    *string,
                       unsigned char *value,
                       unsigned char  min,
                       unsigned char  max)
{
	ustr_assert(string);
	ustr_assert(value);

	unsigned long val;
	int           err;

	err = ustr_parse_xlong(string, &val);
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
ustr_skip_space(const char *string, size_t size)
{
	ustr_assert(string);
	ustr_assert(size);

	const char *str = string;

	while ((str < (string + size)) && isspace(*str))
		str++;

	return str - string;
}

size_t
ustr_rskip_space(const char *string, size_t size)
{
	ustr_assert(string);
	ustr_assert(size);

	const char *str = string + size - 1;

	while ((str >= string) && isspace(*str))
		str--;

	return size - (size_t)((str + 1) - string);
}

size_t
ustr_skip_notspace(const char *string, size_t size)
{
	ustr_assert(string);
	ustr_assert(size);

	const char *str = string;

	while ((str < (string + size)) && *str && !isspace(*str))
		str++;

	return str - string;
}

size_t
ustr_rskip_notspace(const char *string, size_t size)
{
	ustr_assert(string);
	ustr_assert(size);

	const char *str = string + size - 1;

	if (!*str)
		return 0;

	while ((str >= string) && !isspace(*str))
		str--;

	return size - (size_t)((str + 1) - string);
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

size_t
ustr_prefix_len(const char * __restrict string,
                size_t                  str_len,
                const char * __restrict prefix,
                size_t                  pref_len)
{
	ustr_assert(string);
	ustr_assert(prefix);

	if (!str_len || ! pref_len || (pref_len > str_len))
		return 0;

	return memcmp(string, prefix, pref_len) ? 0 : pref_len;
}

size_t
ustr_suffix_len(const char * __restrict string,
                size_t                  str_len,
                const char * __restrict suffix,
                size_t                  suff_len)
{
	ustr_assert(string);
	ustr_assert(suffix);

	if (!str_len || ! suff_len || (suff_len > str_len))
		return 0;

	return memcmp(string + str_len - suff_len,
	              suffix,
	              suff_len) ? 0 : suff_len;
}

int
ustr_parse_token_chain(ustr_parse_token_fn * const parsers[__restrict_arr],
                       unsigned int                count,
                       int                         delim,
                       const char * __restrict     string,
                       size_t                      size,
                       void * __restrict           context)
{
	ustr_assert(parsers);
	ustr_assert(count);
	ustr_assert(string);

	unsigned int p = 0;

	if (size) {
		const char * str = string;
		int          ret;

		do {
			ustr_parse_token_fn * parse = parsers[p];
			size_t                len;

			len = ustr_skip_notchar(str, delim, size);
			if (!len)
				/* Delimiter found with missing token. */
				return -ENODATA;

			ret = parse(str, len, context);
			if (ret <= 0)
				/* No match: bail out. */
				break;

			/* Update count of parsed tokens. */
			p++;

			len = umin(len + 1, size);
			str = &str[len];
			size -= len;
		} while (size && (p < count));

		if (ret <= 0) {
			/* Parsing error or failed to match token. */
			if (!ret)
				/* Token not matched. */
				ret = -EBADMSG;
			return ret;
		}

		if (size) {
			/* Tokens in excess. */
			ustr_assert(p == count);
			return -EMSGSIZE;
		}

		if (str[-1] == delim)
			/*
			 * String ends with delimiter (within specified size),
			 * meaning that last token is missing.
			 */
			return -ENODATA;
	}

	/* Return the number of matched / parsed tokens. */
	return p;
}

int
ustr_foreach_token(ustr_parse_token_fn *   parse,
                   int                     delim,
                   const char * __restrict string,
                   size_t                  size,
                   void * __restrict       context)
{
	ustr_assert(parse);
	ustr_assert(string);

	unsigned int cnt = 0;

	if (size) {
		const char * str = string;

		do {
			size_t len;
			int    ret;

			len = ustr_skip_notchar(str, delim, size);
			if (!len)
				/* Delimiter found with missing token. */
				return -ENODATA;

			ret = parse(str, len, context);
			if (ret <= 0) {
				if (!ret)
					/* Token not matched. */
					ret = -EBADMSG;
				return ret;
			}

			cnt++;

			len = umin(len + 1, size);
			str = &str[len];
			size -= len;
		} while (size);

		if (str[-1] == delim)
			/*
			 * String ends with delimiter (within specified size),
			 * meaning that last token is missing.
			 */
			return -ENODATA;
	}

	/* Return the number of matches. */
	return cnt;
}
