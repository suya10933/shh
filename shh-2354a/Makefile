PROJECT := shh-2354a

BUILD_DIR ?= build
BUILD_TYPE ?= Release
JOBS ?=

CMAKE ?= cmake
OPENOCD ?= openocd

OPENOCD_ADAPTER_CFG ?= $(CURDIR)/debug/ft4232h-rp2354a.cfg
OPENOCD_TARGET_CFG ?= target/rp2350.cfg

CMAKE_CONFIGURE_ARGS ?=
CMAKE_BUILD_ARGS ?=
OPENOCD_ARGS ?=

ELF := $(BUILD_DIR)/$(PROJECT).elf
PARALLEL_ARG := $(if $(strip $(JOBS)),--parallel $(JOBS),--parallel)

.DEFAULT_GOAL := all

.PHONY: all configure build flash clean rebuild help

all: build

configure:
	$(CMAKE) -S "$(CURDIR)" -B "$(BUILD_DIR)" \
		-DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" $(CMAKE_CONFIGURE_ARGS)

build: configure
	$(CMAKE) --build "$(BUILD_DIR)" $(PARALLEL_ARG) $(CMAKE_BUILD_ARGS)

flash: build
	$(OPENOCD) $(OPENOCD_ARGS) \
		-f "$(OPENOCD_ADAPTER_CFG)" \
		-f "$(OPENOCD_TARGET_CFG)" \
		-c 'program "$(abspath $(ELF))" verify reset exit'

clean:
	$(CMAKE) -E remove_directory "$(BUILD_DIR)"