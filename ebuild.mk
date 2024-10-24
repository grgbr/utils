config-in           := Config.in
config-h            := utils/config.h

solibs              := libutils.so
libutils.so-objs    += $(call kconf_enabled,UTILS_SIGNAL,signal.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_THREAD,thread.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_TIME,time.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_TIMER,timer.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_PATH,path.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_FD,fd.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_FILE,file.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_DIR,dir.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_STR,string.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_POLL,poll.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_UNSK,unsk.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_MQUEUE,mqueue.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_NET,net.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_PWD,pwd.o)
libutils.so-cflags   = $(EXTRA_CFLAGS) -Wall -Wextra -D_GNU_SOURCE -DPIC -fpic
libutils.so-cflags  += $(call kconf_enabled,UTILS_THREAD,-pthread)
libutils.so-ldflags  = $(EXTRA_LDFLAGS) -shared -Bsymbolic -fpic -Wl,-soname,libutils.so
libutils.so-ldflags += $(call kconf_enabled,UTILS_THREAD,-pthread)
libutils.so-pkgconf := libstroll

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
headers             += $(call kconf_enabled,UTILS_UNSK,utils/mqueue.h)
headers             += $(call kconf_enabled,UTILS_NET,utils/net.h)
headers             += $(call kconf_enabled,UTILS_PWD,utils/pwd.h)

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
Libs: -L$${libdir} \
      -Wl,--push-state,--as-needed \
      -lutils \
      $(call kconf_enabled,UTILS_THREAD,-pthread) \
      $(call kconf_enabled,UTILS_MQUEUE,-lrt) \
      -Wl,--pop-state
endef

pkgconfigs          := libutils.pc
libutils.pc-tmpl    := libutils_pkgconf_tmpl

################################################################################
# Source code tags generation
################################################################################

tagfiles := $(shell find $(CURDIR) -type f)
