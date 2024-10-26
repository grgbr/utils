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
                        -D_GNU_SOURCE \
                        -I ../include \
                        $(EXTRA_CFLAGS) \
                        $(call kconf_enabled,UTILS_THREAD,-pthread)

common-ldflags       := $(common-cflags) $(EXTRA_LDFLAGS) \
                        -Wl,-z,start-stop-visibility=hidden

ifneq ($(filter y,$(CONFIG_UTILS_ASSERT_API) $(CONFIG_UTILS_ASSERT_INTERN)),)
common-cflags        := $(filter-out -DNDEBUG,$(common-cflags))
common-ldflags       := $(filter-out -DNDEBUG,$(common-ldflags))
endif # ($(filter y,$(CONFIG_UTILS_ASSERT_API) $(CONFIG_UTILS_ASSERT_INTERN)),)

ifeq ($(CONFIG_UTILS_PROVIDES_LIBS),y)
solibs               := libutils.so
libutils.so-objs     += $(call kconf_enabled,UTILS_SIGNAL,shared/signal.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_THREAD,shared/thread.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_TIME,shared/time.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_TIMER,shared/timer.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_PATH,shared/path.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_FD,shared/fd.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_FILE,shared/file.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_DIR,shared/dir.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_STR,shared/string.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_POLL,shared/poll.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_UNSK,shared/unsk.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_MQUEUE,shared/mqueue.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_NET,shared/net.o)
libutils.so-objs     += $(call kconf_enabled,UTILS_PWD,shared/pwd.o)
libutils.so-cflags   := $(filter-out -fpie -fPIE,$(common-cflags)) -fpic
libutils.so-ldflags  := $(filter-out -pie -fpie -fPIE,$(common-ldflags)) \
                        -shared -Bsymbolic -fpic -Wl,-soname,libutils.so
libutils.so-pkgconf  := libstroll

arlibs               := libutils.a
libutils.a-objs      += $(call kconf_enabled,UTILS_SIGNAL,static/signal.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_THREAD,static/thread.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_TIME,static/time.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_TIMER,static/timer.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_PATH,static/path.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_FD,static/fd.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_FILE,static/file.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_DIR,static/dir.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_STR,static/string.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_POLL,static/poll.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_UNSK,static/unsk.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_MQUEUE,static/mqueue.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_NET,static/net.o)
libutils.a-objs      += $(call kconf_enabled,UTILS_PWD,static/pwd.o)
libutils.a-cflags    := $(common-cflags)
libutils.a-pkgconf   := libstroll
endif # ifeq ($(CONFIG_UTILS_PROVIDES_LIBS),y)

# ex: filetype=make :
