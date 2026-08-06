# u-boot/Makefile

include ../env.mk

UBOOT_OUT			:= $(BUILD_OUT)/u-boot
UBOOT_DOT_CONFIG	:= $(UBOOT_OUT)/.config
UBOOT_SRC			:= $(DL)/u-boot

UBOOT_MAKE			:= $(MAKE) -C $(UBOOT_SRC) \
						ARCH=$(UBOOT_ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
						O=$(UBOOT_OUT)

.PHONY: all
all: build

$(UBOOT_SRC):
	mkdir -p $(DL)
	git clone $(UBOOT_GIT_URL) $(UBOOT_SRC)
	cd $(UBOOT_SRC) && git checkout $(UBOOT_VERSION)

$(UBOOT_DOT_CONFIG) : $(UBOOT_CONFIG) $(UBOOT_SRC) $(PROJECT_ROOT)env.mk
	mkdir -p $(UBOOT_OUT)
	cp $(UBOOT_CONFIG) $@
	$(UBOOT_SRC)/scripts/config \
		--file $(UBOOT_OUT)/.config \
		--enable USE_BOOTCOMMAND \
		--set-str BOOTCOMMAND '$(UBOOT_BOOTCMD)'
	$(UBOOT_SRC)/scripts/config \
		--file $(UBOOT_OUT)/.config \
		--enable USE_BOOTARGS \
		--set-str BOOTARGS '$(UBOOT_BOOTARGS)'
	$(UBOOT_MAKE) olddefconfig

.PHONY: menuconfig
menuconfig: $(UBOOT_DOT_CONFIG)
	$(UBOOT_MAKE) menuconfig

.PHONY: saveconfig
saveconfig: $(UBOOT_DOT_CONFIG)
	cp $(UBOOT_DOT_CONFIG) $(UBOOT_CONFIG)

.PHONY: build
build: $(UBOOT_DOT_CONFIG)
	$(UBOOT_MAKE)

.PHONY: install
install : build
	install -D -m 0644 \
		$(UBOOT_OUT)/u-boot.bin \
		$(BOOT_OUT)/u-boot.bin

.PHONY: clean
clean:
	rm -rf $(UBOOT_OUT)