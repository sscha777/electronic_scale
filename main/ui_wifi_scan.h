
#ifndef _WIFI_SCAN_
#define _WIFI_SCAN_

#define ON 1
#define OFF 0
#define Font 27     // Font Size
#define MAX_LINES 4 // Max Lines allows to Display

#define SPECIAL_FUN 251
#define BACK_SPACE 251  // Back space
#define CAPS_LOCK 252   // Caps Lock
#define NUMBER_LOCK 253 // Number Lock
#define BACK 254        // Exit

// key button size
#define KEY_H_SIZE
#define KEY_V_SIZE 0.099 // 0.112

typedef struct _flag
{
    uint8_t Key_Detect : 1;
    uint8_t Caps : 1;
    uint8_t Numeric : 1;
    uint8_t Exit : 1;
} Flag_t;

extern Flag_t Flag;

void *ui_wifi_scan_start(void (*touchcallback)(WIFI_SEL_EVENTS event));
void *keyBoardStart(void (*touchcallback)(KEYBD_EVENT event));
void ui_wifi_scanlist(void);
void wifiScanListCallback(WIFI_SEL_EVENTS event);
void ui_wifi_keyPad(void);
void wifi_kbd_Callback(KEYBD_EVENT event);
void Read_Keypad(void);
uint8_t EVE_Rom_Font_WH(uint8_t Char, uint8_t font);
void readwifiScanList(void);
void ui_wifi_Scanning(void);


#endif
