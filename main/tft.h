
#ifndef TFT_H_
#define TFT_H_

#define AUDIO_VOLUME 0xff

/* Typedef name : TouchCalibrationValues.
   Description: It is a union of uint8 array and uint32 array.
                uint32 array stores values about calibration of touch panel.
                uint8 array gives access to those values in bytes form to ease
                storing in flash rom.
*/
typedef union
{
    uint8_t TouchTransform_Bytes[24];
    int32_t TouchTransform_X[6];
} TouchCalibrationValues;

extern TouchCalibrationValues calibrationvalues;

/* some pre-definded colors */
#define RED 0xff0000UL
#define ORANGE 0xffa500UL
#define GREEN 0x00ff00UL
#define GREEN_1 0x10B010
#define BLUE 0x0000ffUL
#define BLUE_1 0x5dade2L
#define YELLOW 0xffff00UL
#define PINK 0xff00ffUL
#define PURPLE 0x800080UL
#define WHITE 0xffffffUL
#define WHITE_1 0xcfcfcfUL
#define BLACK 0x000000UL
#define STARTEND 0x0029B452UL
#define START 0x0000A2E8UL
#define GRAY 0x007F7F7FUL
#define GRAY_1 0x007F7F7FUL
#define GRAY_2 0x005F5F5FUL
#define MSG_GRN 0x0020b449UL

/* memory-map defines */
#define MEM_LOGO 0x00000000 /* start-address of logo, needs 2242 bytes of memory */
#define MEM_PIC1 0x00000900 /* start of 100x100 pixel test image, ARGB565, needs 20000 bytes of memory */
#define MEM_DIGIFONT 60000

#define MEM_BMP_NOMAL 0x00000000
#define MEM_BMP_GREEN_CHK (BMP_NOMAL_SIZE + 1)
#define MEM_BMP_GREEN
#define MEM_BMP_RED
#define MEM_BMP_BLUE

#define MEM_DL_STATIC (EVE_RAM_G_SIZE - 4096) /* start-address of the static part of the display-list, upper 4k of gfx-mem */

#define AUDIO_CLICK 0x50
#define AUDIO_TAG_CLK 0x53
#define AUDIO_TGA 0x44

extern uint16_t num_profile_a, num_profile_b;

typedef enum
{
    NUM_NONE = 0,
    NUM_0 = 1,
    NUM_1 = 2,
    NUM_2 = 3,
    NUM_3 = 4,
    NUM_4 = 5,
    NUM_5 = 6,
    NUM_6 = 7,
    NUM_7 = 8,
    NUM_8 = 9,
    NUM_9 = 10,
    NUM_CLR = 11,
    NUM_ALL = 12,
    NUM_STARTPAUSE = 13,
    NUM_STOP = 14,
    NUM_CALL = 15
} NUM_EVENTS;

typedef enum
{
    NOMAL_SCALE = 0,
    SMART_SCALE,

} SCALE_STATE;

typedef enum
{
    HOME_NONE = 0,
    WIFI_SCAN,
    SCALE_START,
    SCALE_CAL,
    FAIL_WEIGHT,
    SCALE_RESET,
    HOME_BOWL_SET,
    HOME_ZERO_CALI
} HOME_EVENTS;

typedef enum
{
    WIFI_1 = 1,
    WIFI_2,
    WIFI_3,
    WIFI_4,
    WIFI_5,
    WIFI_6,
    WIFI_7,
    WIFI_8,
    WIFI_9,
    WIFI_10,
    WIFI_11,
    WIFI_12,
    WIFI_AP_CLOSE
} WIFI_SEL_EVENTS;

typedef enum
{
    WIFI_PW_CLOSE = 249,
    WIFI_PW_SAVE = 250,
} KEYBD_EVENT;

typedef enum
{
    WORK_OK = 101,
    FAIL_CANCLE = 102,
    FAIL_OK = 103,
} UI_FAIL_EVENT;

typedef enum
{
    INIT_CANCLE = 99,
    INIT_OK = 100,
} UI_INIT_EVENT;

typedef enum
{
    ZERO_CALI = 112,
    _100_CALI,
    STOP_CALI,
    BOWL_CALI,
    CALI_OK
} UI_CALI_EVENT;

typedef enum
{
    NO_SCENARIO = 0,
    INTRO_SCENARIO = 1,
    MAIN_SCALE_SCENARIO = 2,
    FAILWEIGHT_SCENARIO = 3,
    WIFI_SCAN_SCNARIO = 4,
    SCALEINIT_SCENARIO = 5,
    WIFI_KDB_SCNARIO = 6,
    MSG_INIT_SCNARIO = 7,
    MSG_UNREG_SCNARIO = 8,
    MSG_START_SCNARIO = 9,
    CALIBRATION_SCNARIO = 10
} SCALE_SCENARIO;

typedef enum
{
    UNREG_OK = 101,
} MSG_UN_REG;

typedef enum
{
    START_OK = 102,
    START_CANCLE = 103
} MSG_START;

typedef enum
{
    GMAIN_HANDLE_GREENCHK = 1,
    GMAIN_HANDLE_GREEN = 2,
    GMAIN_HANDLE_RED = 3,
    GMAIN_HANDLE_BLUE = 4,
    GMAIN_HANDLE_NOMAL = 5,
    GMAIN_HANDLE_WIFIMAX = 6,
    GMAIN_HANDLE_WIFIMID = 7,
    GMAIN_HANDLE_WIFILOW = 8,
    GMAIN_HANDLE_NO_WIFI = 9,
    GMAIN_HANDLE_CAL = 10,
    GMAIN_HANDLE_START = 11,
    GMAIN_HANDLE_FAIL_WEIGHT = 12,
    GMAIN_HANDLE_RESET = 13,
    GMAIN_HANDLE_TOTALWEIGHT = 14,
    GMAIN_HANDLE_TOTALTIME = 16,
    GMAIN_HANDLE_WORK_START = 17,
    GMAIN_HANDLE_WORK_END = 18,
    GMAIN_HANDLE_HEAD_ORG = 19,
    GMAIN_BOWL_START = 20,
    GMAIN_HANDLE_100G_ORG,
    GMAIN_HANDLE_CURRENT,
    GMAIN_HANDLE_0G,
    GMAIN_HANDLE_BOWL,
    GMAIN_HANDLE_UNREG,
    GMAIN_HANDLE_INIT,
    GMAIN_HANDLE_UI_FAIL,
    GMAIN_HANDLE_HANGLEFONT,
    GMAIN_HANDLE_STABLE_ICON,
    GMAIN_HANDLE_ZERO_ICON,
    GMAIN_HANDLE_BOWL_ICON
} BMP_HANDLE;

#define LINE_STARTPOS (EVE_HSIZE / 50) // Start of Line
#define LINE_ENDPOS (EVE_HSIZE * 2)    // max length of the line

void TFT_init(void);
uint8_t TouchInput(void);
void guiHome(void);
void initStaticBackground();
void keyBoardCallBack(void);

void guiHomeTouchCallback(HOME_EVENTS event);
void homeScreenLoop();
// void *scaleHome(void (*touchcallback)(), void (**closefunction)());
void *scaleHome(void (*touchcallback)());
void EVE_bitmap_load(void);
void whiteScreen(void);
void ui_reset(void);
void *ui_init_start(void (*touchcallback)(UI_INIT_EVENT event));
void *msg_unreg_start(void (*touchcallback)(MSG_UN_REG event));
void *msg_start_start(void (*touchcallback)(MSG_START event));
void initCallback(UI_INIT_EVENT event);
void unReg_Callback(MSG_UN_REG event);
void msgWorkUnRegScreen(void);
void msgStartScreen(void);
void start_Callback(MSG_START event);

#endif /* TFT_H_ */
