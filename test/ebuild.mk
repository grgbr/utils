################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2017-2023 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

include ../common.mk

common-cflags  += -DUTILS_VERSION_STRING="\"$(VERSION)\""
common-ldflags += -DUTILS_VERSION_STRING="\"$(VERSION)\""

# Use -whole-archive to enforce the linker to scan builtin.a static library
# entirely so that symbols in utest.o may override existing strong symbols
# defined into other compilation units.
# This is required since we want stroll_assert_fail() defined into utest.c to
# override stroll_assert_fail() defined into libstroll.so for assertions testing
# purposes.
utest-ldflags := \
	-L$(BUILDDIR)/../timer \
	-L$(BUILDDIR)/../src \
	$(common-ldflags) \
	-Wl,-whole-archive $(BUILDDIR)/builtin_utest.a -Wl,-no-whole-archive \
	-lutils

ptest-ldflags := \
	-L$(BUILDDIR)/../timer \
	-L$(BUILDDIR)/../src \
	$(common-ldflags) \
	-Wl,-whole-archive $(BUILDDIR)/builtin_ptest.a -Wl,-no-whole-archive \
	-lutils

ptest-pkgconf := $(common-pkgconf) $(call kconf_enabled,ETUX_TRACE,lttng-ust)

builtins                         := builtin_utest.a
builtin_utest.a-objs             := utest.o timer_clock.o $(config-obj)
builtin_utest.a-cflags           := $(common-cflags)

checkbins                        := etux-utest
etux-utest-objs                  := main.o
etux-utest-objs                  += $(call kconf_enabled, \
                                           UTILS_TIME, \
                                           time_utest.o)
etux-utest-cflags                := $(common-cflags)
etux-utest-ldflags               := $(utest-ldflags)
etux-utest-pkgconf               := $(common-pkgconf) libcute

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_LIST, \
                                           etux-timer-list-utest)
etux-timer-list-utest-objs       := list/timer_utest.o
etux-timer-list-utest-cflags     := $(common-cflags) \
                                    -DETUX_TIMER_UTEST="\"eTux Timer List\""
etux-timer-list-utest-ldflags    := $(utest-ldflags) -letux_timer_list
etux-timer-list-utest-pkgconf    := $(common-pkgconf) libcute

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_HEAP, \
                                           etux-timer-heap-utest)
etux-timer-heap-utest-objs       := heap/timer_utest.o
etux-timer-heap-utest-cflags     := $(common-cflags) \
                                    -DETUX_TIMER_UTEST="\"eTux Timer Heap\""
etux-timer-heap-utest-ldflags    := $(utest-ldflags) -letux_timer_heap
etux-timer-heap-utest-pkgconf    := $(common-pkgconf) libcute

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_HWHEEL, \
                                           etux-timer-hwheel-utest)
etux-timer-hwheel-utest-objs     := hwheel/timer_utest.o
etux-timer-hwheel-utest-cflags   := $(common-cflags) \
                                    -DETUX_TIMER_UTEST="\"eTux Timer Hwheel\""
etux-timer-hwheel-utest-ldflags  := $(utest-ldflags) -letux_timer_hwheel
etux-timer-hwheel-utest-pkgconf  := $(common-pkgconf) libcute

ifeq ($(CONFIG_ETUX_PTEST),y)

builtins                         += builtin_ptest.a
builtin_ptest.a-objs             := ptest.o $(config-obj)
builtin_ptest.a-objs             += $(call kconf_enabled, \
                                           ETUX_TRACE, \
                                           trace_clock.o)
builtin_ptest.a-lots             := timer_clock.o
builtin_ptest.a-cflags           := $(common-cflags)

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_LIST, \
                                           etux-timer-list-ptest)
etux-timer-list-ptest-objs       := list/timer_ptest.o
etux-timer-list-ptest-cflags     := $(common-cflags)
etux-timer-list-ptest-ldflags    := $(ptest-ldflags) -letux_timer_list
etux-timer-list-ptest-pkgconf    := $(ptest-pkgconf)

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_HEAP, \
                                           etux-timer-heap-ptest)
etux-timer-heap-ptest-objs       := heap/timer_ptest.o
etux-timer-heap-ptest-cflags     := $(common-cflags)
etux-timer-heap-ptest-ldflags    := $(ptest-ldflags) -letux_timer_heap
etux-timer-heap-ptest-pkgconf    := $(ptest-pkgconf)

checkbins                        += $(call kconf_enabled, \
                                           ETUX_TIMER_HWHEEL, \
                                           etux-timer-hwheel-ptest)
etux-timer-hwheel-ptest-objs     := hwheel/timer_ptest.o
etux-timer-hwheel-ptest-cflags   := $(common-cflags)
etux-timer-hwheel-ptest-ldflags  := $(ptest-ldflags) -letux_timer_hwheel
etux-timer-hwheel-ptest-pkgconf  := $(ptest-pkgconf)

endif # ($(CONFIG_ETUX_PTEST),y)

# ex: filetype=make :
