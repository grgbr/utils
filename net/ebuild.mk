################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

include ../common.mk

common-cflags            += -fvisibility=hidden
shared-common-cflags     += -fvisibility=hidden


builtins                 := shared/builtin.a
shared/builtin.a-objs    += $(call kconf_enabled, ETUX_NETDB, shared/netdb.o)
shared/builtin.a-objs    += $(call kconf_enabled, ETUX_NETIF, shared/netif.o)
shared/builtin.a-objs    += $(call kconf_enabled, UTILS_UNSK, shared/unsk.o)
shared/builtin.a-objs    += $(call kconf_enabled, ETUX_IN4SK, shared/in4sk.o)
shared/builtin.a-objs    += $(call kconf_enabled, ETUX_IN6SK, shared/in6sk.o)
shared/builtin.a-cflags  := $(shared-common-cflags)
shared/builtin.a-pkgconf := $(common-pkgconf)


builtins                 += static/builtin.a
static/builtin.a-objs    += $(call kconf_enabled, ETUX_NETDB, static/netdb.o)
static/builtin.a-objs    += $(call kconf_enabled, ETUX_NETIF, static/netif.o)
static/builtin.a-objs    += $(call kconf_enabled, UTILS_UNSK, static/unsk.o)
static/builtin.a-objs    += $(call kconf_enabled, ETUX_IN4SK, static/in4sk.o)
static/builtin.a-objs    += $(call kconf_enabled, ETUX_IN6SK, static/in6sk.o)
static/builtin.a-cflags  := $(common-cflags)
static/builtin.a-pkgconf := $(common-pkgconf)

# ex: filetype=make :
