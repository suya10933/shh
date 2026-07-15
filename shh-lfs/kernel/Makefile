#kernel/Makefile

include ../env.mk

all: build build-debug

.PHONY: build
build: 
	mkdir -p $(KERNEL_BUILD)
	$(MAKE) -C $(KERNEL_SRC) \
		ARCH=$(ARCH) \
		O=$(KERNEL_BUILD) \
		$(KERNEL_CONFIG)
	$(MAKE) -C $(KERNEL_SRC) \
		ARCH=$(ARCH) \
		O=$(KERNEL_BUILD) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		Image modules dtbs

.PHONY: build-debug
build-debug:
	mkdir -p $(KERNEL_BUILD_DEBUG)
	$(MAKE) -C $(KERNEL_SRC) \
		ARCH=$(ARCH) \
		O=$(KERNEL_BUILD_DEBUG) \
		$(KERNEL_CONFIG)
	$(KERNEL_SRC)/scripts/kconfig/merge_config.sh -m \
		-O $(KERNEL_BUILD_DEBUG) \
		$(KERNEL_BUILD_DEBUG)/.config \
		$(KERNEL_DEBUG_CONFIG)
	$(MAKE) -C $(KERNEL_SRC) \
		ARCH=$(ARCH) \
		O=$(KERNEL_BUILD_DEBUG) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		olddefconfig
	$(MAKE) -C $(KERNEL_SRC) \
		ARCH=$(ARCH) \
		O=$(KERNEL_BUILD_DEBUG) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		Image modules dtbs

.PHONY: install
install:
	mkdir -p $(RELEASE_BOOT)/overlays
	cp $(KERNEL_BUILD)/arch/arm64/boot/Image \
		$(RELEASE_BOOT)/kernel8.img
	cp $(KERNEL_BUILD)/arch/arm64/boot/dts/broadcom/bcm2711-rpi-cm4.dtb \
		$(RELEASE_BOOT)/
	cp $(KERNEL_BUILD)/arch/arm64/boot/dts/overlays/*.dtbo \
		$(RELEASE_BOOT)/overlays/
	mkdir -p $(DEBUG_BOOT)/overlays
	cp $(KERNEL_BUILD_DEBUG)/arch/arm64/boot/Image \
		$(DEBUG_BOOT)/kernel8.img
	cp $(KERNEL_BUILD_DEBUG)/arch/arm64/boot/dts/broadcom/bcm2711-rpi-cm4.dtb \
		$(DEBUG_BOOT)/
	cp $(KERNEL_BUILD_DEBUG)/arch/arm64/boot/dts/overlays/*.dtbo \
		$(DEBUG_BOOT)/overlays/

.PHONY: clean
clean:
	$(MAKE) -C $(KERNEL_SRC) mrproper
	rm -rf $(KERNEL_BUILD) $(KERNEL_BUILD_DEBUG)
