// power_btn.c

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "power_btn.h"

#define DEBOUNCE_DELAY_MS	10
#define SHUTDOWN_DELAY_MS	1000

static alarm_id_t debounce_alarm_id;
static alarm_id_t shutdown_alarm_id;
static volatile bool power_btn_pressed;
static void (*power_btn_callback)(void);
static void (*shutdown_callback)(void);

static int64_t shutdown_alarm(alarm_id_t id, void *user_data)
{
	shutdown_callback();
	return 0;
}

static int64_t debound_alarm(alarm_id_t id, void *user_data)
{
	power_btn_callback();
	shutdown_alarm_id = add_alarm_in_ms(SHUTDOWN_DELAY_MS,
		shutdown_alarm, NULL, false);

	debounce_alarm_id = 0;
	return 0;
}

static void power_btn_irq(uint gpio, uint32_t events)
{
	if (gpio == PIN_POWER_BTN)
	{
		if (events & GPIO_IRQ_EDGE_FALL)
		{
			if (debounce_alarm_id)
				cancel_alarm(debounce_alarm_id);

			debounce_alarm_id = add_alarm_in_ms(DEBOUNCE_DELAY_MS,
				debound_alarm, NULL, false);
		} else if (events & GPIO_IRQ_EDGE_RISE)
		{
			if (debounce_alarm_id)
				cancel_alarm(debounce_alarm_id);
			if (shutdown_alarm_id)
				cancel_alarm(shutdown_alarm_id);
		}
	}
	return;
}

void init_power_btn(void (*_power_btn_callback)(void),
	void (*_shutdown_callback)(void))
{
	debounce_alarm_id = 0;
	power_btn_pressed = false;
	power_btn_callback = _power_btn_callback;
	shutdown_callback = _shutdown_callback;
	gpio_init(PIN_POWER_BTN);
	gpio_set_dir(PIN_POWER_BTN, GPIO_IN);

	gpio_set_irq_enabled_with_callback(PIN_POWER_BTN,
		GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
		true, power_btn_irq);

	return;
}

