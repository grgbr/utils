/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * String interface
 *
 * A set of string manipulation utilities.
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2017
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_STRING_H
#define _UTILS_STRING_H

#include <utils/cdefs.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define ustr_assert_api(_expr) \
	stroll_assert("utils:ustr", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define ustr_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

extern void
ustr_tolower(char * __restrict lower, const char * __restrict orig, size_t size)
	__utils_nonull(1, 2) __utils_nothrow __leaf;

extern void
ustr_tolower_inp(char *string, size_t size)
	__utils_nonull(1) __utils_nothrow __leaf;

extern void
ustr_toupper(char * __restrict upper, const char * __restrict orig, size_t size)
	__utils_nonull(1, 2) __utils_nothrow __leaf;

extern void
ustr_toupper_inp(char * __restrict string, size_t size)
	__utils_nonull(1) __utils_nothrow __leaf;

extern int
ustr_parse_bool(const char * __restrict string, bool * __restrict value)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

extern int
ustr_parse_base_ullong(const char * __restrict         string,
                       unsigned long long * __restrict value,
                       int                             base)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_ullong(const char * __restrict         string,
                  unsigned long long * __restrict value)
{
	return ustr_parse_base_ullong(string, value, 0);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_xllong(const char * __restrict         string,
                  unsigned long long * __restrict value)
{
	return ustr_parse_base_ullong(string, value, 16);
}

extern int
ustr_parse_ullong_range(const char * __restrict         string,
                        unsigned long long * __restrict value,
                        unsigned long long              min,
                        unsigned long long              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

extern int
ustr_parse_xllong_range(const char * __restrict         string,
                        unsigned long long * __restrict value,
                        unsigned long long              min,
                        unsigned long long              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

extern int
ustr_parse_llong(const char * __restrict string, long long * __restrict value)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

extern int
ustr_parse_llong_range(const char * __restrict string,
                       long long  * __restrict value,
                       long long               min,
                       long long               max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

extern int
ustr_parse_base_ulong(const char * __restrict    string,
                      unsigned long * __restrict value,
                      int                        base)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

static inline int __utils_nonull(1, 2) __utils_nothrow __warn_result
ustr_parse_ulong(const char * __restrict    string,
                 unsigned long * __restrict value)
{
	return ustr_parse_base_ulong(string, value, 0);
}

extern int
ustr_parse_ulong_range(const char * __restrict    string,
                       unsigned long * __restrict value,
                       unsigned long              min,
                       unsigned long              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

static inline int __utils_nonull(1, 2) __utils_nothrow __warn_result
ustr_parse_xlong(const char * __restrict string,
                 unsigned long * __restrict value)
{
	return ustr_parse_base_ulong(string, value, 16);
}

extern int
ustr_parse_xlong_range(const char * __restrict    string,
                       unsigned long * __restrict value,
                       unsigned long              min,
                       unsigned long              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

extern int
ustr_parse_long(const char * __restrict string, long * __restrict value)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

extern int
ustr_parse_long_range(const char * __restrict string,
                      long * __restrict       value,
                      long                    min,
                      long                    max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

#if __WORDSIZE == 64

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_uint64(const char * __restrict string, uint64_t * __restrict value)
{
	return ustr_parse_ulong(string, value);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_x64(const char * __restrict string, uint64_t * __restrict value)
{
	return ustr_parse_xlong(string, value);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_int64(const char * __restrict string, int64_t * __restrict value)
{
	return ustr_parse_long(string, value);
}

#elif __WORDSIZE == 32

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_uint64(const char * __restrict string, uint64_t * __restrict value)
{
	return ustr_parse_ullong(string, value);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_x64(const char * __restrict string, uint64_t * __restrict value)
{
	return ustr_parse_xllong(string, value);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_int64(const char * __restrict string, int64_t * __restrict value)
{
	return ustr_parse_llong(string, value);
}

#else
#error "Unsupported machine word size !"
#endif

extern int
ustr_parse_uint_range(const char * __restrict   string,
                      unsigned int * __restrict value,
                      unsigned int              min,
                      unsigned int              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_uint(const char * __restrict string, unsigned int * __restrict value)
{
	return ustr_parse_uint_range(string, value, 0U, UINT_MAX);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_uint32(const char * __restrict string, uint32_t * __restrict value)
{
	return ustr_parse_uint(string, value);
}

extern int
ustr_parse_xint_range(const char * __restrict   string,
                      unsigned int * __restrict value,
                      unsigned int              min,
                      unsigned int              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_xint(const char * __restrict string, unsigned int * __restrict value)
{
	return ustr_parse_xint_range(string, value, 0U, UINT_MAX);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_x32(const char * __restrict string, uint32_t * __restrict value)
{
	return ustr_parse_xint(string, value);
}

extern int
ustr_parse_int_range(const char * __restrict string,
                     int * __restrict        value,
                     int                     min,
                     int                     max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_int(const char * __restrict string, int * __restrict value)
{
	return ustr_parse_int_range(string, value, INT_MIN, INT_MAX);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_int32(const char * __restrict string, int32_t * __restrict value)
{
	return ustr_parse_int(string, value);
}

extern int
ustr_parse_ushrt_range(const char * __restrict     string,
                       unsigned short * __restrict value,
                       unsigned short              min,
                       unsigned short              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_ushrt(const char * __restrict     string,
                 unsigned short * __restrict value)
{
	return ustr_parse_ushrt_range(string, value, 0U, USHRT_MAX);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_uint16(const char * __restrict string, uint16_t * __restrict value)
{
	return ustr_parse_ushrt(string, value);
}

extern int
ustr_parse_xshrt_range(const char * __restrict     string,
                       unsigned short * __restrict value,
                       unsigned short              min,
                       unsigned short              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_xshrt(const char * __restrict     string,
                 unsigned short * __restrict value)
{
	return ustr_parse_xshrt_range(string, value, 0U, USHRT_MAX);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_x16(const char * __restrict string, uint16_t * __restrict value)
{
	return ustr_parse_xshrt(string, value);
}

extern int
ustr_parse_shrt_range(const char * __restrict string,
                      short * __restrict      value,
                      short                   min,
                      short                   max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_shrt(const char * __restrict string, short * __restrict value)
{
	return ustr_parse_shrt_range(string, value, SHRT_MIN, SHRT_MAX);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_int16(const char * __restrict string, int16_t * __restrict value)
{
	return ustr_parse_shrt(string, value);
}

extern int
ustr_parse_uchar_range(const char * __restrict    string,
                       unsigned char * __restrict value,
                       unsigned char              min,
                       unsigned char              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_uchar(const char * __restrict    string,
                 unsigned char * __restrict value)
{
	return ustr_parse_uchar_range(string, value, 0U, UCHAR_MAX);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_uint8(const char * __restrict string, uint8_t * __restrict value)
{
	return ustr_parse_uchar(string, value);
}

extern int
ustr_parse_xchar_range(const char * __restrict    string,
                       unsigned char * __restrict value,
                       unsigned char              min,
                       unsigned char              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_xchar(const char * __restrict    string,
                 unsigned char * __restrict value)
{
	return ustr_parse_xchar_range(string, value, 0U, UCHAR_MAX);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_x8(const char * __restrict string, uint8_t * __restrict value)
{
	return ustr_parse_xchar(string, value);
}

extern int
ustr_parse_char_range(const char * __restrict  string,
                      signed char * __restrict value,
                      signed char              min,
                      signed char              max)
	__utils_nonull(1, 2) __utils_nothrow __warn_result;

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_char(const char * __restrict string, signed char * __restrict value)
{
	return ustr_parse_char_range(string, value, SCHAR_MIN, SCHAR_MAX);
}

static inline __utils_nonull(1, 2) __utils_nothrow __warn_result
int
ustr_parse_int8(const char * __restrict string, int8_t * __restrict value)
{
	return ustr_parse_char(string, value);
}

extern size_t
ustr_skip_char(const char * __restrict string, int ch, size_t size)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

extern size_t
ustr_rskip_char(const char * __restrict string, int ch, size_t size)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

extern size_t
ustr_skip_notchar(const char * __restrict string, int ch, size_t size)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

extern size_t
ustr_rskip_notchar(const char * __restrict string, int ch, size_t size)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

extern size_t
ustr_skip_space(const char * __restrict string, size_t size)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

extern size_t
ustr_rskip_space(const char * __restrict string, size_t size)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

extern size_t
ustr_skip_notspace(const char * __restrict string, size_t size)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

extern size_t
ustr_rskip_notspace(const char * __restrict string, size_t size)
	__utils_nonull(1) __utils_pure __utils_nothrow __leaf __warn_result;

static inline __utils_nonull(1) __utils_pure __utils_nothrow
ssize_t
ustr_parse(const char * __restrict string, size_t max_size)
{
	ustr_assert_api(string);
	ustr_assert_api(max_size);

	size_t len;

	len = strnlen(string, max_size);

	return (len != max_size) ? (ssize_t)len : -ENAMETOOLONG;
}

extern char *
ustr_clone(const char * __restrict orig, size_t len)
	__utils_nonull(1) __utils_nothrow __leaf __warn_result;

extern char *
ustr_sized_clone(const char * __restrict orig, size_t max_size)
	__utils_nonull(1) __utils_nothrow __warn_result;

extern size_t
ustr_prefix_len(const char * __restrict string,
                size_t                  str_len,
                const char * __restrict prefix,
                size_t                  pref_len)
	__utils_nonull(1, 3) __utils_pure __utils_nothrow __leaf __warn_result;

#define ustr_const_prefix_len(_string, _len, _prefix) \
	ustr_prefix_len(_string, _len, _prefix, sizeof(_prefix) - 1)

extern size_t
ustr_suffix_len(const char * __restrict string,
                size_t                  str_len,
                const char * __restrict suffix,
                size_t                  suff_len)
	__utils_nonull(1, 3) __utils_pure __utils_nothrow __leaf __warn_result;

#define ustr_const_suffix_len(_string, _len, _suffix) \
	ustr_suffix_len(_string, _len, _suffix, sizeof(_suffix) - 1)

static inline __utils_nonull(1, 3) __utils_pure __utils_nothrow __warn_result
bool
ustr_match_token(const char * __restrict string,
                 size_t                  str_len,
                 const char * __restrict token,
                 size_t                  tok_len)
{
	ustr_assert_api(string);
	ustr_assert_api(token);
	ustr_assert_api(token[0]);
	ustr_assert_api(tok_len);

	if (str_len != tok_len)
		return false;

	return !memcmp(string, token, tok_len);
}

#define ustr_match_const_token(_string, _str_len, _token) \
	ustr_match_token(_string, _str_len, _token, sizeof(_token) - 1)

/**
 * Token parsing callback.
 *
 * While parsing, callback implementation MUST look only at the first @p size
 * characters in the string pointed to by @p string.
 *
 * If needed, the callback shall return -ENOENT error code to tell the caller
 * that no token were matched. In addition, it is NOT ALLOWED to use any of the
 * following value to indicate the caller an error:
 * * -ENODATA
 * * -EMSGSIZE
 *
 * @param[inout] string  string to parse
 * @param[in]    size    maximum size of @p string
 * @param[inout] context callback specific parsing context
 *
 * @return Parsing result
 * @retval 0       token matched
 * @retval -ENOENT no token matched
 * @retval <0      callback specific negative errno like error code
 */
typedef int ustr_parse_token_fn(char * __restrict string,
                                size_t            size,
                                void * __restrict context) __utils_nonull(1);

/**
 * Parse a string containing a sequence of token fields located at fixed
 * positions.
 *
 * @p string is a null byte terminated string and must contain a sequence of
 * tokens separated by @p delim character called fields. For each field found,
 * this function runs the corresponding parser callback found into @p parsers
 * array.
 *
 * To lookup the parser callback corresponding to a given field, this function
 * assigns field an index starting from zero and incremented each time a @p
 * delim character is crossed. The corresponding parser callback is located into
 * @p parsers using this field index.
 *
 * When called, parsing callbacks are given @p context as last argument. @p
 * context shall point to arbitrary memory location owned by the caller. It may
 * be given as NULL.
 *
 * As soon as a parser callback returns a negative error code, this function
 * stops processing @p string and returns the error code to the caller.
 * Strings containing empty fields, i.e., a substring of 2 successive @p delim
 * characters, are not allowed and causes this function to return an error.
 * Once all expected fields have been parsed, this functions returns an error if
 * there are still unparsed trailing characters found at the end of @p string.
 *
 * @param[in]    string  string to parse
 * @param[in]    delim   token sequence delimiter character
 * @param[in]    parsers array of parsing callbacks
 * @param[in]    count   number of entries contained into @p parsers array /
 *                       number of expected token fields
 * @param[inout] context callback specific parsing context
 *
 * @return Parsing result
 * @retval >=0       count of matched tokens / callbacks run
 * @retval -ENODATA  empty / missing token found
 * @retval -ENOENT   unexpected / unmatched token
 * @retval -EMSGSIZE all expected fields matched, but @p string contains
 *                   trailing garbage character(s)
 * @retval <0        parsing callback negative errno like error code
 */
extern int
ustr_parse_token_fields(char * __restrict           string,
                        int                         delim,
                        ustr_parse_token_fn * const parsers[__restrict_arr],
                        unsigned int                count,
                        void * __restrict           context)
	__utils_nonull(1, 3) __warn_result;

/**
 * Parse a string containing a sequence of token fields
 *
 * @p string is a null byte terminated string and must contain a sequence of
 * tokens separated by @p delim character called fields. For each field found,
 * this function runs the @p parse parsing callback given in argument.
 *
 * When called, parsing callback is given @p context as last argument. @p
 * context shall point to arbitrary memory location owned by the caller. It may
 * be given as NULL.
 *
 * As soon as @p parse callback returns a negative error code, this function
 * stops processing @p string and returns the error code to the caller.
 * Strings containing empty fields, i.e., a substring of 2 successive @p delim
 * characters, are not allowed and causes this function to return an error.
 *
 * @param[in]    string  string to parse
 * @param[in]    delim   token sequence delimiter character
 * @param[in]    parse   parsing callback
 * @param[inout] context callback specific parsing context
 *
 * @return Parsing result
 * @retval >=0       count of matched tokens
 * @retval -ENODATA  empty / missing token found
 * @retval -ENOENT   unexpected / unmatched token
 * @retval <0        parsing callback negative errno like error code
 */
extern int
ustr_parse_each_token(char * __restrict     string,
                      int                   delim,
                      ustr_parse_token_fn * parse,
                      void * __restrict     context)
	__utils_nonull(1, 3) __warn_result;

#endif /* _UTILS_STRING_H */
