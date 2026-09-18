// inc/power_btn.h

#ifndef POWER_BTN_H
#define POWER_BTN_H

#include <stdbool.h>

#define PIN_POWER_BTN		23

typedef enum {
	POWER_BTN_NONE,
	POWER_BTN_SHORT,
	POWER_BTN_LONG
} power_btn_event_t;

void init_power_btn(void);
bool power_btn_get(void);
power_btn_event_t power_btn_get_event(void);
void power_btn_handle(void);

#endif