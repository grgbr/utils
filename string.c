/**
 * @defgroup string String
 * String handling
 *
 * @file
 * String implementation
 *
 * @ingroup      string
 * @author       Grégor Boirie <gregor.boirie@free.fr>
 * @date         29 Aug 2017
 * @copyright    Copyright (C) 2017-2021 Grégor Boirie.
 * @licensestart GNU Lesser General Public License (LGPL) v3
 *
 * This file is part of libutils
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, If not, see <http://www.gnu.org/licenses/>.
 * @licenseend
 */
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

	/*
	 * ustr_tolower add \0 at the end, we add 1 to the size.
	 * We copy in str the first 6 bytes. 5 for the case "false" and 1 more
	 * to exclude case "falseX" where the string start with false but have
	 * other char after.
	 */
	char str[7];
	int  ret = 0;

	ustr_tolower(str, string, sizeof(str));

	if (!strcmp(str, "yes") ||
	    !strcmp(str, "y") ||
	    !strcmp(str, "true") ||
	    !strcmp(str, "1"))
		*value = true;
	else if (!strcmp(str, "no") ||
	         !strcmp(str, "n") ||
	         !strcmp(str, "false") ||
	         !strcmp(str, "0"))
		*value = false;
	else
		ret = -EINVAL;

	return ret;
}

int
ustr_parse_base_ullong(const char *string, unsigned long long *value, int base)
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

int
ustr_parse_base_ulong(const char * __restrict    string,
                      unsigned long * __restrict value,
                      int                        base)
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
ustr_parse_uint_range(const char   * __restrict string,
                      unsigned int * __restrict value,
                      unsigned int              min,
                      unsigned int              max)
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
ustr_parse_token_fields(char * __restrict           string,
                        int                         delim,
                        ustr_parse_token_fn * const parsers[__restrict_arr],
                        unsigned int                count,
                        void * __restrict           context)
{
	ustr_assert(string);
	ustr_assert(parsers);
	ustr_assert(count);

	unsigned int cnt = 0;
	char *       str = string;

	while (true) {
		char *                sep;
		size_t                len;
		ustr_parse_token_fn * parse = parsers[cnt];
		int                   ret;
		bool                  out;

		sep = strchrnul(str, delim);
		len = sep - str;
		if (!len)
			return -ENODATA;

		out = !*sep;
		*sep = '\0';

		ret = parse(str, len, context);
		if (ret)
			return ret;

		cnt++;
		ustr_assert(cnt <= count);

		if (out)
			/* End of string: return the number of matches. */
			return cnt;

		if (cnt == count)
			/* Trailing characters in excess: tell the caller. */
			return -EMSGSIZE;

		str = sep + 1;
	}
}

int
ustr_parse_each_token(char * __restrict     string,
                      int                   delim,
                      ustr_parse_token_fn * parse,
                      void * __restrict     context)
{
	ustr_assert(string);
	ustr_assert(parse);

	unsigned int cnt = 0;
	char *       str = string;

	while (true) {
		char * sep;
		size_t len;
		int    ret;
		bool   out;

		sep = strchrnul(str, delim);
		len = sep - str;
		if (!len)
			return -ENODATA;

		out = !*sep;
		*sep = '\0';

		ret = parse(str, len, context);
		if (ret)
			return ret;

		cnt++;

		if (out)
			/* End of string: return the number of matches. */
			return cnt;

		str = sep + 1;
	}
}
