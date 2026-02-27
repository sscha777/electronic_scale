
#ifndef __SNTP_H__
#define __SNTP_H__

void sntp_task(void *pvParameter);
void time_sync_notification_cb(struct timeval *tv);
struct tm scGetTime(void);
void scSetTime(struct tm sTime);
void scSetTime(struct tm sTime);
void scSetDay(struct tm sTime);

#endif
