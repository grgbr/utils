/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

/**
 * @file
 * System network object interface
 *
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      08 Sep 2020
 * @copyright Copyright (C) 2017-2024 Grégor Boirie.
 * @license   [GNU Lesser General Public License (LGPL) v3]
 *            (https://www.gnu.org/licenses/lgpl+gpl-3.0.txt)
 */

#ifndef _UTILS_NET_H
#define _UTILS_NET_H

#include <utils/string.h>
#include <stdbool.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/if.h>

#if defined(CONFIG_UTILS_ASSERT_API)

#include <stroll/assert.h>

#define unet_assert_api(_expr) \
	stroll_assert("utils:unet", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_API) */

#define unet_assert_api(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_API) */

#define UNET_IFACE_CLASS_PREFIX   "/sys/class/net"
#define UNET_IFACE_SYSPATH_PREFIX "/sys/devices"
#define UNET_IFACE_SYSPATH_MAX    (64U)

static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
ssize_t
unet_check_iface_syspath(const char * __restrict syspath)
{
	unet_assert_api(syspath);

	return (*syspath) ? ustr_parse(syspath, UNET_IFACE_SYSPATH_MAX) :
	                    -ENOENT;
}

extern ssize_t
unet_normalize_iface_syspath(const char * __restrict orig,
                             char ** __restrict      norm)
	__utils_nonull(1, 2) __utils_nothrow __leaf __warn_result;

extern ssize_t
unet_resolve_iface_syspath(const char * __restrict orig,
                           char ** __restrict      real)
	__utils_nonull(1, 2) __warn_result __leaf __warn_result;


static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
ssize_t
unet_check_iface_name(const char * __restrict name)
{
	unet_assert_api(name);

	return (*name) ? ustr_parse(name, IFNAMSIZ) : -ENOENT;
}

static inline __const __nothrow __warn_result
bool
unet_iface_mtu_isok(uint32_t mtu)
{
	/*
	 * We might be using IP_MAXPACKET or ETH_MAX_MTU, i.e. 65535.
	 * However loopback interface MTU is 65536 bytes long...
	 */ 
	return mtu && (mtu <= UINT32_C(65536));
}

static inline __const __nothrow __warn_result
bool
unet_iface_admin_state_isok(uint8_t state)
{
	switch (state) {
	case IF_OPER_UP:
	case IF_OPER_DOWN:
		return true;

	default:
		return false;
	}
}

static inline __const __nothrow __warn_result
bool
unet_iface_oper_state_isok(uint8_t state)
{
	switch (state) {
	case IF_OPER_UNKNOWN:
	case IF_OPER_DOWN:
	case IF_OPER_LOWERLAYERDOWN:
	case IF_OPER_DORMANT:
	case IF_OPER_UP:
		return true;

	default:
		return false;
	}
}

static inline __const __nothrow __warn_result
bool
unet_iface_carrier_state_isok(uint8_t state)
{
	switch (state) {
	case IF_OPER_UNKNOWN:
	case IF_OPER_NOTPRESENT:
	case IF_OPER_DOWN:
	case IF_OPER_LOWERLAYERDOWN:
	case IF_OPER_DORMANT:
	case IF_OPER_UP:
		return true;

	default:
		return false;
	}
}

#define UNET_HWADDR_STRING_MAX \
	(sizeof_member(struct ether_addr, ether_addr_octet) *  3)

/**
 * unet_hwaddr_is_laa() - Check wether a EUI-48 MAC address is locally
 *                       administered or not.
 */
static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
unet_hwaddr_is_laa(const struct ether_addr * __restrict addr)
{
	unet_assert_api(addr);

	return !!(addr->ether_addr_octet[0] & 0x2);
}

/**
 * unet_hwaddr_is_uaa() - Check wether a EUI-48 MAC address is universally
 *                       administered or not.
 */
static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
unet_hwaddr_is_uaa(const struct ether_addr * __restrict addr)
{
	return !unet_hwaddr_is_laa(addr);
}

/**
 * unet_hwaddr_is_mcast() - Check wether a EUI-48 MAC address is multicast or
 *                          not.
 */
static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
unet_hwaddr_is_mcast(const struct ether_addr * __restrict addr)
{
	unet_assert_api(addr);

	return !!(addr->ether_addr_octet[0] & 0x1);
}

/**
 * unet_hwaddr_is_ucast() - Check wether a EUI-48 MAC address isunicast or not.
 */
static inline __utils_nonull(1) __utils_pure __utils_nothrow __warn_result
bool
unet_hwaddr_is_ucast(const struct ether_addr * __restrict addr)
{
	return !unet_hwaddr_is_mcast(addr);
}

#endif /* _UTILS_NET_H */
