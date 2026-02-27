#ifndef __UI_CALI__
#define __UI_CALI__

void *ui_cali_start(void (*touchcallback)(UI_CALI_EVENT event));
void caliCallback(UI_CALI_EVENT event);
void readCaliRoof(void);
void uiCaliScreen(void);
#endif
