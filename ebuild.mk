config-in           := Config.in
config-h            := utils/config.h

solibs              := libutils.so
libutils.so-objs     = sys.o
libutils.so-objs    += $(call kconf_enabled,UTILS_ASSERT,assert.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_THREAD,thread.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_TIME,time.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_BITMAP,bitmap.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_PATH,path.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_FILE,file.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_DIR,dir.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_DLIST,dlist.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_STR,string.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_PILE,pile.o)
libutils.so-cflags   = $(EXTRA_CFLAGS) -Wall -Wextra -D_GNU_SOURCE -DPIC -fpic
libutils.so-cflags  += $(call kconf_enabled,UTILS_THREAD,-pthread)
libutils.so-ldflags  = $(EXTRA_LDFLAGS) -shared -fpic -Wl,-soname,libutils.so
libutils.so-ldflags += $(call kconf_enabled,UTILS_THREAD,-pthread)

HEADERDIR           := $(CURDIR)/include
headers              = utils/cdefs.h
headers             += $(call kconf_enabled,UTILS_ASSERT,utils/assert.h)
headers             += $(call kconf_enabled,UTILS_BITOPS,utils/bitops.h)
headers             += $(call kconf_enabled,UTILS_ATOMIC,utils/atomic.h)
headers             += $(call kconf_enabled,UTILS_POW2,utils/pow2.h)
headers             += $(call kconf_enabled,UTILS_SIGNAL,utils/signal.h)
headers             += $(call kconf_enabled,UTILS_THREAD,utils/thread.h)
headers             += $(call kconf_enabled,UTILS_TIME,utils/time.h)
headers             += $(call kconf_enabled,UTILS_BITMAP,utils/bitmap.h)
headers             += $(call kconf_enabled,UTILS_PATH,utils/path.h)
headers             += $(call kconf_enabled,UTILS_FILE,utils/file.h)
headers             += $(call kconf_enabled,UTILS_DIR,utils/dir.h)
headers             += $(call kconf_enabled,UTILS_DLIST,utils/dlist.h)
headers             += $(call kconf_enabled,UTILS_DLIST,utils/string.h)
headers             += $(call kconf_enabled,UTILS_PILE,utils/pile.h)
headers             += $(call kconf_enabled,UTILS_NET,utils/net.h)

define libutils_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: libutils
Description: Utils library
Version: %%PKG_VERSION%%
Requires:
Cflags: -I$${includedir} $(call kconf_enabled,UTILS_THREAD,-pthread)
Libs: -L$${libdir} -lutils $(call kconf_enabled,UTILS_THREAD,-pthread)
endef

pkgconfigs          := libutils.pc
libutils.pc-tmpl    := libutils_pkgconf_tmpl
