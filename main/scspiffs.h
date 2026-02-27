
#ifndef __SPIFFS__
#define __SPIFFS__
#include <machine/endian.h>
#include "esp_wifi_types.h"
#include "esp_netif_ip_addr.h"

#define FILE_RE_ERR 1
#define FILE_WR_ERR 2

typedef struct _SCALE_ERROR
{
    unsigned char scale_factor_err : 2;
    unsigned char wifi_err         : 2;
    unsigned char overload         : 1;
    unsigned char reseve           : 3;
} SCALE_ERROR;

#define WIFI_AP_NAME_SIZE 32
#define WIFI_AP_PW_SIZE 64

#define W_PLUS_SCALE_MODE 0
#define W_MINUS_SCALE_MODE 1

#define W_DEFAULT_SAMPLING 1

typedef struct _SCALE_INFO
{
    uint32_t id;
    float scale_factor; // hx711 scale factor
    uint8_t wifi_status; /*0:disconnect, 1: wick, 2:nomal 3: max */
    wifi_config_t wifi_info;
    SCALE_ERROR err;
    uint8_t ethernetConnect; /* 1:ethernet or 0:wifi*/
    uint8_t scale_mode;      /* 0 : plus  1: minus */
    uint8_t touch_cali_flag;
    size_t scale_sampling; /* default 2, 1: 매우 빠름 그러나 흔들림 */
    char ip[16];
    char server[255];
    float standardWeight; /* 교정 분동값 */
    float maxFactor;      /* 교정 할때 max 값*/
    float maxweight;      /* 표시창 최고 값 */
    uint32_t loadCellSpec; /* 로드셀 규격 */
} SCALE_INFO;

extern SCALE_INFO scaleInfo;

void spiffs_init(void);
void saveScaleFactor(float factor);
void saveWifiApname(char *ap);
void saveWifipw(char *pw);
void saveTouch(int32_t tc[6]);
void saveDefaultConfig(void);
void saveDefaultServer(void);
void saveSamplingSpeed(uint8_t sampling);
void savepassdelay(uint8_t delay);
void saveCheattingTimeOut(uint8_t timeOut);
void saveAutoCalibration(uint32_t auto_time, uint32_t auto_cali_band, int32_t st_band, uint8_t stableBufCnt);
void savestandardWeight(float maxFactor, float stadardweight, uint32_t loadcellSpec);

#endif
