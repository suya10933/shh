// main.c

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/stdio_uart.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/sync.h"

#include "status_led.h"
#include "power_btn.h"
#include "usbc.h"

#define PIN_POWER_HOLD		11
#define PIN_5V_PRE_EN		9
#define PIN_5V_EN 			10

#define PIN_DEBUG_UART_TX	12

typedef enum {
	CM4_OFF,
	CM4_POWERED,
	CM4_SHUTDOWN_WAIT
} cm4_state_t;

typedef struct {
	cm4_state_t cm4_state;
} state_t;

static state_t state;

static void configure_clock(void)
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

static void cm4_on(void)
{
	gpio_put(PIN_5V_PRE_EN, 1);
	sleep_ms(100);
	gpio_put(PIN_5V_EN, 1);
	return;
}

static void cm4_off(void)
{
	gpio_put(PIN_5V_EN, 0);
	sleep_ms(100);
	gpio_put(PIN_5V_PRE_EN, 0);
	return;
}

static void power_hold(void)
{
	gpio_init(PIN_POWER_HOLD);
	gpio_put(PIN_POWER_HOLD, 1);
	gpio_set_dir(PIN_POWER_HOLD, GPIO_OUT);
	return;
}

static void power_release(void)
{
	usbc_cleanup();
	set_led_state(LED_STATE_OFF);
	gpio_put(PIN_POWER_HOLD, 0);
	for (;;)
		__wfi();
	return;
}

int main(void)
{
	power_hold();
	init_power_btn();
	init_status_led();
	configure_clock();
	init_usbc();

	stdio_uart_init_full(uart0, 115200, PIN_DEBUG_UART_TX, -1);

	gpio_init(PIN_5V_PRE_EN);
	gpio_put(PIN_5V_PRE_EN, 0);
	gpio_set_dir(PIN_5V_PRE_EN, GPIO_OUT);
	gpio_init(PIN_5V_EN);
	gpio_put(PIN_5V_EN, 0);
	gpio_set_dir(PIN_5V_EN, GPIO_OUT);

	if (power_btn_get())
	{
		cm4_on();
		state.cm4_state = CM4_POWERED;
		set_led_state(LED_STATE_LOADING);
	}
	else
	{
		state.cm4_state = CM4_OFF;
		set_led_state(LED_STATE_ALIVE);
	}


	power_btn_event_t power_btn_event = POWER_BTN_NONE;
	usbc_state_t usbc_state = USBC_UNKNOWN;

	for (;;)
	{
		usbc_handle();
		power_btn_handle();

		power_btn_event = power_btn_get_event();
		usbc_state = usbc_get_state();

		switch(state.cm4_state)
		{
		case CM4_OFF :
			if (power_btn_event == POWER_BTN_SHORT ||
				power_btn_event == POWER_BTN_LONG)
			{
				cm4_on();
				state.cm4_state = CM4_POWERED;
				set_led_state(LED_STATE_LOADING);
			} else if (usbc_state == USBC_DETACHED)
			{
				power_release();
			}
			break;
		case CM4_POWERED :
			if (power_btn_event == POWER_BTN_SHORT)
			{
				// pass cm4 shutdown signal;
			} else if (power_btn_event == POWER_BTN_LONG)
			{
				cm4_off();
				state.cm4_state = CM4_OFF;
				set_led_state(LED_STATE_OFF);
			}
			break;
		case CM4_SHUTDOWN_WAIT :
			// 
			break;
		}


		sleep_ms(1);
	}

	return 0;
}
