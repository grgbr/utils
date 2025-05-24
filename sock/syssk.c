/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#include "syssk.h"
#include "utils/string.h"
#include <sysexits.h>
#include <regex.h>

static
void
etux_syssk_compile_regex(regex_t * __restrict    regex,
                         const char * __restrict expr)
{
	etux_syssk_assert_intern(regex);
	etux_syssk_assert_intern(expr);
	etux_syssk_assert_intern(expr[0] != '\0');

	int err;

	err = regcomp(regex, expr, REG_EXTENDED | REG_NOSUB);
	if (err) {
		etux_syssk_assert_intern(err == REG_ESPACE);
		exit(EX_OSERR);
	}
}

static
ssize_t
etux_syssk_match_pattern(const char * __restrict    string,
                         size_t                     maxsz,
                         const regex_t * __restrict regex)
{
	etux_syssk_assert_intern(string);
	etux_syssk_assert_intern(maxsz);
	etux_syssk_assert_intern(regex);

	size_t len;
	int    ret;

	len = strnlen(string, maxsz);
	if (!len)
		return -ENODATA;
	else if (len >= maxsz)
		return -ENAMETOOLONG;

	ret = regexec(regex, string, 0, NULL, 0);
	switch (ret) {
	case 0:
		break;
	case REG_NOMATCH:
		return -EINVAL;
	case REG_ESPACE:
		return -ENOMEM;
	default:
		etux_syssk_assert_intern(0);
	}

	return (ssize_t)len;
}

#define ETUX_SYSSK_HOST_LABEL_MAX \
	MAXHOSTNAMELEN

#define ETUX_SYSSK_HOST_NAME_MAX \
	(256U)

/*
 * POSIX regular expression allowing to validate a DNS hostname label.
 *
 * RFC 1123 states that a valid label must satisfy the following conditions:
 * - it is composed of a-z, A-Z, 0–9 and hyphen `-' characters ;
 * - it is between 1 and 63 characters long ;
 * - it cannot start or end with a hyphen `-'.
 */
#define _ETUX_SYSSK_HOST_LABEL_REGEX \
	"[a-zA-Z0-9]([a-zA-Z0-9-]{,61}[a-zA-Z0-9])?"

#define ETUX_SYSSK_HOST_LABEL_REGEX \
	"^" _ETUX_SYSSK_HOST_LABEL_REGEX "$"

/*
 * POSIX regular expression allowing to validate a DNS TLD (Top-level Domain)
 * label..
 *
 * Must be at least 2 and up to 6 characters long. May be followed by an
 * optional trailing dot `.' that denotes DNS root.
 */
#define _ETUX_SYSSK_TLD_REGEX \
	"[A-Za-z]{2,6}\\.?"

/*
 * POSIX regular expression allowing to validate a DNS (sub)domain.
 *
 * A dot `.' separated succession of optional DNS labels ending with a DNS TLD
 * label.
 * A simple `.' the root zone is also accepted.
 */
#define _ETUX_SYSSK_DOMAIN_REGEX \
	"(" _ETUX_SYSSK_HOST_LABEL_REGEX "\\.)*" _ETUX_SYSSK_TLD_REGEX

#define ETUX_SYSSK_DOMAIN_REGEX \
	"^" _ETUX_SYSSK_DOMAIN_REGEX "$|^\\.$"

/*
 * POSIX regular expression allowing to validate a DNS Fully Qualified Domain
 * Name.
 *
 * A DNS hostname label followed by a dot `.' and a domain.
 */
#define ETUX_SYSSK_FQDN_REGEX \
	"^" _ETUX_SYSSK_HOST_LABEL_REGEX "\\." _ETUX_SYSSK_DOMAIN_REGEX "$"

/*
 * POSIX regular expression allowing to validate a DNS hostname.
 * Name.
 *
 * Either a DNS hostname label, a DNS domain or the top-level DNS root (`.')
 */
#define ETUX_SYSSK_HOST_NAME_REGEX \
	ETUX_SYSSK_HOST_LABEL_REGEX "|" ETUX_SYSSK_DOMAIN_REGEX

static regex_t etux_syssk_host_name_regex;

static __ctor()
void
etux_syssk_regex_ctor(void)
{
	etux_syssk_compile_regex(&etux_syssk_host_name_regex,
	                         ETUX_SYSSK_HOST_NAME_REGEX);
}

#if defined(CONFIG_UTILS_VALGRIND)

static __dtor()
void
etux_syssk_regex_dtor(void)
{
	regfree(&etux_syssk_host_name_regex);
}

#endif /* defined(CONFIG_UTILS_VALGRIND) */

ssize_t
etux_syssk_validate_host_name(const char * __restrict string)
{
	etux_syssk_assert_api(string);

	return etux_syssk_match_pattern(string,
	                                ETUX_SYSSK_HOST_NAME_MAX,
	                                &etux_syssk_host_name_regex);
}

int
etux_syssk_parse_port(in_port_t * __restrict  port,
                      const char * __restrict string)
{
	etux_syssk_assert_api(port);
	etux_syssk_assert_api(string);

	in_port_t prt;
	int       err;

	err = ustr_parse_ushrt(string, &prt);
	if (!err) {
		*port = htons(prt);
		return 0;
	}

	return err;
}

int
etux_syssk_resolv_serv(in_port_t * __restrict  port,
                       const char * __restrict string)
{
	etux_syssk_assert_api(port);
	etux_syssk_assert_api(string);

	if (etux_syssk_parse_port(port, string)) {
		const struct servent * ent;

		ent = getservbyname(string, NULL);
		if (!ent)
			return -ENOENT;

		if (ent->s_port > USHRT_MAX)
			return -ERANGE;

		*port = htons((const in_port_t)ent->s_port);
	}

	return 0;
}
