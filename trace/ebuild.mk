################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

include ../common.mk

common-cflags            += -fvisibility=hidden
shared-common-cflags     += -fvisibility=hidden
common-ldflags           += -fvisibility=hidden
shared-common-ldflags    += -fvisibility=hidden

builtins                 := shared/builtin.a
shared/builtin.a-objs    := shared/trace.o
shared/builtin.a-cflags  := $(shared-common-cflags)
shared/builtin.a-pkgconf := lttng-ust

builtins                 += static/builtin.a
static/builtin.a-objs    := static/trace.o
static/builtin.a-cflags  := $(common-cflags)
static/builtin.a-pkgconf := lttng-ust

install install-strip: \
	$(DESTDIR)$(BINDIR)/etux-trace

$(DESTDIR)$(BINDIR)/etux-trace: $(SRCDIR)/trace.sh
	$(call install_recipe,--mode=755,$(<),$(@))

uninstall-check: _uninstall-check

.PHONY: _uninstall-check
_uninstall-check:
	$(call rm_recipe,$(DESTDIR)$(BINDIR)/etux-trace)
