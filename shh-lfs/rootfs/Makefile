# rootfs/Makefile

include ../env.mk

.PHONY: all
all:
	mkdir -p $(ROOTFS)

.PHONY: install
install: all
	mkdir -p \
		$(ROOTFS)/dev \
		$(ROOTFS)/proc \
		$(ROOTFS)/sys \
		$(ROOTFS)/run \
		$(ROOTFS)/tmp \
		$(ROOTFS)/sbin
	chmod 1777 $(ROOTFS)/tmp

.PHONY: clean
clean:
	rm -rf $(ROOTFS)
