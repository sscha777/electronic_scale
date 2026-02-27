/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "sntp.h"
#include "eve/EVE_target.h"
#include "sntp.h"
#include "scspiffs.h"
#include "esp_timer.h"
#include "http.h"

struct tm timeinfo;
static esp_timer_handle_t timer = NULL;
#define CHECK(x)                \
    do                          \
    {                           \
        esp_err_t __;           \
        if ((__ = x) != ESP_OK) \
            return __;          \
    } while (0)
#define CHECK_ARG(VAL)                  \
    do                                  \
    {                                   \
        if (!(VAL))                     \
            return ESP_ERR_INVALID_ARG; \
    } while (0)

#define CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH 0
static const char *TAG = "sntp";

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int boot_count = 0;

// static void obtain_time(void);
//static void initialize_sntp(void);

void time_sync_notification_cb(struct timeval *tv)
{
    //ESP_LOGI(TAG, "Notification of a time synchronization event");
}

struct tm scGetTime(void)
{
    return timeinfo;
}

void scSetTime(struct tm sTime)
{
    timeinfo.tm_sec = sTime.tm_sec;
    timeinfo.tm_min = sTime.tm_min;
    timeinfo.tm_hour = sTime.tm_hour;
}

void scSetDay(struct tm sTime)
{
    timeinfo.tm_mday = sTime.tm_mday;
    timeinfo.tm_mon = sTime.tm_mon;
    timeinfo.tm_year = sTime.tm_year;
}
#define TIME_1SEC 1000 * 1000 // 1sec

static void time_count(void *arg)
{
    // static uint32_t test_count = 0;
    // test_count++;
    // //ESP_LOGI(TAG, "timer test %d", test_count);

    timeinfo.tm_sec++;
    if (timeinfo.tm_sec >= 60)
    {
        timeinfo.tm_sec = 0;
        timeinfo.tm_min++;
        if (timeinfo.tm_min >= 60)
        {
            timeinfo.tm_min = 0;
            timeinfo.tm_hour++;
            if (timeinfo.tm_hour >= 24)
            {
                timeinfo.tm_hour = 0;
                timeinfo.tm_mday++;
            }
        }
    }
}

static const esp_timer_create_args_t timer_args = {
    .arg = NULL,
    .name = "_1sec",
    .dispatch_method = ESP_TIMER_TASK,
    .callback = time_count,
};

esp_err_t time_count_init(void)
{
    if (!timer)
        CHECK(esp_timer_create(&timer_args, &timer));

    esp_timer_stop(timer);
    esp_err_t res = ESP_ERR_NO_MEM;

    CHECK(esp_timer_start_periodic(timer, TIME_1SEC));
    return res;
}

void sntp_task(void *pvParameter)
{
    static char delay_count = 0;

    ++boot_count;
    //ESP_LOGI(TAG, "Boot count: %d", boot_count);
    time_count_init();

    while (1)
    {
        if (scaleInfo.wifi_status >= 1)
        {
            ////ESP_LOGE(TAG, "wifi connected!\n");
            break;
        }
        else
        {
            delay_count++;
            if (delay_count > 20)
            {
                ////ESP_LOGE(TAG, "wifi is not connect!\n");
                break;
            }
            printf(".");
            DELAY_MS(500);
        }
        DELAY_MS(500);
    }

    time_t now;

    time(&now);
    localtime_r(&now, &timeinfo);
#if 0
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900))
    {
        //ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    else
    {
        // add 500 ms error to the current system time.
        // Only to demonstrate a work of adjusting method!
        {
            //ESP_LOGI(TAG, "Add a error for test adjtime");
            struct timeval tv_now;
            gettimeofday(&tv_now, NULL);
            int64_t cpu_time = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
            int64_t error_time = cpu_time + 500 * 1000L;
            struct timeval tv_error = {.tv_sec = error_time / 1000000L, .tv_usec = error_time % 1000000L};
            settimeofday(&tv_error, NULL);
        }

        //ESP_LOGI(TAG, "Time was set, now just adjusting it. Use SMOOTH SYNC method.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
#endif

#endif

#if 0
    char strftime_buf[64];
    // Set timezone to Eastern Standard Time and print local time
    // Set timezone to China Standard Time
    setenv("TZ", "UTC-9", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    //ESP_LOGI(TAG, "The current date/time in south korea is: %s", strftime_buf);

    taskInfo.timeServerMode = TIME_SERVER_SNTP;

    //taskInfo.timeServerMode = TIME_SERVER_NOT_FOUND; // debug

    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH)
    {
        struct timeval outdelta;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS)
        {
            adjtime(NULL, &outdelta);
            //ESP_LOGI(TAG, "Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us",
                     (long)outdelta.tv_sec,
                     outdelta.tv_usec / 1000,
                     outdelta.tv_usec % 1000);
            DELAY_MS(2000);
        }
    }
#endif

    while (1)
    {
        DELAY_MS(2000);
    }
}

#if 0
static void obtain_time(void)
{
    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 5;
    DELAY_MS(2000);
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET)
    {
        if (retry++ > retry_count)
        {
            taskInfo.timeServerMode = TIME_SERVER_NOT_FOUND;
            //ESP_LOGI(TAG, "sntp server error! (%d/%d)", retry, retry_count);
        }
        else
        {
            //ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        }
        DELAY_MS(2000);
    }

    time(&now);
    localtime_r(&now, &timeinfo);
}

static void initialize_sntp(void)
{
    //ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    sntp_setservername(0, "pool.ntp.org");

    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);

    sntp_init();
}
#endif 