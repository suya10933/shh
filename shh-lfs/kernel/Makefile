# kernel/Makefile

include ../env.mk

KERNEL_OUT			:= $(BUILD_OUT)/kernel
KERNEL_DOT_CONFIG	:= $(KERNEL_OUT)/.config
KERNEL_SRC 			:= $(PROJECT_ROOT)kernel/linux

KERNEL_DEFCONFIG	:= $(KERNEL_SRC)/arch/$(ARCH)/configs/$(KERNEL_CONFIG)
KERNEL_IMAGE		:= $(KERNEL_OUT)/arch/$(ARCH)/boot/Image
KERNEL_DTB 			:= $(KERNEL_OUT)/arch/$(ARCH)/boot/dts/broadcom/bcm2711-rpi-cm4.dtb
KERNEL_OVERLAYS		:= $(KERNEL_OUT)/arch/$(ARCH)/boot/dts/overlays

KERNEL_MAKE 		:= $(MAKE) -C $(KERNEL_SRC) \
						ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
						O=$(KERNEL_OUT)

CONFIG_DEPS			:= $(KERNEL_DEFCONFIG)

ifeq ($(PROFILE),debug)
CONFIG_DEPS 		+= $(KERNEL_DEBUG_FRAG)
endif

.PHONY: all
all: build

$(KERNEL_DOT_CONFIG): $(CONFIG_DEPS)
	mkdir -p $(KERNEL_OUT)
	$(KERNEL_MAKE) $(KERNEL_CONFIG)
ifeq ($(PROFILE),debug)
	$(KERNEL_SRC)/scripts/kconfig/merge_config.sh \
		-m -O $(KERNEL_OUT) $(KERNEL_DOT_CONFIG) $(KERNEL_DEBUG_FRAG)
	$(KERNEL_MAKE) olddefconfig
endif

.PHONY: build
build: $(KERNEL_DOT_CONFIG)
	$(KERNEL_MAKE) Image dtbs

.PHONY: menuconfig
menuconfig: $(KERNEL_DOT_CONFIG)
	$(KERNEL_MAKE) menuconfig

.PHONY: install
install: build
ifeq ($(PROFILE),debug)
	install -D -m 0644 \
		$(KERNEL_IMAGE) \
		$(DEBUG_NFS)/Image

	install -D -m 0644 \
		$(KERNEL_DTB) \
		$(DEBUG_NFS)/bcm2711-rpi-cm4.dtb

	mkdir -p $(DEBUG_NFS)/overlays
	install -m 0644 \
		$(KERNEL_OVERLAYS)/*.dtbo \
		$(DEBUG_NFS)/overlays/

	install -D -m 0644 \
		$(KERNEL_DTB) \
		$(BOOT_OUT)/bcm2711-rpi-cm4.dtb

	install -D -m 0644 \
		$(KERNEL_OVERLAYS)/dwc2.dtbo \
		$(BOOT_OUT)/overlays/dwc2.dtbo
else
	install -D -m 0644 \
		$(KERNEL_IMAGE) \
		$(BOOT_OUT)/kernel8.img

	install -D -m 0644 \
		$(KERNEL_DTB) \
		$(BOOT_OUT)/bcm2711-rpi-cm4.dtb

	mkdir -p $(BOOT_OUT)/overlays
	install -m 0644 \
		$(KERNEL_OVERLAYS)/*.dtbo \
		$(BOOT_OUT)/overlays/
endif

.PHONY: clean
clean:
	rm -rf $(KERNEL_OUT)
