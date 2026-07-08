# toolchain/Makefile

include ../env.mk

STAMP 			:= $(PROJECT_ROOT)toolchain/stamp
SRC 			:= $(PROJECT_ROOT)toolchain/crosstool-ng

all: xtools

$(STAMP)/.ct-ng:
	mkdir -p $(STAMP)
	git clone $(GIT_URL_CT_NG)
	cd $(SRC) && git checkout $(VERSION_CT_NG)
	cd $(SRC) && ./bootstrap
	cd $(SRC) && ./configure --enable-local
	$(MAKE) -C $(SRC)
	touch $@

ct-ng: $(STAMP)/.ct-ng

$(SRC)/.config: $(STAMP)/.ct-ng $(CT_NG_CONFIG)
	cp $(CT_NG_CONFIG) $(SRC)/.config
	cd $(SRC) && ./ct-ng olddefconfig

set_config: $(SRC)/.config

menuconfig: $(SRC)/.config
	cd $(SRC) && ./ct-ng menuconfig

save_config: $(SRC)/.config
	cp $(SRC)/.config $(CT_NG_CONFIG)

$(STAMP)/.xtools: $(SRC)/.config
	mkdir -p $(CT_NG_XTOOLS) $(CT_NG_DL)
	cd $(SRC) && ./ct-ng build
	touch $@

xtools: $(STAMP)/.xtools

clean:
	rm -rf $(STAMP) $(SRC) $(CT_NG_XTOOLS) $(CT_NG_DL)
