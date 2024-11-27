################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

common-cflags        := -Wall \
                        -Wextra \
                        -Wformat=2 \
                        -Wconversion \
                        -Wundef \
                        -Wshadow \
                        -Wcast-qual \
                        -Wcast-align \
                        -Wmissing-declarations \
                        -fvisibility=hidden \
                        -D_GNU_SOURCE \
                        -I ../include \
                        $(EXTRA_CFLAGS)

ifeq ($(CONFIG_UTILS_UTEST)$(CONFIG_UTILS_ASSERT_API),yy)
# When unit testsuite is required to be built, make sure to enable ELF semantic
# interposition.
# This allows unit test programs to override the stroll_assert_fail() using
# their own definitions based on CUTe's expectations to validate assertions.
#
# See http://maskray.me/blog/2021-05-09-fno-semantic-interposition for more
# informations about semantic interposition.
common-cflags        := $(common-cflags) -fsemantic-interposition
endif # ($(CONFIG_UTILS_UTEST)$(CONFIG_UTILS_ASSERT_API),yy)

common-ldflags       := $(common-cflags) $(EXTRA_LDFLAGS) \
                        -Wl,-z,start-stop-visibility=hidden

ifneq ($(filter y,$(CONFIG_UTILS_ASSERT_API) $(CONFIG_UTILS_ASSERT_INTERN)),)
common-cflags        := $(filter-out -DNDEBUG,$(common-cflags))
common-ldflags       := $(filter-out -DNDEBUG,$(common-ldflags))
endif # ($(filter y,$(CONFIG_UTILS_ASSERT_API) $(CONFIG_UTILS_ASSERT_INTERN)),)

shared-common-cflags  := $(filter-out -fpie -fPIE,$(common-cflags)) -fpic
shared-common-ldflags := $(filter-out -pie -fpie -fPIE,$(common-ldflags)) \
                         -shared -Bsymbolic -fpic \
                         -lutils
common-pkgconf        := libstroll $(call kconf_enabled,ETUX_TRACE,lttng-ust)

builtins                 := shared/builtin.a
shared/builtin.a-objs    := $(call kconf_enabled,ETUX_TRACE,shared/trace.o)
shared/builtin.a-cflags  := $(shared-common-cflags)
shared/builtin.a-pkgconf := $(common-pkgconf)

builtins                 += static/builtin.a
static/builtin.a-objs    := $(call kconf_enabled,ETUX_TRACE,static/trace.o)
static/builtin.a-cflags  := $(common-cflags)
static/builtin.a-pkgconf := $(common-pkgconf)

solibs                          := $(call kconf_enabled,ETUX_TIMER_LIST,libetux_timer_list.so)
libetux_timer_list.so-objs      := shared/list.o
libetux_timer_list.so-cflags    := $(shared-common-cflags)
libetux_timer_list.so-ldflags   := $(shared-common-ldflags) \
                                   -l:shared/builtin.a \
                                   -Wl,-soname,libetux_timer_list.so
libetux_timer_list.so-pkgconf    = $(common-pkgconf)

arlibs                          := $(call kconf_enabled,ETUX_TIMER_LIST,libetux_timer_list.a)
libetux_timer_list.a-objs       := static/list.o static/trace.o
libetux_timer_list.a-cflags     := $(common-cflags)
libetux_timer_list.a-pkgconf     = $(common-pkgconf)

solibs                          += $(call kconf_enabled,ETUX_TIMER_HWHEEL,libetux_timer_hwheel.so)
libetux_timer_hwheel.so-objs    := shared/hwheel.o
libetux_timer_hwheel.so-cflags  := $(shared-common-cflags)
libetux_timer_hwheel.so-ldflags := $(shared-common-ldflags) \
                                   -l:shared/builtin.a \
                                   -Wl,-soname,libetux_timer_hwheel.so
libetux_timer_hwheel.so-pkgconf  = $(common-pkgconf)

arlibs                          += $(call kconf_enabled,ETUX_TIMER_HWHEEL,libetux_timer_hwheel.a)
libetux_timer_hwheel.a-objs     := static/hwheel.o static/trace.o
libetux_timer_hwheel.a-cflags   := $(common-cflags)
libetux_timer_hwheel.a-pkgconf   = $(common-pkgconf)

# ex: filetype=make :
