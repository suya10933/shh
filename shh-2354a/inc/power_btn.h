// inc/power_btn.h

#ifndef POWER_BTN_H
#define POWER_BTN_H

#define PIN_POWER_BTN		23

void init_power_btn(void (*_power_btn_callback)(void),
	void (*_shutdown_callback)(void));

#endif