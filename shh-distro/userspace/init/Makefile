# userspace/init/Makefile

include ../../env.mk

CC				:= $(CROSS_COMPILE)gcc

BUILD_DIR		:= $(BUILD_OUT)/userspace/init
TARGET 			:= $(BUILD_DIR)/shh-init

SRC				:= init.c

CFLAGS 			+= -Wall -Wextra -Werror $(USER_CFLAGS) $(USER_CPPFLAGS)

LDFLAGS 		+= -static

.PHONY: all
all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

.PHONY: install
install: $(TARGET)
	install -D -m 0755 $(TARGET) $(ROOTFS)/sbin/shh-init

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)