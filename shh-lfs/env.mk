# env.mk

PROJECT_ROOT := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
export PROJECT_ROOT

TARGET 			?= aarch64-shh-linux-gnu
ARCH 			?= arm64
CROSS_COMPILE 	?= $(TARGET)-

GIT_URL_CT_NG	?= https://github.com/crosstool-ng/crosstool-ng.git
VERSION_CT_NG	?= crosstool-ng-1.28.0
CT_NG			?= $(SRC)/ct-ng
CT_NG_CONFIG	?= $(PROJECT_ROOT)toolchain/.config
CT_NG_XTOOLS 	?= $(PROJECT_ROOT)toolchain/x-tools
CT_NG_DL		?= $(PROJECT_ROOT)toolchain/dl
export CT_NG_XTOOLS CT_NG_DL

FW_DL			?= $(PROJECT_ROOT)firmware/dl
FW_URL 			?= https://raw.githubusercontent.com/raspberrypi/firmware/master/boot

