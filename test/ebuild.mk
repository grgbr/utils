################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2017-2023 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

test-cflags := -Wall \
               -Wextra \
               -Wformat=2 \
               -Wconversion \
               -Wundef \
               -Wshadow \
               -Wcast-qual \
               -Wcast-align \
               -Wmissing-declarations \
               -D_GNU_SOURCE \
               -DUTILS_VERSION_STRING="\"$(VERSION)\"" \
               -I../include \
               $(EXTRA_CFLAGS)

# Use -whole-archive to enforce the linker to scan builtin.a static library
# entirely so that symbols in utest.o may override existing strong symbols
# defined into other compilation units.
# This is required since we want stroll_assert_fail() defined into utest.c to
# override stroll_assert_fail() defined into libstroll.so for assertions testing
# purposes.
utest-ldflags := \
	$(test-cflags) \
	-L$(BUILDDIR)/../src \
	$(EXTRA_LDFLAGS) \
	-Wl,-z,start-stop-visibility=hidden \
	-Wl,-whole-archive $(BUILDDIR)/builtin_utest.a -Wl,-no-whole-archive \
	-lutils

ifneq ($(filter y,$(CONFIG_UTILS_ASSERT_API) $(CONFIG_UTILS_ASSERT_INTERN)),)
test-cflags   := $(filter-out -DNDEBUG,$(test-cflags))
utest-ldflags := $(filter-out -DNDEBUG,$(utest-ldflags))
endif # ($(filter y,$(CONFIG_UTILS_ASSERT_API) $(CONFIG_UTILS_ASSERT_INTERN)),)

builtins                         := builtin_utest.a
builtin_utest.a-objs             := utest.o $(config-obj)
builtin_utest.a-cflags           := $(test-cflags)

checkbins                        := utils-utest
utils-utest-objs                 := main.o
utils-utest-objs                 += $(call kconf_enabled, \
                                           UTILS_TIME, \
                                           time-utest.o)
utils-utest-cflags               := $(test-cflags)
utils-utest-ldflags              := $(utest-ldflags)
utils-utest-pkgconf              := libstroll libcute

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_LIST, \
                                           utils-timer-list-utest)
utils-timer-list-utest-objs      := list/timer-utest.o
utils-timer-list-utest-cflags    := $(test-cflags)
utils-timer-list-utest-ldflags   := $(utest-ldflags)
utils-timer-list-utest-pkgconf   := libetux_timer_list libstroll libcute

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_HWHEEL, \
                                           utils-timer-hwheel-utest)
utils-timer-hwheel-utest-objs    := hwheel/timer-utest.o
utils-timer-hwheel-utest-cflags  := $(test-cflags)
utils-timer-hwheel-utest-ldflags := $(utest-ldflags)
utils-timer-hwheel-utest-pkgconf := libetux_timer_hwheel libstroll libcute

# ex: filetype=make :
