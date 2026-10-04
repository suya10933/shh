# userspace/Makefile

include ../env.mk

.PHONY: all
all: init

.PHONY: init
init:
	$(MAKE) -C init

.PHONY: install
install:
	$(MAKE) -C init install

.PHONY: clean
clean:
	$(MAKE) -C init clean
