# Makefile

SHELL := /bin/bash
.DEFAULT_GOAL := all

include env.mk

.PHONY: all
all: toolchain firmware kernel u-boot userspace

.PHONY: release debug
release:
	$(MAKE) PROFILE=release all

debug:
	$(MAKE) PROFILE=debug all

.PHONY: toolchain
toolchain:
	$(MAKE) -C toolchain all

.PHONY: firmware
firmware:
	$(MAKE) -C firmware all

.PHONY: kernel
kernel: toolchain
	$(MAKE) -C kernel all

.PHONY: u-boot
u-boot: toolchain kernel
ifeq ($(PROFILE),debug)
	$(MAKE) -C u-boot all
else
	@echo "No u-boot for release build."
endif

.PHONY: userspace
userspace: toolchain
	$(MAKE) -C userspace all

.PHONY: install
install: all
	$(MAKE) -C kernel install
	$(MAKE) -C firmware install
ifeq ($(PROFILE),debug)
	$(MAKE) -C u-boot install
endif
	$(MAKE) -C userspace install
	$(MAKE) -C rootfs install

.PHONY: image
image: install
	mkdir -p $(IMAGES)
	$(PROJECT_ROOT)scripts/image.sh $(PROFILE)

.PHONY: release-image debug-image
release-image:
	$(MAKE) PROFILE=release image

debug-image:
	$(MAKE) PROFILE=debug image

.PHONY: clean distclean
clean:
	rm -rf $(PROFILE_OUT)

distclean:
	$(MAKE) -C toolchain clean
	$(MAKE) -C kernel clean
	$(MAKE) -C firmware clean
	$(MAKE) -C u-boot clean
	rm -rf $(OUT)
