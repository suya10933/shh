# Makefile

SHELL := /bin/bash
.DEFAULT_GOAL := all

include env.mk

.PHONY: all
all: toolchain firmware kernel

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
u-boot: toolchain
	$(MAKE) -C u-boot all

.PHONY: image
image: install
	mkdir -p $(IMAGES)
	$(PROJECT_ROOT)scripts/image.sh release
	$(PROJECT_ROOT)scripts/image.sh debug

.PHONY: install
install: all
	$(MAKE) -C kernel install
	$(MAKE) -C firmware install
	$(MAKE) -C u-boot install

.PHONY: clean
clean:
	$(MAKE) -C toolchain clean
	$(MAKE) -C kernel clean
	$(MAKE) -C firmware clean
	$(MAKE) -C u-boot clean
	rm -rf $(OUT)
