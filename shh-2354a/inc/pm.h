// inc/pm.h

#ifndef PM_H
#define PM_H

typedef enum {
	USBC_UNDEF,
	USBC_UNKNOWN,
	USBC_DETACHED,
	USBC_ATTACHED
} pm_state_t;

typedef enum {
	USB_500MA,
	USB_1500MA,
	USB_3000MA
} pm_current_t;

void init_pm(void);
pm_state_t pm_get_state(void);
pm_current_t pm_get_current(void);
void pm_handle(void);
void pm_cleanup(void);

#endif