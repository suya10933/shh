# CMakeLists.txt

cmake_minimum_required(VERSION 3.13...3.27)

set(PICO_BOARD none)
set(PICO_PLATFORM rp2350-arm-s)
set(PICO_FLASH_SIZE_BYTES 2097152)

set(PICO_CONFIG_HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/rp2354a_config.h)

include(pico-sdk/pico_sdk_init.cmake)

project(shh-2354a)

pico_sdk_init()

add_executable(shh-2354a
	main.c
	status_led.c
	power_btn.c
)

target_link_libraries(shh-2354a
	pico_stdlib
	pico_stdio_uart
	hardware_clocks
	hardware_pll
)

target_include_directories(shh-2354a
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/inc
)

pico_enable_stdio_usb(shh-2354a 0)
pico_enable_stdio_uart(shh-2354a 0)

pico_add_extra_outputs(shh-2354a)