# env.mk

PROJECT_ROOT := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

OUT 			?= $(PROJECT_ROOT)out
DL				?= $(PROJECT_ROOT)dl

TARGET 			?= aarch64-shh-linux-gnu
ARCH 			?= arm64
CROSS_COMPILE 	?= $(TARGET)-

CTNG_PREFIX 	?= $(OUT)/ct-ng
XTOOLS 			?= $(OUT)/x-tools
TOOLCHAIN_DIR 	?= $(XTOOLS)/$(TARGET)

export PROJECT_ROOT
export OUT DL TARGET ARCH CROSS_COMPILE CTNG_PREFIX XTOOLS TOOLCHAIN_DIR
