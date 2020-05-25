config-in           := Config.in
config-h            := utils/config.h

solibs              := libutils.so
libutils.so-objs     = $(call kconf_enabled,UTILS_ASSERT,assert.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_THREAD,thread.o)
libutils.so-objs    += $(call kconf_enabled,UTILS_TIME,time.o)
libutils.so-cflags  := $(EXTRA_CFLAGS) -Wall -Wextra -D_GNU_SOURCE -DPIC -fpic
libutils.so-cflags  += $(call kconf_enabled,UTILS_THREAD,-pthread)
libutils.so-ldflags  = $(EXTRA_LDFLAGS) -shared -fpic -Wl,-soname,libutils.so
libutils.so-ldflags += $(call kconf_enabled,UTILS_BTRACE,-rdynamic)
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
