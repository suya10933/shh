# env.mk

PROJECT_ROOT := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
export PROJECT_ROOT

TARGET 			?= aarch64-shh-linux-gnu
ARCH 			?= arm64
OUT				?= $(PROJECT_ROOT)out
RELEASE			?= $(OUT)/release
DEBUG 			?= $(OUT)/debug
RELEASE_BOOT	?= $(RELEASE)/boot
DEBUG_BOOT		?= $(DEBUG)/boot
IMAGES			?= $(OUT)/images

GIT_URL_CT_NG	?= https://github.com/crosstool-ng/crosstool-ng.git
VERSION_CT_NG	?= crosstool-ng-1.28.0
CT_NG			?= $(SRC)/ct-ng
CT_NG_CONFIG	?= $(PROJECT_ROOT)toolchain/.config
CT_NG_XTOOLS 	?= $(PROJECT_ROOT)toolchain/x-tools
CT_NG_DL		?= $(PROJECT_ROOT)toolchain/dl
CROSS_COMPILE 	?= $(CT_NG_XTOOLS)/$(TARGET)/bin/$(TARGET)-
export CT_NG_XTOOLS CT_NG_DL CROSS_COMPILE

FW				?= $(PROJECT_ROOT)firmware
FW_DL			?= $(FW)/dl
FW_URL 			?= https://raw.githubusercontent.com/raspberrypi/firmware/master/boot

KERNEL_SRC 		?= $(PROJECT_ROOT)kernel/linux
KERNEL_CONFIG	?= shh_defconfig
KERNEL_BUILD 	?= $(PROJECT_ROOT)kernel/build
KERNEL_DEBUG_CONFIG ?= $(PROJECT_ROOT)kernel/configs/debug.config
KERNEL_BUILD_DEBUG 	?= $(PROJECT_ROOT)kernel/build-debug
