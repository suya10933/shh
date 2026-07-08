# firmware/Makefile

include ../env.mk

all: $(FW_DL)/start4.elf $(FW_DL)/fixup4.dat

$(FW_DL):
	mkdir -p $@

$(FW_DL)/start4.elf: $(FW_DL)
	curl -L --fail --retry 3 -o $@ $(FW_URL)/start4.elf

$(FW_DL)/fixup4.dat: $(FW_DL)
	curl -L --fail --retry 3 -o $@ $(FW_URL)/fixup4.dat

clean:
	rm -rf $(FW_DL)
