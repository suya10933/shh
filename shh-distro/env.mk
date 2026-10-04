# env.mk

PROJECT_ROOT := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
export PROJECT_ROOT

TARGET 				?= aarch64-shh-linux-gnu
ARCH 				?= arm64
OUT					?= $(PROJECT_ROOT)out
PROFILE 			?= release
PROFILE_OUT			:= $(OUT)/$(PROFILE)
BUILD_OUT 			:= $(PROFILE_OUT)/build
BOOT_OUT			:= $(PROFILE_OUT)/boot
DL 					:= $(PROJECT_ROOT)dl
export OUT

VALID_PROFILES		:= release debug

ifeq ($(filter $(PROFILE),$(VALID_PROFILES)),)
$(error Invalid PROFILE)
endif

ifeq ($(PROFILE),debug)

DEBUG_NFS			:= $(PROFILE_OUT)/nfs
ROOTFS				:= $(DEBUG_NFS)/rootfs

USER_CPPFLAGS		:= -DDEBUG
USER_CFLAGS 		:= -Og -g3 -fno-omit-frame-pointer

else

ROOTFS				:= $(PROFILE_OUT)/rootfs

USER_CPPFLAGS		:=
USER_CFLAGS 		:= -Os -DNDEBUG

endif

export PROFILE PROFILE_OUT BUILD_OUT BOOT_OUT ROOTFS
export USER_CPPFLAGS USER_CFLAGS

ifeq ($(PROFILE),debug)

BOOTARGS 			:= console=ttyS0,115200n8 earlycon ignore_loglevel loglevel=8 \
						root=/dev/nfs nfsroot=192.168.7.1:$(ROOTFS),vers=3,tcp,nolock \
						ip=192.168.7.2:192.168.7.1::255.255.255.0:cm4:usb0:off \
						g_ether.dev_addr=02:00:00:00:00:02 \
						g_ether.host_addr=02:00:00:00:00:01 \
						rw init=/sbin/shh-init nokaslr

else 

BOOTARGS 			:= root=/dev/mmcblk0p2 rw init=/sbin/shh-init

endif

IMAGES				?= $(OUT)/images

TOOLCHAIN_ROOT		?= $(PROJECT_ROOT)toolchain
CTNG_GIT_URL		?= https://github.com/crosstool-ng/crosstool-ng.git
CTNG_VERSION		?= crosstool-ng-1.28.0
CTNG_CONFIG			?= $(PROJECT_ROOT)toolchain/.config
CTNG_DL				?= $(DL)/ct-ng_dl
XTOOLS 				?= $(TOOLCHAIN_ROOT)/x-tools
CROSS_COMPILE 		?= $(XTOOLS)/$(TARGET)/bin/$(TARGET)-
export XTOOLS CTNG_DL CROSS_COMPILE

FW_ROOT				?= $(PROJECT_ROOT)firmware
FW_URL 				?= https://raw.githubusercontent.com/raspberrypi/firmware/master/boot

UBOOT_ROOT			?= $(PROJECT_ROOT)u-boot
UBOOT_GIT_URL		?= https://github.com/u-boot/u-boot
UBOOT_VERSION		?= v2026.07
UBOOT_CONFIG		?= $(UBOOT_ROOT)/.config
UBOOT_ARCH			?= arm
UBOOT_BOOTCMD		?= bind usb_gadget 0 usb_ether; \
						setenv ethact usb_ether; \
						setenv ethprime usb_ether; \
						setenv ethrotate no; \
						setenv ipaddr1 192.168.7.2; \
						setenv netmask1 255.255.255.0; \
						setenv gatewayip1 0.0.0.0; \
						setenv serverip 192.168.7.1; \
						setenv nfsserverip 192.168.7.1; \
						setenv fdtoverlay_addr_r 0x05800000; \
						nfs $${kernel_addr_r} $${serverip}:$(DEBUG_NFS)/Image; \
						nfs $${fdt_addr_r} $${serverip}:$(DEBUG_NFS)/bcm2711-rpi-cm4.dtb; \
						fdt addr $${fdt_addr_r}; \
						fdt resize 0x10000; \
						nfs $${fdtoverlay_addr_r} $${serverip}:$(DEBUG_NFS)/overlays/uart1.dtbo; \
			  			fdt apply $${fdtoverlay_addr_r}; \
						nfs $${fdtoverlay_addr_r} $${serverip}:$(DEBUG_NFS)/overlays/dwc2.dtbo; \
						fdt apply $${fdtoverlay_addr_r}; \
						booti $${kernel_addr_r} - $${fdt_addr_r};
UBOOT_BOOTARGS		?= $(BOOTARGS)

KERNEL_CONFIG		?= shh_defconfig
KERNEL_DEBUG_FRAG 	?= $(PROJECT_ROOT)kernel/configs/debug.config

USERSPACE_ROOT		?= $(PROJECT_ROOT)userspace
