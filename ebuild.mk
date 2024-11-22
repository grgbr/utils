################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

config-in           := Config.in
config-h            := utils/config.h
config-obj          := config.o

HEADERDIR           := $(CURDIR)/include
headers              = utils/cdefs.h
headers             += $(call kconf_enabled,UTILS_ATOMIC,utils/atomic.h)
headers             += $(call kconf_enabled,UTILS_SIGNAL,utils/signal.h)
headers             += $(call kconf_enabled,UTILS_THREAD,utils/thread.h)
headers             += $(call kconf_enabled,UTILS_TIME,utils/time.h)
headers             += $(call kconf_enabled,UTILS_TIMER,utils/timer.h)
headers             += $(call kconf_enabled,UTILS_PATH,utils/path.h)
headers             += $(call kconf_enabled,UTILS_FD,utils/fd.h)
headers             += $(call kconf_enabled,UTILS_PIPE,utils/pipe.h)
headers             += $(call kconf_enabled,UTILS_FILE,utils/file.h)
headers             += $(call kconf_enabled,UTILS_DIR,utils/dir.h)
headers             += $(call kconf_enabled,UTILS_STR,utils/string.h)
headers             += $(call kconf_enabled,UTILS_POLL,utils/poll.h)
headers             += $(call kconf_enabled,UTILS_UNSK,utils/unsk.h)
headers             += $(call kconf_enabled,UTILS_MQUEUE,utils/mqueue.h)
headers             += $(call kconf_enabled,UTILS_NET,utils/net.h)
headers             += $(call kconf_enabled,UTILS_PWD,utils/pwd.h)

subdirs   := src

ifeq ($(CONFIG_UTILS_UTEST),y)
subdirs   += test
test-deps := src
endif # ($(CONFIG_UTILS_UTEST),y)

ifeq ($(CONFIG_ETUX_TIMER),y)
subdirs    += timer
timer-deps := src
test-deps  += timer
endif # ($(CONFIG_ETUX_TIMER),y)

ifeq ($(CONFIG_UTILS_PROVIDES_LIBS),y)
override libutils_pkgconf_libs := \
	Libs: -L$${libdir} \
	-Wl,--push-state,--as-needed \
	-lutils \
	$(call kconf_enabled,UTILS_THREAD,-pthread) \
	$(call kconf_enabled,UTILS_MQUEUE,-lrt) \
	-Wl,--pop-state
endif # ifeq ($(CONFIG_UTILS_PROVIDES_LIBS),y)

define libutils_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: libutils
Description: Utils library
Version: $(VERSION)
Requires: libstroll
Cflags: -I$${includedir} $(call kconf_enabled,UTILS_THREAD,-pthread)
$(libutils_pkgconf_libs)
endef

pkgconfigs          := libutils.pc
libutils.pc-tmpl    := libutils_pkgconf_tmpl

################################################################################
# Source code tags generation
################################################################################

tagfiles := $(shell find $(addprefix $(CURDIR)/,$(subdirs)) \
                         $(HEADERDIR) \
                         -type f)
