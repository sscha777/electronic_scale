#ifndef __UI_FAIL__
#define __UI_FAIL__

void *ui_fail_start(void (*touchcallback)(UI_FAIL_EVENT event));
void failWeightCallback(UI_FAIL_EVENT event);
void ui_msg_job_complete(void);
void ui_fail_screen(void);
void readFailWeight(void);

#endif
