# u-boot/Makefile

include ../env.mk

.PHONY: all
all: build

$(UBOOT_ROOT)/u-boot:
	git clone $(UBOOT_GIT_URL) $(UBOOT_SRC)
	cd $(UBOOT_SRC) && git checkout $(UBOOT_VERSION)

$(UBOOT_BUILD)/.config: $(UBOOT_ROOT)/u-boot
	mkdir -p $(UBOOT_BUILD)
	cp $(UBOOT_CONFIG) $@

.PHONY: setconfig
setconfig: $(UBOOT_BUILD)/.config
	$(MAKE) -C $(UBOOT_SRC) \
		ARCH=$(UBOOT_ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		O=$(UBOOT_BUILD) \
		olddefconfig

.PHONY: menuconfig
menuconfig: setconfig
	$(MAKE) -C $(UBOOT_SRC) \
		ARCH=$(UBOOT_ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		O=$(UBOOT_BUILD) \
		menuconfig

.PHONY: saveconfig
saveconfig: $(UBOOT_BUILD)/.config
	cp $^ $(UBOOT_CONFIG)

.PHONY: build
build: setconfig
	mkdir -p $(UBOOT_BUILD)
	$(MAKE) -C $(UBOOT_SRC) \
		ARCH=$(UBOOT_ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		O=$(UBOOT_BUILD)

.PHONY: install
install: build
	mkdir -p $(DEBUG_BOOT)/
	cp $(UBOOT_BUILD)/u-boot.bin \
		$(DEBUG_BOOT)/

.PHONY: clean
clean:
	rm -rf $(UBOOT_BUILD) $(UBOOT_SRC)
