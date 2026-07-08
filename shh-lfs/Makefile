# Makefile

SHELL := /bin/bash
.DEFAULT_GOAL := all

include env.mk

.PHONY: all
all: image

.PHONY: toolchain
toolchain:
	$(MAKE) -C toolchain all

.PHONY: firmware
firmware:
	$(MAKE) -C firmware all

.PHONY: clean
clean:
	$(MAKE) -C toolchain clean
	$(MAKE) -C firmware clean
