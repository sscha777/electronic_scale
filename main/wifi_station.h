
#ifndef __WIFI__
#define __WIFI__

//#define EXAMPLE_ESP_WIFI_SSID      "sscnet"
//#define EXAMPLE_ESP_WIFI_PASS      "11111112"

#define DEFAULT_SCAN_LIST_SIZE 12

#define EXAMPLE_ESP_WIFI_SSID "iptime\0"
#define EXAMPLE_ESP_WIFI_PASS "01052400792\0"

enum WIFI_SCAN_STEP
{
    WIFI_SCAN_STOP = 0,
    WIFI_SCAN_START,
    WIFI_SCAN_SCANNING,
    WIFI_SCAN_END
};

typedef struct _WIFI_SCAN_
{
    uint8_t wifi_scanStep; /* 0: scan stop 1: scanstart 2: scannign 3: scanend */
    uint16_t wifi_ap_count;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];

} WIFI_SCAN_INFO;

void wifiInit(void);
void wifiTask(void *pvParameter);

void wifi_scan(void);



#endif
