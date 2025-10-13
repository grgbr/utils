################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

override PACKAGE := utils
override VERSION := 1.0
EXTRA_CFLAGS     := -O2 -DNDEBUG -Wall -Wextra -Wformat=2
EXTRA_LDFLAGS    := -O2

export VERSION EXTRA_CFLAGS EXTRA_LDFLAGS

ifeq ($(strip $(EBUILDDIR)),)
ifneq ($(realpath ebuild/main.mk),)
EBUILDDIR := $(realpath ebuild)
else  # ($(realpath ebuild/main.mk),)
EBUILDDIR := $(realpath /usr/share/ebuild)
endif # !($(realpath ebuild/main.mk),)
endif # ($(strip $(EBUILDDIR)),)

ifeq ($(realpath $(EBUILDDIR)/main.mk),)
$(error '$(EBUILDDIR)': no valid eBuild install found !)
endif # ($(realpath $(EBUILDDIR)/main.mk),)

include $(EBUILDDIR)/main.mk

ifeq ($(CONFIG_UTILS_UTEST),y)

ifeq ($(strip $(CROSS_COMPILE)),)

define list_check_confs_cmds :=
if ! nr=$$($(PYTHON) $(TOPDIR)/scripts/gen_check_confs.py count); then \
	exit 1; \
fi; \
c=0; \
while [ $$c -lt $$nr ]; do \
	echo $$c; \
	c=$$((c + 1)); \
done
endef

check_conf_list       := $(shell $(list_check_confs_cmds))
ifeq ($(strip $(check_conf_list)),)
$(error Missing testing build configurations !)
endif

check_conf_targets    := $(addprefix check-conf,$(check_conf_list))
checkall_builddir     := $(BUILDDIR)/checkall
check_lib_search_path := \
	$(BUILDDIR)/src$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))

CHECK_FORCE ?= y
ifneq ($(filter y 1,$(CHECK_FORCE)),)
.PHONY: $(BUILDDIR)/test/utils-utest.xml
endif

CHECK_HALT_ON_FAIL ?= n
ifeq ($(filter y 1,$(CHECK_HALT_ON_FAIL)),)
K := --keep-going
else
K := --no-keep-going
endif

CHECK_VERBOSE ?= --silent

.PHONY: checkall
checkall: $(check_conf_targets)

.PHONY: $(check_conf_targets)
$(check_conf_targets): check-conf%: $(checkall_builddir)/conf%/.config
	$(Q)$(MAKE) $(K) -C $(TOPDIR) check BUILDDIR:='$(abspath $(dir $(<)))'
	$(Q)cute-report join --package 'Utils' \
	                     --revision '$(VERSION)' \
	                     $(checkall_builddir)/utils-all-utest.xml \
	                     $(dir $(<))/test/utils-utest.xml \
	                     utils-conf$(*)

$(addprefix $(checkall_builddir)/conf,$(addsuffix /.config,$(check_conf_list))): \
$(checkall_builddir)/conf%/.config: $(checkall_builddir)/conf%/test.config \
                                    $(config-in)
	$(Q)env KCONFIG_ALLCONFIG='$(<)' \
	        KCONFIG_CONFIG='$(@)' \
	        $(KCONF) --allnoconfig '$(config-in)' >/dev/null
	$(Q)$(MAKE) -C $(TOPDIR) olddefconfig BUILDDIR:='$(abspath $(dir $(@)))'

$(addprefix $(checkall_builddir)/conf,$(addsuffix /test.config,$(check_conf_list))): \
$(checkall_builddir)/conf%/test.config: $(TOPDIR)/scripts/gen_check_confs.py
	@mkdir -p $(dir $(@))
	$(Q)$(PYTHON) $(TOPDIR)/scripts/gen_check_confs.py genone $(*) $(@)

.PHONY: check
check: $(BUILDDIR)/test/utils-utest.xml

#TODO: merge test timer reports with utests report

$(BUILDDIR)/test/utils-utest.xml: | build-check
	@echo "  CHECK   $(@)"
	$(Q)env LD_LIBRARY_PATH="$(check_lib_search_path)" \
	        $(BUILDDIR)/test/utils-utest \
	        $(CHECK_VERBOSE) \
	        --xml='$(@)' \
	        run

ifeq ($(CONFIG_ETUX_TIMER_LIST),y)

check: $(BUILDDIR)/test/etux-timer-list-utest.xml

ifneq ($(filter y 1,$(CHECK_FORCE)),)
.PHONY: $(BUILDDIR)/test/etux-timer-list-utest.xml
endif
$(BUILDDIR)/test/etux-timer-list-utest.xml: | build-check
	@echo "  CHECK   $(@)"
	$(Q)env LD_LIBRARY_PATH="$(check_lib_search_path)" \
	        $(BUILDDIR)/test/etux-timer-list-utest \
	        $(CHECK_VERBOSE) \
	        --xml='$(@)' \
	        run

endif # ($(CONFIG_ETUX_TIMER_LIST),y)

ifeq ($(CONFIG_ETUX_TIMER_HEAP),y)

check: $(BUILDDIR)/test/etux-timer-heap-utest.xml

ifneq ($(filter y 1,$(CHECK_FORCE)),)
.PHONY: $(BUILDDIR)/test/etux-timer-heap-utest.xml
endif
$(BUILDDIR)/test/etux-timer-heap-utest.xml: | build-check
	@echo "  CHECK   $(@)"
	$(Q)env LD_LIBRARY_PATH="$(check_lib_search_path)" \
	        $(BUILDDIR)/test/etux-timer-heap-utest \
	        $(CHECK_VERBOSE) \
	        --xml='$(@)' \
	        run

endif # ($(CONFIG_ETUX_TIMER_HEAP),y)

ifeq ($(CONFIG_ETUX_TIMER_HWHEEL),y)

check: $(BUILDDIR)/test/etux-timer-hwheel-utest.xml

ifneq ($(filter y 1,$(CHECK_FORCE)),)
.PHONY: $(BUILDDIR)/test/etux-timer-hwheel-utest.xml
endif
$(BUILDDIR)/test/etux-timer-hwheel-utest.xml: | build-check
	@echo "  CHECK   $(@)"
	$(Q)env LD_LIBRARY_PATH="$(check_lib_search_path)" \
	        $(BUILDDIR)/test/etux-timer-hwheel-utest \
	        $(CHECK_VERBOSE) \
	        --xml='$(@)' \
	        run

endif # ($(CONFIG_ETUX_TIMER_HWHEEL),y)

clean-check: _clean-check

.PHONY: _clean-check
_clean-check:
	$(call rm_recipe,$(BUILDDIR)/test/utils-utest.xml)
	$(call rm_recipe,$(BUILDDIR)/test/etux-timer-list-utest.xml)
	$(call rm_recipe,$(BUILDDIR)/test/etux-timer-heap-utest.xml)
	$(call rm_recipe,$(BUILDDIR)/test/etux-timer-hwheel-utest.xml)

clean-check: _clean-checkall

.PHONY: _clean-checkall
_clean-checkall:
	$(call rmr_recipe,$(checkall_builddir))

else  # ifneq ($(strip $(CROSS_COMPILE)),)

.PHONY: check
check checkall:
	$(error Cannot check while cross building !)

endif # ifeq ($(strip $(CROSS_COMPILE)),)

else  # ifneq ($(CONFIG_UTILS_UTEST),y)

.PHONY: check
check checkall:

endif # ifeq ($(CONFIG_UTILS_UTEST),y)
