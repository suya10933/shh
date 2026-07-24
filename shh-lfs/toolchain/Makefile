# toolchain/Makefile

include ../env.mk

SRC 			:= $(PROJECT_ROOT)toolchain/crosstool-ng

all: xtools

$(SRC)/ct-ng:
	git clone $(GIT_URL_CT_NG)
	cd $(SRC) && git checkout $(VERSION_CT_NG)
	cd $(SRC) && ./bootstrap
	cd $(SRC) && ./configure --enable-local
	$(MAKE) -C $(SRC)

$(SRC)/.config: $(SRC)/ct-ng $(CT_NG_CONFIG)
	cp $(CT_NG_CONFIG) $(SRC)/.config
	cd $(SRC) && ./ct-ng olddefconfig

setconfig: $(SRC)/.config

menuconfig: $(SRC)/.config
	cd $(SRC) && ./ct-ng menuconfig

saveconfig: $(SRC)/.config
	cp $(SRC)/.config $(CT_NG_CONFIG)

$(PROJECT_ROOT)toolchain/.stamp.xtools: $(SRC)/.config
	mkdir -p $(CT_NG_XTOOLS) $(CT_NG_DL)
	cd $(SRC) && ./ct-ng build
	touch $@

xtools: $(PROJECT_ROOT)toolchain/.stamp.xtools

clean:
	rm -rf $(SRC) $(CT_NG_XTOOLS) $(CT_NG_DL)
