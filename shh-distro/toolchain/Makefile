# toolchain/Makefile

include ../env.mk

CTNG_SRC 			:= $(DL)/crosstool-ng
CTNG_STAMP			:= $(TOOLCHAIN_ROOT)/.stamp.ready
CTNG_DOT_CONFIG		:= $(CTNG_SRC)/.config
XTOOLS_STAMP		:= $(TOOLCHAIN_ROOT)/.stamp.xtools

.PHONY: all
all: $(XTOOLS_STAMP)

$(CTNG_STAMP):
	mkdir -p $(DL)
	git clone --branch $(CTNG_VERSION) \
		$(CTNG_GIT_URL) $(CTNG_SRC)
	cd $(CTNG_SRC) && ./bootstrap
	cd $(CTNG_SRC) && ./configure --enable-local
	$(MAKE) -C $(CTNG_SRC)
	touch $@

$(CTNG_DOT_CONFIG): $(CTNG_STAMP) $(CTNG_CONFIG)
	cp $(CTNG_CONFIG) $@
	cd $(CTNG_SRC) && ./ct-ng olddefconfig

$(XTOOLS_STAMP): $(CTNG_DOT_CONFIG)
	mkdir -p $(XTOOLS)
	cd $(CTNG_SRC) && ./ct-ng build
	touch $@

.PHONY: menuconfig
menuconfig: $(CTNG_DOT_CONFIG)
	cd $(CTNG_SRC) && ./ct-ng menuconfig

.PHONY: saveconfig
saveconfig: $(CTNG_DOT_CONFIG)
	cp $(CTNG_DOT_CONFIG) $(CTNG_CONFIG)

.PHONY: clean
clean:
	rm -rf $(XTOOLS) $(XTOOLS_STAMP)

.PHONY: distclean
distclean:
	rm -rf $(CTNG_SRC) $(XTOOLS) $(CTNG_STAMP) $(XTOOLS_STAMP)
