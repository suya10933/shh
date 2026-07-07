# Makefile

SHELL := /bin/bash
.DEFAULT_GOAL := all

include env.mk

.PHONY: all
all: image

.PHONY: toolchain
toolchain:
	$(MAKE) -C toolchain all
