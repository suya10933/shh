# firmware/Makefile

include ../env.mk

all: $(FW_DL)/start4.elf $(FW_DL)/fixup4.dat

$(FW_DL):
	mkdir -p $@

$(FW_DL)/start4.elf: $(FW_DL)
	curl -L --fail --retry 3 -o $@ $(FW_URL)/start4.elf

$(FW_DL)/fixup4.dat: $(FW_DL)
	curl -L --fail --retry 3 -o $@ $(FW_URL)/fixup4.dat

.PHONY: install
install: $(FW_DL)/fixup4.dat $(FW_DL)/start4.elf
	mkdir -p $(RELEASE_BOOT)
	cp $(FW_ROOT)/cmdline.txt \
		$(RELEASE_BOOT)/
	cp $(FW_ROOT)/config.txt \
		$(RELEASE_BOOT)/
	cp $(FW_DL)/fixup4.dat \
		$(RELEASE_BOOT)/
	cp $(FW_DL)/start4.elf \
		$(RELEASE_BOOT)/
	mkdir -p $(DEBUG_BOOT)
	cp $(FW_ROOT)/config_debug.txt \
		$(DEBUG_BOOT)/config.txt
	cp $(FW_DL)/fixup4.dat \
		$(DEBUG_BOOT)/
	cp $(FW_DL)/start4.elf \
		$(DEBUG_BOOT)/

clean:
	rm -rf $(FW_DL)
