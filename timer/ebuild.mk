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
                        $(EXTRA_CFLAGS) \
                        $(call kconf_enabled,UTILS_THREAD,-pthread)

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

solibs                        := $(call kconf_enabled,ETUX_TIMER_LIST,libetux_timer_list.so)
libetux_timer_list.so-objs    := list.o
libetux_timer_list.so-cflags  := $(filter-out -fpie -fPIE,$(common-cflags)) -fpic
libetux_timer_list.so-ldflags := $(filter-out -pie -fpie -fPIE,$(common-ldflags)) \
                                 -shared -Bsymbolic -fpic \
                                 -Wl,-soname,libetux_timer_list.so \
                                 -lutils

arlibs                        := $(call kconf_enabled,ETUX_TIMER_LIST,libetux_timer_list.a)
libetux_timer_list.a-objs     := list.o
libutils.a-cflags             := $(common-cflags)

# ex: filetype=make :
