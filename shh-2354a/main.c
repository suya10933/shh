// main.c

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/stdio_uart.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"

#include "status_led.h"
#include "power_btn.h"

#define PIN_POWER_HOLD		11
#define PIN_5V_PRE_EN		9
#define PIN_5V_EN 			10

#define PIN_DEBUG_UART_TX	12

static volatile bool shutdown = false;

void configure_clock(void)
{
	clock_stop(clk_gpout0);
	clock_stop(clk_gpout1);
	clock_stop(clk_gpout2);
	clock_stop(clk_gpout3);
	clock_stop(clk_usb);
	clock_stop(clk_adc);
	clock_stop(clk_hstx);

	pll_deinit(pll_usb);
	return;
}

void power_on(void)
{
	gpio_init(PIN_5V_PRE_EN);
	gpio_set_dir(PIN_5V_PRE_EN, GPIO_OUT);
	gpio_init(PIN_5V_EN);
	gpio_set_dir(PIN_5V_EN, GPIO_OUT);
	gpio_init(PIN_POWER_HOLD);
	gpio_set_dir(PIN_POWER_HOLD, GPIO_OUT);

	sleep_ms(500);

	gpio_put(PIN_POWER_HOLD, 1);
	gpio_put(PIN_5V_PRE_EN, 1);
	sleep_ms(100);
	gpio_put(PIN_5V_EN, 1);
	return;
}

void power_btn(void)
{
	return;
}

void shutdown_handle(void)
{
	shutdown = true;
	return;
}

int main(void)
{
	init_status_led();
	power_on();
	configure_clock();

	stdio_uart_init_full(uart0, 115200, PIN_DEBUG_UART_TX, -1);

	init_power_btn(power_btn, shutdown_handle);

	//printf("clk_ref : %lu\n", (uint32_t)clock_get_hz(clk_ref));
	//printf("clk_sys : %lu\n", (uint32_t)clock_get_hz(clk_sys));
	//printf("clk_peri : %lu\n", (uint32_t)clock_get_hz(clk_peri));

	set_led_state(LED_STATE_LOADING);

	for (;;)
	{
		if (shutdown)
		{
			set_led_state(LED_STATE_OFF);
			gpio_put(PIN_POWER_HOLD, 0);
			for (;;) {;}
		}
	}

	return 0;
}
