
#ifndef TFT_DATA_H_
#define TFT_DATA_H_

#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#define PROGMEM
#endif
#define BMP_GREEN_CHK_SIZE 8871
#define BMP_GREEN
#define BMP_NOMAL_SIZE 11514
#define BMP_RED_SIZE 9040
#define BMP_BLUE_SIZE 4793
#define BMP_BAD_SIZE 1417
// #define BMP_CALIB_SIZE 1055//nomal
#define BMP_CALIB_SIZE 1500 //blue
#define BMP_START_SIZE 1062
#define BMP_RESET_SIZE 857
#define BMP_BOWL_SIZE 1129
#define BMP_WIFI_FULL_SIZE 826
#define BMP_WIFI_MID_SIZE 515
#define BMP_WIFI_LOW_SIZE 313
#define BMP_WIFI_DISCON_SIZE 654

#define BMP_TOTALWEIGHT_SIZE 1192
//#define BMP_TOTALCOUNT_SIZE 1384
#define BMP_TOTALCOUNT_SIZE 878
#define BMP_TOTALTIME_SIZE 1547
// #define BMP_TOTALTIME_SIZE 1440

#define BMP_UI_FAIL_SIZE 3638
#define BMP_HG_WORK_START_SIZE 2268
#define BMP_HG_WORK_END_SIZE 7863
#define BMP_HG_HEAD_ORG_SIZE 1588
#define BMP_HG_100G_ORG_SIZE 1069
#define BMP_HG_CURRENT_SIZE 857
#define BMP_HG_0G_ORG_SIZE 890
#define BMP_HG_BOWL_SIZE 1309
#define BMP_HG_UNREG_SIZE 5386
#define BMP_HG_INIT_SIZE 5142

#define BMP_STATE_ICON_SIZE 3600

// extern const uint8_t DigitsFont[10000];

extern const uint8_t green_check[BMP_GREEN_CHK_SIZE];
extern const uint8_t bmp_nomal[BMP_NOMAL_SIZE];
extern const uint8_t bmp_red[BMP_RED_SIZE];
extern const uint8_t bmp_blue[BMP_BLUE_SIZE];
extern const uint8_t bmp_bad[BMP_BAD_SIZE];
extern const uint8_t bmp_calib[BMP_CALIB_SIZE];
extern const uint8_t bmp_start[BMP_START_SIZE];
extern const uint8_t bmp_bowl[BMP_BOWL_SIZE];
extern const uint8_t bmp_reset[BMP_RESET_SIZE];
extern const uint8_t bmp_wifi_discon[BMP_WIFI_DISCON_SIZE];
extern const uint8_t bmp_wifi_full[BMP_WIFI_FULL_SIZE];
extern const uint8_t bmp_wifi_mid[BMP_WIFI_MID_SIZE];
extern const uint8_t bmp_wifi_low[BMP_WIFI_LOW_SIZE];
// extern const uint8_t bmp_totalweight[BMP_TOTALWEIGHT_SIZE];
extern const uint8_t bmp_totalCount[BMP_TOTALCOUNT_SIZE];
extern const uint8_t bmp_totaltime[BMP_TOTALTIME_SIZE];
extern const uint8_t bmp_hg_work_start[BMP_HG_WORK_START_SIZE];
extern const uint8_t bmp_hg_work_end[BMP_HG_WORK_END_SIZE];
extern const uint8_t bmp_hg_head_org[BMP_HG_HEAD_ORG_SIZE];
extern const uint8_t bmp_hg_100g_org[BMP_HG_100G_ORG_SIZE];
extern const uint8_t bmp_hg_current[BMP_HG_CURRENT_SIZE];
extern const uint8_t bmp_hg_0g_org[BMP_HG_0G_ORG_SIZE];
extern const uint8_t bmp_hg_bowl[BMP_HG_BOWL_SIZE];
extern const uint8_t bmp_hg_unreg[BMP_HG_UNREG_SIZE];
extern const uint8_t bmp_hg_init[BMP_HG_INIT_SIZE];
extern const uint8_t bmp_ui_fail[BMP_UI_FAIL_SIZE];

// extern const uint8_t bmp_zero_icon[BMP_STATE_ICON_SIZE];
// extern const uint8_t bmp_stable_icon[BMP_STATE_ICON_SIZE];
// extern const uint8_t bmp_bowl_icon[BMP_STATE_ICON_SIZE];

#endif /* TFT_DATA_H_ */
