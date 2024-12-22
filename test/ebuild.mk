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
	-L$(BUILDDIR)/../timer \
	-L$(BUILDDIR)/../src \
	$(EXTRA_LDFLAGS) \
	-Wl,-z,start-stop-visibility=hidden \
	-Wl,-whole-archive $(BUILDDIR)/builtin_utest.a -Wl,-no-whole-archive \
	-lutils

ptest-ldflags := \
	$(test-cflags) \
	-L$(BUILDDIR)/../timer \
	-L$(BUILDDIR)/../src \
	$(EXTRA_LDFLAGS) \
	-Wl,-z,start-stop-visibility=hidden \
	-Wl,-whole-archive $(BUILDDIR)/builtin_ptest.a -Wl,-no-whole-archive \
	-lutils

test-pkgconf  := libstroll $(call kconf_enabled,ETUX_TRACE,lttng-ust)

ifneq ($(filter y,$(CONFIG_UTILS_ASSERT_API) $(CONFIG_UTILS_ASSERT_INTERN)),)
test-cflags   := $(filter-out -DNDEBUG,$(test-cflags))
utest-ldflags := $(filter-out -DNDEBUG,$(utest-ldflags))
ptest-ldflags := $(filter-out -DNDEBUG,$(ptest-ldflags))
endif # ($(filter y,$(CONFIG_UTILS_ASSERT_API) $(CONFIG_UTILS_ASSERT_INTERN)),)

builtins                         := builtin_utest.a
builtin_utest.a-objs             := utest.o timer_clock.o $(config-obj)
builtin_utest.a-cflags           := $(test-cflags)

checkbins                        := etux-utest
etux-utest-objs                 := main.o
etux-utest-objs                 += $(call kconf_enabled, \
                                           UTILS_TIME, \
                                           time_utest.o)
etux-utest-cflags               := $(test-cflags)
etux-utest-ldflags              := $(utest-ldflags)
etux-utest-pkgconf              := $(test-pkgconf) libcute

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_LIST, \
                                           etux-timer-list-utest)
etux-timer-list-utest-objs       := list/timer_utest.o
etux-timer-list-utest-cflags     := $(test-cflags)
etux-timer-list-utest-ldflags    := $(utest-ldflags) -letux_timer_list
etux-timer-list-utest-pkgconf    := $(test-pkgconf) libcute

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_HEAP, \
                                           etux-timer-heap-utest)
etux-timer-heap-utest-objs       := heap/timer_utest.o
etux-timer-heap-utest-cflags     := $(test-cflags)
etux-timer-heap-utest-ldflags    := $(utest-ldflags) -letux_timer_heap
etux-timer-heap-utest-pkgconf    := $(test-pkgconf) libcute

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_HWHEEL, \
                                           etux-timer-hwheel-utest)
etux-timer-hwheel-utest-objs     := hwheel/timer_utest.o
etux-timer-hwheel-utest-cflags   := $(test-cflags)
etux-timer-hwheel-utest-ldflags  := $(utest-ldflags) -letux_timer_hwheel
etux-timer-hwheel-utest-pkgconf  := $(test-pkgconf) libcute

ifeq ($(CONFIG_ETUX_PTEST),y)

builtins                         := builtin_ptest.a
builtin_ptest.a-objs             := ptest.o timer_clock.o $(config-obj)
builtin_ptest.a-cflags           := $(test-cflags)

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_LIST, \
                                           etux-timer-list-ptest)
etux-timer-list-ptest-objs       := list/timer_ptest.o
etux-timer-list-ptest-cflags     := $(test-cflags)
etux-timer-list-ptest-ldflags    := $(ptest-ldflags) -letux_timer_list
etux-timer-list-ptest-pkgconf    := $(test-pkgconf)

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_HEAP, \
                                           etux-timer-heap-ptest)
etux-timer-heap-ptest-objs       := heap/timer_ptest.o
etux-timer-heap-ptest-cflags     := $(test-cflags)
etux-timer-heap-ptest-ldflags    := $(ptest-ldflags) -letux_timer_heap
etux-timer-heap-ptest-pkgconf    := $(test-pkgconf)

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_HWHEEL, \
                                           etux-timer-hwheel-ptest)
etux-timer-hwheel-ptest-objs     := hwheel/timer_ptest.o
etux-timer-hwheel-ptest-cflags   := $(test-cflags)
etux-timer-hwheel-ptest-ldflags  := $(ptest-ldflags) -letux_timer_hwheel
etux-timer-hwheel-ptest-pkgconf  := $(test-pkgconf)

endif # ($(CONFIG_ETUX_PTEST),y)

# ex: filetype=make :
