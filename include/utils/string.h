#ifndef _UTILS_STRING_H
#define _UTILS_STRING_H

#include <utils/cdefs.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define ustr_assert(_expr) \
	uassert("ustr", _expr)

#define __ustr_nonull(_arg_index, ...)

#else /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define ustr_assert(_expr)

#define __ustr_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

extern int
ustr_parse_bool(const char *string, bool *value);

extern int
ustr_parse_ullong(const char *string, unsigned long long *value);

extern int
ustr_parse_ullong_range(const char         *string,
                        unsigned long long *value,
                        unsigned long long  min,
                        unsigned long long  max);

extern int
ustr_parse_xllong(const char *string, unsigned long long *value);

extern int
ustr_parse_xllong_range(const char         *string,
                        unsigned long long *value,
                        unsigned long long  min,
                        unsigned long long  max);

extern int
ustr_parse_llong(const char *string, long long *value);

extern int
ustr_parse_llong_range(const char *string,
                       long long  *value,
                       long long   min,
                       long long   max);

extern int
ustr_parse_ulong(const char *string, unsigned long *value);

extern int
ustr_parse_ulong_range(const char    *string,
                       unsigned long *value,
                       unsigned long  min,
                       unsigned long  max);

extern int
ustr_parse_xlong(const char *string, unsigned long *value);

extern int
ustr_parse_xlong_range(const char    *string,
                       unsigned long *value,
                       unsigned long  min,
                       unsigned long  max);

extern int
ustr_parse_long(const char *string, long *value);

extern int
ustr_parse_long_range(const char *string, long *value, long min, long max);

#if __WORDSIZE == 64

static inline int
ustr_parse_uint64(const char *string, uint64_t *value)
{
	return ustr_parse_ulong(string, value);
}

static inline int
ustr_parse_x64(const char *string, uint64_t *value)
{
	return ustr_parse_xlong(string, value);
}

static inline int
ustr_parse_int64(const char *string, int64_t *value)
{
	return ustr_parse_long(string, value);
}

#elif __WORDSIZE == 32

static inline int
ustr_parse_uint64(const char *string, uint64_t *value)
{
	return ustr_parse_ullong(string, value);
}

static inline int
ustr_parse_x64(const char *string, uint64_t *value)
{
	return ustr_parse_xllong(string, value);
}

static inline int
ustr_parse_int64(const char *string, int64_t *value)
{
	return ustr_parse_llong(string, value);
}

#else
#error "Unsupported machine word size !"
#endif

extern int
ustr_parse_uint_range(const char   *string,
                      unsigned int *value,
                      unsigned int  min,
                      unsigned int  max);

static inline int
ustr_parse_uint(const char *string, unsigned int *value)
{
	return ustr_parse_uint_range(string, value, 0U, UINT_MAX);
}

static inline int
ustr_parse_uint32(const char *string, uint32_t *value)
{
	return ustr_parse_uint(string, value);
}

extern int
ustr_parse_xint_range(const char   *string,
                      unsigned int *value,
                      unsigned int  min,
                      unsigned int  max);

static inline int
ustr_parse_xint(const char *string, unsigned int *value)
{
	return ustr_parse_xint_range(string, value, 0U, UINT_MAX);
}

static inline int
ustr_parse_x32(const char *string, uint32_t *value)
{
	return ustr_parse_xint(string, value);
}

extern int
ustr_parse_int_range(const char *string, int *value, int min, int max);

static inline int
ustr_parse_int(const char *string, int *value)
{
	return ustr_parse_int_range(string, value, INT_MIN, INT_MAX);
}

static inline int
ustr_parse_int32(const char *string, int32_t *value)
{
	return ustr_parse_int(string, value);
}

extern int
ustr_parse_ushrt_range(const char     *string,
                       unsigned short *value,
                       unsigned short  min,
                       unsigned short  max);

static inline int
ustr_parse_ushrt(const char *string, unsigned short *value)
{
	return ustr_parse_ushrt_range(string, value, 0U, USHRT_MAX);
}

static inline int
ustr_parse_uint16(const char *string, uint16_t *value)
{
	return ustr_parse_ushrt(string, value);
}

extern int
ustr_parse_xshrt_range(const char     *string,
                       unsigned short *value,
                       unsigned short  min,
                       unsigned short  max);

static inline int
ustr_parse_xshrt(const char *string, unsigned short *value)
{
	return ustr_parse_xshrt_range(string, value, 0U, USHRT_MAX);
}

static inline int
ustr_parse_x16(const char *string, uint16_t *value)
{
	return ustr_parse_xshrt(string, value);
}

extern int
ustr_parse_shrt_range(const char *string, short *value, short min, short max);

static inline int
ustr_parse_shrt(const char *string, short *value)
{
	return ustr_parse_shrt_range(string, value, SHRT_MIN, SHRT_MAX);
}

static inline int
ustr_parse_int16(const char *string, int16_t *value)
{
	return ustr_parse_shrt(string, value);
}

extern int
ustr_parse_uchar_range(const char    *string,
                       unsigned char *value,
                       unsigned char  min,
                       unsigned char  max);

static inline int
ustr_parse_uchar(const char *string, unsigned char *value)
{
	return ustr_parse_uchar_range(string, value, 0U, UCHAR_MAX);
}

static inline int
ustr_parse_uint8(const char *string, uint8_t *value)
{
	return ustr_parse_uchar(string, value);
}

extern int
ustr_parse_xchar_range(const char    *string,
                       unsigned char *value,
                       unsigned char  min,
                       unsigned char  max);

static inline int
ustr_parse_xchar(const char *string, unsigned char *value)
{
	return ustr_parse_xchar_range(string, value, 0U, UCHAR_MAX);
}

static inline int
ustr_parse_x8(const char *string, uint8_t *value)
{
	return ustr_parse_xchar(string, value);
}

extern int
ustr_parse_char_range(const char  *string,
                      signed char *value,
                      signed char  min,
                      signed char  max);

static inline int
ustr_parse_char(const char *string, signed char *value)
{
	return ustr_parse_char_range(string, value, SCHAR_MIN, SCHAR_MAX);
}

static inline int
ustr_parse_int8(const char *string, int8_t *value)
{
	return ustr_parse_char(string, value);
}

extern void
ustr_tolower(char *lower, const char *orig, size_t size);

extern void
ustr_tolower_inp(char *string, size_t size);

extern void
ustr_toupper(char *upper, const char *orig, size_t size);

extern void
ustr_toupper_inp(char *string, size_t size);

static inline size_t
ustr_skip_char(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string;

	while ((str < (string + size)) && (*str == ch))
		str++;

	return str - string;
}

static inline size_t
ustr_rskip_char(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string + size - 1;

	while ((str >= string) && (*str == ch))
		str--;

	return size - (size_t)((str + 1) - string);
}

static inline size_t __ustr_nonull(1) __pure __nothrow
ustr_skip_notchar(const char * string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string;

	while ((str < (string + size)) && *str && (*str != ch))
		str++;

	return str - string;
}

static inline size_t
ustr_rskip_notchar(const char *string, int ch, size_t size)
{
	ustr_assert(string);
	ustr_assert(ch);
	ustr_assert(size);

	const char *str = string + size - 1;

	if (!*str)
		return 0;

	while ((str >= string) && (*str != ch))
		str--;

	return size - (size_t)((str + 1) - string);
}

extern size_t
ustr_skip_space(const char *string, size_t size);

extern size_t
ustr_rskip_space(const char *string, size_t size);

extern size_t
ustr_skip_notspace(const char *string, size_t size);

extern size_t
ustr_rskip_notspace(const char *string, size_t size);

static inline ssize_t
ustr_parse(const char *str, size_t max_size)
{
	ustr_assert(str);
	ustr_assert(max_size);

	size_t len;

	len = strnlen(str, max_size);

	return (len != max_size) ? (ssize_t)len : -ENAMETOOLONG;
}

extern char *
ustr_clone(const char *orig, size_t len);

extern char *
ustr_sized_clone(const char *orig, size_t max_size);

extern size_t
ustr_prefix_len(const char * __restrict string,
                size_t                  str_len,
                const char * __restrict prefix,
                size_t                  pref_len)
	__ustr_nonull(1, 3) __pure __nothrow __leaf;

#define ustr_const_prefix_len(_string, _len, _prefix) \
	ustr_prefix_len(_string, _len, _prefix, sizeof(_prefix) - 1)

extern size_t
ustr_suffix_len(const char * __restrict string,
                size_t                  str_len,
                const char * __restrict suffix,
                size_t                  suff_len)
	__ustr_nonull(1, 3) __pure __nothrow __leaf;

#define ustr_const_suffix_len(_string, _len, _suffix) \
	ustr_suffix_len(_string, _len, _suffix, sizeof(_suffix) - 1)

static inline bool __ustr_nonull(1, 3) __pure __nothrow
ustr_match_token(const char * __restrict string,
                 size_t                  str_len,
                 const char * __restrict token,
                 size_t                  tok_len)
{
	ustr_assert(string);
	ustr_assert(token);
	ustr_assert(token[0]);
	ustr_assert(tok_len);

	if (str_len != tok_len)
		return false;

	return !memcmp(string, token, tok_len);
}

#define ustr_match_const_token(_string, _str_len, _token) \
	ustr_match_token(_string, _str_len, _token, sizeof(_token) - 1)

/*
 * Return:
 * * 1  -- token matched
 * * 0  -- no match
 * * <0 -- parsing error.
 */
typedef int (ustr_parse_token_fn)(const char * __restrict string,
                                  size_t                  length,
                                  void * __restrict       context);

/*
 * Parse a string containing a chain of tokens located at specified positions.
 *
 * Return:
 * >=0       -- count of token matched
 * -ENODATA  -- empty / missing token
 * -EBADMSG  -- unexpected token
 * -EMSGSIZE -- all expected tokens matched, but string has not been parsed
 *              entirely, i.e. characters in excess
 */
extern int
ustr_parse_token_chain(ustr_parse_token_fn * const parsers[__restrict_arr],
                       unsigned int                count,
                       int                         delim,
                       const char * __restrict     string,
                       size_t                      size,
                       void * __restrict           context)
	__ustr_nonull(1, 4) __nothrow;

/*
 * Parse a string containing an unordered list of token.
 *
 * Return:
 * >=0       -- count of token matched
 * -ENODATA  -- empty / missing token
 * -EBADMSG  -- unexpected token
 */
extern int
ustr_foreach_token(ustr_parse_token_fn *   parse,
                   int                     delim,
                   const char * __restrict string,
                   size_t                  size,
                   void * __restrict       context)
	__ustr_nonull(1, 3) __nothrow;

#endif /* _UTILS_STRING_H */
