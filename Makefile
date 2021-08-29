#EBUILDDIR     := $(CURDIR)/ebuild
PACKAGE       := utils
EXTRA_CFLAGS  := -O2 -DNDEBUG -Wformat=2
EXTRA_LDFLAGS := -O2

export EXTRA_CFLAGS EXTRA_LDFLAGS

include $(EBUILDDIR)/main.mk
