// inc/usbc.h

#ifndef USBC_H
#define USBC_H

typedef enum {
	USBC_UNKNOWN,
	USBC_DETACHED,
	USBC_ATTACHED
} usbc_state_t;

typedef enum {
	USB_500MA,
	USB_1500MA,
	USB_3000MA
} usbc_current_t;

void init_usbc(void);
usbc_state_t usbc_get_state(void);
usbc_current_t usbc_get_current(void);
void usbc_handle(void);
void usbc_cleanup(void);

#endif