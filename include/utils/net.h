#ifndef _UTILS_NET_H
#define _UTILS_NET_H

#include <utils/string.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <stdbool.h>

#if defined(CONFIG_UTILS_ASSERT_INTERNAL)

#include <utils/assert.h>

#define unet_assert(_expr) \
	uassert("unet", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERNAL) */

#define unet_assert(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERNAL) */

static inline ssize_t
unet_check_iface_name(const char *name)
{
	unet_assert(name);

	return (*name) ? ustr_parse(name, IFNAMSIZ) : -ENOENT;
}

static inline bool
unet_mtu_isok(uint32_t mtu)
{
	return mtu && (mtu <= ETH_MAX_MTU);
}

/**
 * unet_hwaddr_is_laa() - Check wether a EUI-48 MAC address is locally
 *                       administered or not.
 */
static inline bool
unet_hwaddr_is_laa(const struct ether_addr *addr)
{
	unet_assert(addr);

	return !!(addr->ether_addr_octet[0] & 0x2);
}

/**
 * unet_hwaddr_is_uaa() - Check wether a EUI-48 MAC address is universally
 *                       administered or not.
 */
static inline bool
unet_hwaddr_is_uaa(const struct ether_addr *addr)
{
	return !unet_hwaddr_is_laa(addr);
}

/**
 * unet_hwaddr_is_mcast() - Check wether a EUI-48 MAC address is multicast or
 *                         not.
 */
static inline bool
unet_hwaddr_is_mcast(const struct ether_addr *addr)
{
	unet_assert(addr);

	return !!(addr->ether_addr_octet[0] & 0x1);
}

/**
 * unet_hwaddr_is_ucast() - Check wether a EUI-48 MAC address isunicast or not.
 */
static inline bool
unet_hwaddr_is_ucast(const struct ether_addr *addr)
{
	return !unet_hwaddr_is_mcast(addr);
}

#endif /* _UTILS_NET_H */
