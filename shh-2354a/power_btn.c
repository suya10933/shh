// power_btn.c

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "power_btn.h"

#define DEBOUNCE_DELAY_MS	10
#define LONG_DELAY_MS	1000

static volatile bool power_btn_pressed;

static absolute_time_t next_handle;

static power_btn_event_t power_btn_event = POWER_BTN_NONE; 

void init_power_btn(void)
{
	gpio_init(PIN_POWER_BTN);
	gpio_set_dir(PIN_POWER_BTN, GPIO_IN);
	power_btn_event = POWER_BTN_NONE;

	return;
}

bool power_btn_get(void)
{
	return (gpio_get(PIN_POWER_BTN) == 0);
}

power_btn_event_t power_btn_get_event(void)
{
	power_btn_event_t ret = power_btn_event;
	power_btn_event = POWER_BTN_NONE;
	return ret;
}

void power_btn_handle(void)
{
	if (!time_reached(next_handle))
		return;

	next_handle = make_timeout_time_ms(2);

	static bool last_btn = false;
	static bool curr_btn = false;
	static bool is_pressed = false;

	static bool short_flag = false;

	static absolute_time_t short_timer = 0;
	static absolute_time_t long_timer = 0;
	curr_btn = (gpio_get(PIN_POWER_BTN) == false);

	if (last_btn == false && curr_btn == true &&
		is_pressed == false)
	{
		is_pressed = true;
		short_timer = make_timeout_time_ms(DEBOUNCE_DELAY_MS);
		long_timer = make_timeout_time_ms(LONG_DELAY_MS);
	}

	if (last_btn == true && curr_btn == false &&
		is_pressed == true)
	{
		is_pressed = false;
		if (short_flag)
		{
			power_btn_event = POWER_BTN_SHORT;
			short_flag = false;
		}
	}

	if (is_pressed)
	{
		if (time_reached(short_timer))
		{
			short_flag = true;
		}
		if (time_reached(long_timer))
		{
			short_flag = false;
			power_btn_event = POWER_BTN_LONG;
			is_pressed = false;
		}
	}

	last_btn = curr_btn;
}