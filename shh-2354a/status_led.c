// status_led.c

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "status_led.h"

#define LED(X)		(gpio_put(PIN_STATUS_LED,X))

typedef struct {
	bool level;
	uint32_t ms;
} led_step_t;

typedef struct {
	const led_step_t *step;
	uint8_t count;
} led_pattern_t;

typedef struct {
	uint8_t state;
	uint8_t step;
} led_current_action_t ;

static const led_step_t off[] =
{
	{false, 0}
};
static const led_step_t idle[] =
{
	{true, 0}
};
static const led_step_t loading[] =
{
	{false, 200}, {true, 200}
};
static const led_step_t error[] =
{
	{false, 100}, {true, 100}
};
static const led_step_t alive[] =
{
	{false, 4950}, {true, 50}
};

/*
	STATE_OFF = 0,
	STATE_IDLE = 1,
	STATE_LOADING = 2,
	STATE_ERROR = 3,
	STATE_ALIVE = 4
*/
static const led_pattern_t patterns[LED_STATE_COUNT] =
{
	[LED_STATE_OFF] 	= {off, 	1},
	[LED_STATE_IDLE]	= {idle, 	1},
	[LED_STATE_LOADING]	= {loading,	2},
	[LED_STATE_ERROR]	= {error,	2},
	[LED_STATE_ALIVE] 	= {alive,	2},
};

static led_current_action_t current;
static alarm_id_t led_alarm_id;

void init_status_led(void)
{
	gpio_init(PIN_STATUS_LED);
	gpio_set_dir(PIN_STATUS_LED, GPIO_OUT);
	LED(false);
	led_alarm_id = 0;
	set_led_state(LED_STATE_OFF);
	return;
}

static int64_t status_led_alarm(alarm_id_t id, void *user_data)
{
	const led_pattern_t *pattern = &patterns[current.state];
	const led_step_t *step = &pattern->step[current.step];
	LED(step->level);

	current.step = (current.step + 1) % pattern->count;

	return (int64_t)step->ms * 1000;
}

void set_led_state(uint8_t state)
{
	if (state < LED_STATE_COUNT && current.state != state)
	{
		if (led_alarm_id)
			cancel_alarm(led_alarm_id);
		current.state = state;
		current.step = 0;
		led_alarm_id = add_alarm_in_ms(1, status_led_alarm, NULL, false);
	}
	return;
}
