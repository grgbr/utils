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
headers             += utils/syscall.h
headers             += $(call kconf_enabled,UTILS_ATOMIC,utils/atomic.h)
headers             += $(call kconf_enabled,UTILS_SIGNAL,utils/signal.h)
headers             += $(call kconf_enabled,UTILS_THREAD,utils/thread.h)
headers             += $(call kconf_enabled,UTILS_TIME,utils/time.h)
headers             += $(call kconf_enabled,ETUX_TIMER,utils/timer.h)
headers             += $(call kconf_enabled,UTILS_PATH,utils/path.h)
headers             += $(call kconf_enabled,UTILS_FD,utils/fd.h)
headers             += $(call kconf_enabled,UTILS_PIPE,utils/pipe.h)
headers             += $(call kconf_enabled,UTILS_FILE,utils/file.h)
headers             += $(call kconf_enabled,UTILS_DIR,utils/dir.h)
headers             += $(call kconf_enabled,ETUX_FSTREE,utils/fstree.h)
headers             += $(call kconf_enabled,UTILS_STR,utils/string.h)
headers             += $(call kconf_enabled,UTILS_POLL,utils/poll.h)
headers             += $(call kconf_enabled,UTILS_MQUEUE,utils/mqueue.h)
headers             += $(call kconf_enabled,UTILS_NET,utils/net.h)
headers             += $(call kconf_enabled,UTILS_PWD,utils/pwd.h)
headers             += $(call kconf_enabled,ETUX_NETDB,utils/netdb.h)
headers             += $(call kconf_enabled,ETUX_NETIF,utils/netif.h)
headers             += $(call kconf_enabled,ETUX_SOCK,utils/sock.h)
headers             += $(call kconf_enabled,ETUX_INSK,utils/insk.h)
headers             += $(call kconf_enabled,ETUX_IN4SK,utils/in4sk.h)
headers             += $(call kconf_enabled,ETUX_IN6SK,utils/in6sk.h)
headers             += $(call kconf_enabled,UTILS_UNSK,utils/unsk.h)
headers             += $(call kconf_enabled,UTILS_PRNG,utils/prng.h)

subdirs   := src

ifeq ($(CONFIG_ETUX_TRACE),y)
subdirs   += trace
src-deps  := trace
endif # ($(CONFIG_ETUX_TRACE),y)

ifeq ($(CONFIG_UTILS_UTEST),y)
subdirs   += test
test-deps := src
endif # ($(CONFIG_UTILS_UTEST),y)

ifeq ($(CONFIG_ETUX_TIMER),y)
subdirs    += timer
timer-deps := src
test-deps  += timer
endif # ($(CONFIG_ETUX_TIMER),y)

ifeq ($(CONFIG_ETUX_NET),y)
subdirs    += net
src-deps   += net
test-deps  += net
endif # ($(CONFIG_ETUX_NET),y)

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

define libetux_timer_list_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: libetux_timer_list
Description: eTux timer list library
Version: $(VERSION)
Requires: libutils libstroll
Cflags: -I$${includedir}
Libs: -L$${libdir} \
      -Wl,--push-state,--as-needed -letux_timer_list -Wl,--pop-state
endef

pkgconfigs                 += $(call kconf_enabled,ETUX_TIMER_LIST, \
                                libetux_timer_list.pc)
libetux_timer_list.pc-tmpl := libetux_timer_list_pkgconf_tmpl

define libetux_timer_heap_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: libetux_timer_heap
Description: eTux timer heap library
Version: $(VERSION)
Requires: libutils libstroll
Cflags: -I$${includedir}
Libs: -L$${libdir} \
      -Wl,--push-state,--as-needed -letux_timer_heap -Wl,--pop-state
endef

pkgconfigs                 += $(call kconf_enabled,ETUX_TIMER_HEAP, \
                                libetux_timer_heap.pc)
libetux_timer_heap.pc-tmpl := libetux_timer_heap_pkgconf_tmpl

define libetux_timer_hwheel_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: libetux_timer_hwheel
Description: eTux hierarchical timer wheel library
Version: $(VERSION)
Requires: libutils libstroll
Cflags: -I$${includedir}
Libs: -L$${libdir} \
      -Wl,--push-state,--as-needed -letux_timer_hwheel -Wl,--pop-state
endef

pkgconfigs                   += $(call kconf_enabled,ETUX_TIMER_HWHEEL, \
                                  libetux_timer_hwheel.pc)
libetux_timer_hwheel.pc-tmpl := libetux_timer_hwheel_pkgconf_tmpl

################################################################################
# Source code tags generation
################################################################################

tagfiles := $(shell find $(addprefix $(CURDIR)/,$(subdirs)) \
                         $(HEADERDIR) \
                         -type f)

################################################################################
# Documentation generation
################################################################################

doxyconf  := $(CURDIR)/sphinx/Doxyfile
doxyenv   := SRCDIR="$(HEADERDIR) $(SRCDIR)"

sphinxsrc := $(CURDIR)/sphinx
sphinxenv := \
	VERSION="$(VERSION)" \
	$(if $(strip $(EBUILDDOC_TARGET_PATH)), \
	     EBUILDDOC_TARGET_PATH="$(strip $(EBUILDDOC_TARGET_PATH))") \
	$(if $(strip $(EBUILDDOC_INVENTORY_PATH)), \
	     EBUILDDOC_INVENTORY_PATH="$(strip $(EBUILDDOC_INVENTORY_PATH))") \
	$(if $(strip $(STROLLDOC_TARGET_PATH)), \
	     STROLLDOC_TARGET_PATH="$(strip $(STROLLDOC_TARGET_PATH))") \
	$(if $(strip $(STROLLDOC_INVENTORY_PATH)), \
	     STROLLDOC_INVENTORY_PATH="$(strip $(STROLLDOC_INVENTORY_PATH))") \
	$(if $(strip $(CUTEDOC_TARGET_PATH)), \
	     CUTEDOC_TARGET_PATH="$(strip $(CUTEDOC_TARGET_PATH))") \
	$(if $(strip $(CUTEDOC_INVENTORY_PATH)), \
	     CUTEDOC_INVENTORY_PATH="$(strip $(CUTEDOC_INVENTORY_PATH))")
