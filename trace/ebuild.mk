################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

include ../common.mk

install install-strip: \
	$(DESTDIR)$(BINDIR)/etux-trace

$(DESTDIR)$(BINDIR)/etux-trace: $(SRCDIR)/trace.sh
	$(call install_recipe,--mode=755,$(<),$(@))

uninstall-check: _uninstall-check

.PHONY: _uninstall-check
_uninstall-check:
	$(call rm_recipe,$(DESTDIR)$(BINDIR)/etux-trace)
