# firmware/Makefile

include ../env.mk

.PHONY: all
all: $(DL)/start4.elf $(DL)/fixup4.dat

$(DL)/start4.elf:
	mkdir -p $(DL)
	curl -L --fail --retry 3 -o $@ $(FW_URL)/start4.elf

$(DL)/fixup4.dat:
	mkdir -p $(DL)
	curl -L --fail --retry 3 -o $@ $(FW_URL)/fixup4.dat

.PHONY: install
install: $(DL)/fixup4.dat $(DL)/start4.elf
	mkdir -p $(BOOT_OUT)
ifeq ($(PROFILE),debug)
	cp $(FW_ROOT)/config_debug.txt \
		$(BOOT_OUT)/config.txt
else
	cp $(FW_ROOT)/config_release.txt \
		$(BOOT_OUT)/config.txt
	echo $(BOOTARGS) > $(BOOT_OUT)/cmdline.txt
endif
	cp $(DL)/fixup4.dat \
		$(BOOT_OUT)/
	cp $(DL)/start4.elf \
		$(BOOT_OUT)/

.PHONY: clean
clean:
	rm -rf $(DL)/start4.elf $(DL)/fixup4.dat
