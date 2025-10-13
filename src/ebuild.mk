################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

include ../common.mk

ifeq ($(CONFIG_UTILS_PROVIDES_LIBS),y)

libutils-objects      := $(call kconf_enabled,UTILS_SIGNAL,signal.o) \
                         $(call kconf_enabled,UTILS_THREAD,thread.o) \
                         $(call kconf_enabled,UTILS_TIME,time.o) \
                         $(call kconf_enabled,UTILS_PATH,path.o) \
                         $(call kconf_enabled,UTILS_FD,fd.o) \
                         $(call kconf_enabled,UTILS_FILE,file.o) \
                         $(call kconf_enabled,UTILS_DIR,dir.o) \
                         $(call kconf_enabled,ETUX_FSTREE,fstree.o) \
                         $(call kconf_enabled,UTILS_STR,string.o) \
                         $(call kconf_enabled,UTILS_POLL,poll.o) \
                         $(call kconf_enabled,UTILS_MQUEUE,mqueue.o) \
                         $(call kconf_enabled,UTILS_NET,net.o) \
                         $(call kconf_enabled,UTILS_PWD,pwd.o)

ifeq ($(CONFIG_ETUX_NET),y)
builtins               := shared/builtin.a
shared/builtin.a-lots  := ../net/shared/builtin.a
endif # ($(CONFIG_ETUX_NET),y)

solibs                 := libutils.so
libutils.so-objs       := $(addprefix shared/,$(libutils-objects))
shared/thread.o-cflags := -pthread $(shared-common-cflags)
libutils.so-cflags     := $(shared-common-cflags)
libutils.so-ldflags    := $(call kconf_enabled,UTILS_THREAD,-pthread) \
                          $(shared-common-ldflags)
ifeq ($(CONFIG_ETUX_NET),y)
libutils.so-ldflags    += -Wl,--push-state,--whole-archive \
                          -l:shared/builtin.a \
                          -Wl,--pop-state \
                          -Wl,-soname,libutils.so
endif # ($(CONFIG_ETUX_NET),y)
libutils.so-pkgconf    := $(common-pkgconf)


arlibs                 := libutils.a
libutils.a-objs        := $(addprefix static/,$(libutils-objects))
libutils.a-lots        := $(call kconf_enabled, \
                                 ETUX_NET, \
                                 ../net/static/builtin.a)
static/thread.o-cflags := -pthread $(shared-common-cflags)
libutils.a-cflags      := $(common-cflags)
libutils.a-pkgconf     := $(common-pkgconf)

endif # ifeq ($(CONFIG_UTILS_PROVIDES_LIBS),y)

# ex: filetype=make :
