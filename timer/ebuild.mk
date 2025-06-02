################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

include ../common.mk

common-cflags                   += -fvisibility=hidden
shared-common-cflags            += -fvisibility=hidden
common-ldflags                  += -fvisibility=hidden
shared-common-ldflags           := -L$(BUILDDIR)/../src \
                                   $(shared-common-ldflags) \
                                   -fvisibility=hidden \
                                   -l:shared/builtin.a -lutils
common-pkgconf                  += $(call kconf_enabled,ETUX_TRACE,lttng-ust)

builtins                        := shared/builtin.a
shared/builtin.a-objs           := shared/common.o
shared/builtin.a-objs           += $(call kconf_enabled, \
                                          ETUX_TRACE, \
                                          shared/trace.o)
shared/builtin.a-cflags         := $(shared-common-cflags)
shared/builtin.a-pkgconf        := $(common-pkgconf)

builtins                        += static/builtin.a
static/builtin.a-objs           := static/common.o
static/builtin.a-objs           += $(call kconf_enabled, \
                                          ETUX_TRACE, \
                                          static/trace.o)
static/builtin.a-cflags         := $(common-cflags)
static/builtin.a-pkgconf        := $(common-pkgconf)

# Doubly linked list based implementation.
solibs                          := $(call kconf_enabled, \
                                          ETUX_TIMER_LIST, \
                                          libetux_timer_list.so)
libetux_timer_list.so-objs      := shared/list.o
libetux_timer_list.so-cflags    := $(shared-common-cflags)
libetux_timer_list.so-ldflags   := $(shared-common-ldflags) \
                                   -Wl,-soname,libetux_timer_list.so
libetux_timer_list.so-pkgconf   := $(common-pkgconf)

arlibs                          := $(call kconf_enabled, \
                                          ETUX_TIMER_LIST, \
                                          libetux_timer_list.a)
libetux_timer_list.a-objs       := static/list.o
libetux_timer_list.a-lots       := static/builtin.a
libetux_timer_list.a-cflags     := $(common-cflags)
libetux_timer_list.a-pkgconf    := $(common-pkgconf)

# Heap based implementation.
solibs                          += $(call kconf_enabled, \
                                          ETUX_TIMER_HEAP, \
                                          libetux_timer_heap.so)
libetux_timer_heap.so-objs      := shared/heap.o
libetux_timer_heap.so-cflags    := $(shared-common-cflags)
libetux_timer_heap.so-ldflags   := $(shared-common-ldflags) \
                                   -Wl,-soname,libetux_timer_heap.so
libetux_timer_heap.so-pkgconf   := $(common-pkgconf)

arlibs                          += $(call kconf_enabled, \
                                          ETUX_TIMER_HEAP, \
                                          libetux_timer_heap.a)
libetux_timer_heap.a-objs       := static/heap.o
libetux_timer_heap.a-lots       := static/builtin.a
libetux_timer_heap.a-cflags     := $(common-cflags)
libetux_timer_heap.a-pkgconf    := $(common-pkgconf)

# Hierarchical timing wheel based implementation.

solibs                          += $(call kconf_enabled, \
                                          ETUX_TIMER_HWHEEL, \
                                          libetux_timer_hwheel.so)
libetux_timer_hwheel.so-objs    := shared/hwheel.o
libetux_timer_hwheel.so-cflags  := $(shared-common-cflags)
libetux_timer_hwheel.so-ldflags := $(shared-common-ldflags) \
                                   -Wl,-soname,libetux_timer_hwheel.so
libetux_timer_hwheel.so-pkgconf := $(common-pkgconf)

arlibs                          += $(call kconf_enabled, \
                                          ETUX_TIMER_HWHEEL, \
                                          libetux_timer_hwheel.a)
libetux_timer_hwheel.a-objs     := static/hwheel.o
libetux_timer_hwheel.a-lots     := static/builtin.a
libetux_timer_hwheel.a-cflags   := $(common-cflags)
libetux_timer_hwheel.a-pkgconf  := $(common-pkgconf)

# ex: filetype=make :
