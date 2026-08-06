// inc/status_led.h

#ifndef STATUS_LED_H
#define STATUS_LED_H

#define PIN_STATUS_LED		22
#define LED_STATE_COUNT		5

#define LED_STATE_OFF		0
#define LED_STATE_IDLE		1
#define LED_STATE_LOADING	2
#define LED_STATE_ERROR 	3
#define LED_STATE_ALIVE 	4

void init_status_led(void);
void set_led_state(uint8_t state);

#endif