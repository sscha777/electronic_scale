
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "esp_log.h"
#include "eve/EVE.h"
#include "eve/EVE_target.h"
#include "eve/EVE_commands.h"
#include "tft_data.h"
#include "ads1232.h"
#include "scspiffs.h"
#include "sntp.h"
#include "tft.h"
#include "hangle.h"
#include "http.h"
#include "intro.h"
#include "wifi_station.h"
#include "ui_wifi_scan.h"

// static const char *TAG = "wifi_scan";
struct Notepad_buffer
{
    char *temp;
    char notepad[80];
} Buffer;

Flag_t Flag;

//static char kb_val = 0;
static uint16_t Disp_pos = 0;
static uint16_t noofchars = 0;
static uint8_t touch_detect = 1;
static uint8_t Read_sfk = 0;
static uint8_t sel_ssid_num = 0;

static void (*wifiTouchCallback)(WIFI_SEL_EVENTS event) = 0;
extern WIFI_SCAN_INFO wifiScanInfo;
extern SCALE_SCENARIO newScenario;

static void (*KBDtouchCallback)(KEYBD_EVENT event) = 0;
static uint16_t wifiApTag = 0;

/**
 * @brief ap list를 보여준다, 클릭하면 password 입력
 * ap scan 시 진행중 표시
 * 스캔 타이아웃될때까지 혹은 지정된 ap갯수에 도달하면 스캔 중지
 */
void *ui_wifi_scan_start(void (*touchcallback)(WIFI_SEL_EVENTS event))
{
    // ui_wifi_scanlist();
    wifiTouchCallback = touchcallback;
    wifiScanInfo.wifi_ap_count = 0;
    wifiScanInfo.wifi_scanStep = WIFI_SCAN_START;
    while (wifiScanInfo.wifi_scanStep != WIFI_SCAN_END)
    {
        ui_wifi_Scanning();
        DELAY_MS(1000);
    }

    return readwifiScanList;
}

void ui_wifi_Scanning(void)
{
    // uint8_t But_opt;
    static uint16_t old_offset, new_offset;
    uint32_t calc;

    EVE_start_cmd_burst(); /* start writing to the cmd-fifo as one stream of bytes, only sending the address once */

    EVE_cmd_dl(CMD_DLSTART); /* start the display list */
    EVE_cmd_dl(VERTEX_FORMAT(0));

    EVE_cmd_dl(DL_CLEAR_RGB | BLACK);                   /* set the default clear color to white */
    EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG); /* clear the screen - this and the previous prevent artifacts between lists, Attributes are the color, stencil and tag buffers */

    EVE_cmd_text(EVE_HSIZE * 0.5, EVE_VSIZE * 0.2, 30, EVE_OPT_CENTER, "Wifi AP Scanning...");
    // EVE_cmd_spinner(EVE_HSIZE * 0.5, EVE_VSIZE * 0.5, 0, 0);

    // But_opt = (wifiApTag == WIFI_AP_CLOSE) ? EVE_OPT_FLAT : 0;
    // EVE_cmd_dl(TAG(WIFI_AP_CLOSE));
    // EVE_cmd_button(10, 10, 60, 40, 28, But_opt, "CLOSE");

    new_offset = EVE_report_cmdoffset();
    if (old_offset > new_offset)
    {
        new_offset += 4096;
    }
    calc = new_offset - old_offset;
    calc += 24; /* adjust for the commands that follow before the end */

    EVE_cmd_dl(DL_DISPLAY); /* instruct the graphics processor to show the list */
    EVE_cmd_dl(CMD_SWAP);   /* make this list active */

    EVE_end_cmd_burst(); /* stop writing to the cmd-fifo */
    EVE_cmd_start();     /* order the command co-processor to start processing its FIFO queue but do not wait for completion */

    //ESP_LOGI(TAG, "ui_wifi_Scanning()");
    return;
}

void ui_wifi_scanlist(void)
{
    uint8_t But_opt;
    static uint16_t old_offset, new_offset;
    uint32_t calc;

    EVE_start_cmd_burst(); /* start writing to the cmd-fifo as one stream of bytes, only sending the address once */

    EVE_cmd_dl(CMD_DLSTART); /* start the display list */
    EVE_cmd_dl(VERTEX_FORMAT(0));

    EVE_cmd_dl(DL_CLEAR_RGB | BLACK);                   /* set the default clear color to white */
    EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG); /* clear the screen - this and the previous prevent artifacts between lists, Attributes are the color, stencil and tag buffers */

    EVE_cmd_dl(TAG_MASK(1));
    EVE_cmd_fgcolor(0x505050);
    EVE_cmd_bgcolor(0x505050);

    int font_size = 28;

    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < wifiScanInfo.wifi_ap_count); i++)
    {
        if (!strcmp((char *)wifiScanInfo.ap_info[i].ssid, (char *)scaleInfo.wifi_info.sta.ssid))
        {
            EVE_cmd_fgcolor(0xB03030);
        }
        else
        {
            EVE_cmd_fgcolor(0x505050);
        }
        if (i < 6)
        {
            But_opt = (wifiApTag == i + 1) ? EVE_OPT_FLAT : 0; // button color change if the button during press
            EVE_cmd_dl(TAG(i + 1));
            EVE_cmd_button(EVE_HSIZE * 0.05, EVE_VSIZE * (0.14 * (i + 1)), EVE_HSIZE * 0.43, EVE_VSIZE * 0.1, font_size, But_opt, (char *)wifiScanInfo.ap_info[i].ssid);
        }
        else
        {
            But_opt = (wifiApTag == i + 1) ? EVE_OPT_FLAT : 0;
            EVE_cmd_dl(TAG(i + 1));
            EVE_cmd_button(EVE_HSIZE * 0.55, EVE_VSIZE * (0.14 * (i - 5)), EVE_HSIZE * 0.43, EVE_VSIZE * 0.1, font_size, But_opt, (char *)wifiScanInfo.ap_info[i].ssid);
        }
        // //ESP_LOGI(TAG,"(%d)%s",i,wifiScanInfo.ap_info[i].ssid);
    }
    EVE_cmd_fgcolor(0x505050);
    EVE_cmd_text(EVE_HSIZE * 0.5, EVE_VSIZE * 0.05, 30, EVE_OPT_CENTER, "Wifi AP List");
    But_opt = (wifiApTag == WIFI_AP_CLOSE) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(WIFI_AP_CLOSE));
    EVE_cmd_button(10, 10, 80, 40, font_size, But_opt, "CLOSE");

    new_offset = EVE_report_cmdoffset();
    if (old_offset > new_offset)
    {
        new_offset += 4096;
    }
    calc = new_offset - old_offset;
    calc += 24; /* adjust for the commands that follow before the end */

    EVE_cmd_dl(DL_DISPLAY); /* instruct the graphics processor to show the list */
    EVE_cmd_dl(CMD_SWAP);   /* make this list active */

    EVE_end_cmd_burst(); /* stop writing to the cmd-fifo */
    EVE_cmd_start();     /* order the command co-processor to start processing its FIFO queue but do not wait for completion */

    // //ESP_LOGI(TAG, "ui_wifi_scanlist");
    return;
}

void readwifiScanList(void)
{
    uint8_t TouchTag = TouchInput();
    static uint8_t preTag = 0;

    if (preTag != TouchTag)
    {
        preTag = TouchTag;
        wifiApTag = TouchTag;
        //ESP_LOGI(TAG, "tag = %d", TouchTag);
    }

    if (TouchTag >= WIFI_1 && TouchTag <= WIFI_AP_CLOSE)
    {
        //ESP_LOGI(TAG, ".TOUCH(%d)", TouchTag);
        (*wifiTouchCallback)(TouchTag);
    }

    ui_wifi_scanlist();
    return;
}

void wifiScanListCallback(WIFI_SEL_EVENTS event)
{
    switch (event)
    {
    case WIFI_1:
    case WIFI_2:
    case WIFI_3:
    case WIFI_4:
    case WIFI_5:
    case WIFI_6:
    case WIFI_7:
    case WIFI_8:
    case WIFI_9:
    case WIFI_10:
    case WIFI_11:
    case WIFI_12:
        newScenario = WIFI_KDB_SCNARIO;
        sel_ssid_num = event - 1;
        //ESP_LOGI(TAG, "call = %d", event);
        break;
    case WIFI_AP_CLOSE:
        //ESP_LOGI(TAG, "MAIN_SCALE_SCENARIO = %d", event);
        newScenario = MAIN_SCALE_SCENARIO;
        break;
    }
    EVE_sound_play(AUDIO_CLICK,AUDIO_VOLUME);
}

/***
 *
 *
 *
 */

void *keyBoardStart(void (*touchcallback)(KEYBD_EVENT event))
{
    KBDtouchCallback = touchcallback;

    ui_wifi_keyPad();
    // Clear Linebuffer
    memset(&Buffer.notepad, '\0', sizeof(Buffer.notepad));

    /*intial setup*/
    Disp_pos = LINE_STARTPOS;                             // starting pos
    Flag.Numeric = OFF;                                   // Disable the numbers and spcial charaters
    memset((Buffer.notepad + 0), '_', 1);                 // For Cursor
    Disp_pos += EVE_Rom_Font_WH(Buffer.notepad[0], Font); // Update the Disp_Pos
    noofchars += 1;                                       // for cursor
    /*enter*/
    Flag.Exit = 0;

    return Read_Keypad;
}

static uint8_t istouch()
{
    return !(EVE_memRead16(REG_TOUCH_RAW_XY) & 0x8000);
}

void Read_Keypad(void)
{
    static uint8_t temp_tag = 0;
    Read_sfk = EVE_memRead8(REG_TOUCH_TAG);

    if (istouch() == 0)
        touch_detect = 0;

    if (Read_sfk != 0) // Allow if the Key is released
    {
        if (temp_tag != Read_sfk && touch_detect == 0)
        {
            temp_tag = Read_sfk; // Load the Read tag to temp variable
            touch_detect = 1;
            //ESP_LOGI(TAG, "press = %d", temp_tag);
        }
    }
    else
    {
        if (temp_tag != 0)
        {
            Flag.Key_Detect = 1;
            Read_sfk = temp_tag;
            (*KBDtouchCallback)(Read_sfk);
            //ESP_LOGI(TAG, "touch = %d", Read_sfk);
        }
        temp_tag = 0;
    }
    ui_wifi_keyPad();
}

/*
keyboard
*/
void RemoveEnd(char *buf)
{
    int i = 0;
    while (buf[i]) // buf[i]가 참(널문자가 아님)이면 반복하여라.
    {
        i++;
    }
    //현재 i는 널문자가 있는 위치, i-1은 마지막 문자 위치
    buf[i - 1] = '\0';
}

void wifi_kbd_Callback(KEYBD_EVENT event)
{
    switch (event)
    {
    case WIFI_PW_CLOSE:
        newScenario = MAIN_SCALE_SCENARIO;
        break;
    case WIFI_PW_SAVE:
        saveWifiApname((char *)wifiScanInfo.ap_info[sel_ssid_num].ssid);
        RemoveEnd(Buffer.notepad); // 커서 제거
        saveWifipw(Buffer.notepad);
        //ESP_LOGI(TAG, "ap = %s, pw = %s", (char *)wifiScanInfo.ap_info[sel_ssid_num].ssid, (char *)Buffer.notepad);
        newScenario = MAIN_SCALE_SCENARIO;
        break;

    default:
        break;
    }
    EVE_sound_play(AUDIO_CLICK, AUDIO_VOLUME);
}

uint8_t EVE_Rom_Font_WH(uint8_t Char, uint8_t font)
{
    uint32_t ptr, Wptr;
    uint8_t Width = 0;
    ptr = EVE_memRead32(0xffffc);

    // read Width of the character
    Wptr = (ptr + (148 * (font - 16))) + Char; // (table starts at font 16)
    Width = EVE_memRead8(Wptr);
    return Width;
}

void ui_wifi_keyPad(void)
{
    uint16_t But_opt;
    uint8_t tval = 0;
    uint8_t font = 30;
    uint16_t old_offset, new_offset;
    uint16_t display_list_size = 0;
    uint16_t FT_DispWidth = EVE_HSIZE;
    uint16_t FT_DispHeight = EVE_VSIZE;
    uint32_t calc;

    if (Flag.Key_Detect)
    { // check if key is pressed
        //ESP_LOGI(TAG, "Flag.Key_Detect");
        EVE_sound_play(AUDIO_CLICK, AUDIO_VOLUME);
        Flag.Key_Detect = 0; // clear it
        if (Read_sfk >= SPECIAL_FUN)
        { // check any special function keys are pressed
            switch (Read_sfk)
            {
            case BACK_SPACE:
                if (noofchars > 1) // check in the line there is any characters are present,cursor not include
                {
                    noofchars -= 1;                                                       // clear the character inthe buffer
                    Disp_pos -= EVE_Rom_Font_WH(*(Buffer.notepad + noofchars - 1), Font); // Update the Disp_Pos
                }
                else
                {
                    noofchars = strlen(Buffer.notepad);                          // Read the len of the line
                    for (tval = 0; tval < noofchars; tval++)                     // Compute the length of the Line
                        Disp_pos += EVE_Rom_Font_WH(Buffer.notepad[tval], Font); // Update the Disp_Pos
                }
                Buffer.temp = (Buffer.notepad + noofchars); // load into temporary buffer
                Buffer.temp[-1] = '_';                      // update the string
                Buffer.temp[0] = '\0';
                break;

            case CAPS_LOCK:
                Flag.Caps ^= 1; // toggle the caps lock on when the key detect
                break;

            case NUMBER_LOCK:
                Flag.Numeric ^= 1; // toggle the number lock on when the key detect
                break;

            case BACK: // clear
                memset(&Buffer.notepad[0], '\0', sizeof(Buffer.notepad));
                Disp_pos = LINE_STARTPOS; // starting pos
                //       Flag.Numeric = OFF;                             // Disable the numbers and spcial charaters
                memset((Buffer.notepad + 0), '_', 1);                 // For Cursor
                Disp_pos += EVE_Rom_Font_WH(Buffer.notepad[0], Font); // Update the Disp_Pos
                noofchars += 1;
                break;
            }
        }
        else
        {
            Disp_pos += EVE_Rom_Font_WH(Read_sfk, Font);           // update dispos
            Buffer.temp = Buffer.notepad + strlen(Buffer.notepad); // load into temporary buffer
            Buffer.temp[-1] = Read_sfk;                            // update the string
            Buffer.temp[0] = '_';
            Buffer.temp[1] = '\0';
            noofchars = strlen(Buffer.notepad); // get the string len
            //ESP_LOGI(TAG, "Disp_pos = %d(0x%x)", Disp_pos, Disp_pos);
            if (Disp_pos > LINE_ENDPOS) // check if there is place to put a character in a specific line
            {
                Buffer.temp = Buffer.notepad + (strlen(Buffer.notepad) - 1);
                Buffer.temp[0] = '\0';
                noofchars -= 1;
                Disp_pos = LINE_STARTPOS;

                memset((Buffer.notepad), '\0', sizeof(Buffer.notepad)); // Clear the line buffer
                for (; noofchars >= 1; noofchars--, tval++)
                {
                    if (Buffer.notepad[noofchars] == ' ' || Buffer.notepad[noofchars] == '.') // In case of space(New String) or end of statement(.)
                    {
                        memset(Buffer.notepad + noofchars, '\0', 1);
                        noofchars += 1; // Include the space
                        memcpy(&Buffer.notepad, (Buffer.notepad + noofchars), tval);
                        break;
                    }
                }
                noofchars = strlen(Buffer.notepad);
                Buffer.temp = Buffer.notepad + noofchars;
                Buffer.temp[0] = '_';
                Buffer.temp[1] = '\0';
                for (tval = 0; tval < noofchars; tval++)
                    Disp_pos += EVE_Rom_Font_WH(Buffer.notepad[tval], Font); // Update the Disp_Pos
            }
        }
    }

    old_offset = EVE_report_cmdoffset(); /* used to calculate the amount of cmd-fifo bytes necessary */
    display_list_size = EVE_memRead16(REG_CMD_DL);
    display_list_size = display_list_size; // to avoid warning
    EVE_start_cmd_burst(); /* start writing to the cmd-fifo as one stream of bytes, only sending the address once */

    EVE_cmd_dl(CMD_DLSTART); /* start the display list */
    EVE_cmd_dl(VERTEX_FORMAT(0));

    EVE_cmd_dl(DL_CLEAR_RGB | BLACK);                   /* set the default clear color to white */
    EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG); /* clear the screen - this and the previous prevent artifacts between lists, Attributes are the color, stencil and tag buffers */

    EVE_cmd_dl(TAG_MASK(1));
    EVE_cmd_fgcolor(0x505050);
    EVE_cmd_bgcolor(0x505050);

    But_opt = (Read_sfk == BACK) ? EVE_OPT_FLAT : 0; // button color change if the button during press
    EVE_cmd_dl(TAG(BACK));                           // Back		 Return to Home
    EVE_cmd_button((FT_DispWidth * 0.855), (FT_DispHeight * 0.83), (FT_DispWidth * 0.146), (FT_DispHeight * KEY_V_SIZE), font, But_opt, "Clear");
    But_opt = (Read_sfk == BACK_SPACE) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(BACK_SPACE)); // BackSpace
    EVE_cmd_button((FT_DispWidth * 0.875), (FT_DispHeight * 0.70), (FT_DispWidth * 0.125), (FT_DispHeight * KEY_V_SIZE), font, But_opt, "<-");
    But_opt = (Read_sfk == ' ') ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(' ')); // Space
    EVE_cmd_button((FT_DispWidth * 0.115), (FT_DispHeight * 0.83), (FT_DispWidth * 0.73), (FT_DispHeight * KEY_V_SIZE), font, But_opt, "Space");

    if (Flag.Numeric == OFF)
    {
        EVE_cmd_keys(0, (FT_DispHeight * 0.442), FT_DispWidth, (FT_DispHeight * KEY_V_SIZE), font, Read_sfk, (Flag.Caps == ON ? "QWERTYUIOP" : "qwertyuiop"));
        EVE_cmd_keys((FT_DispWidth * 0.042), (FT_DispHeight * 0.57), (FT_DispWidth * 0.96), (FT_DispHeight * KEY_V_SIZE), font, Read_sfk, (Flag.Caps == ON ? "ASDFGHJKL" : "asdfghjkl"));
        EVE_cmd_keys((FT_DispWidth * 0.125), (FT_DispHeight * 0.70), (FT_DispWidth * 0.73), (FT_DispHeight * KEY_V_SIZE), font, Read_sfk, (Flag.Caps == ON ? "ZXCVBNM" : "zxcvbnm"));

        But_opt = (Read_sfk == CAPS_LOCK) ? EVE_OPT_FLAT : 0;
        EVE_cmd_dl(TAG(CAPS_LOCK)); // Capslock
        EVE_cmd_button(0, (FT_DispHeight * 0.70), (FT_DispWidth * 0.10), (FT_DispHeight * KEY_V_SIZE), font, But_opt, "a^");
        But_opt = (Read_sfk == NUMBER_LOCK) ? EVE_OPT_FLAT : 0;
        EVE_cmd_dl(TAG(NUMBER_LOCK)); // Numberlock
        EVE_cmd_button(0, (FT_DispHeight * 0.83), (FT_DispWidth * 0.10), (FT_DispHeight * KEY_V_SIZE), font, But_opt, "12*");
    }

    if (Flag.Numeric == ON)
    {
        EVE_cmd_keys((FT_DispWidth * 0), (FT_DispHeight * 0.442), FT_DispWidth, (FT_DispHeight * KEY_V_SIZE), font, Read_sfk, "1234567890");
        EVE_cmd_keys((FT_DispWidth * 0.042), (FT_DispHeight * 0.57), (FT_DispWidth * 0.96), (FT_DispHeight * KEY_V_SIZE), font, Read_sfk, "-@#$%^&*(");
        EVE_cmd_keys((FT_DispWidth * 0.125), (FT_DispHeight * 0.70), (FT_DispWidth * 0.73), (FT_DispHeight * KEY_V_SIZE), font, Read_sfk, ")_+[]{}");
        But_opt = (Read_sfk == NUMBER_LOCK) ? EVE_OPT_FLAT : 0;
        EVE_cmd_dl(TAG(253)); // Numberlock
        EVE_cmd_button(0, (FT_DispHeight * 0.83), (FT_DispWidth * 0.10), (FT_DispHeight * KEY_V_SIZE), font, But_opt, "AB*");
    }

    But_opt = (Read_sfk == WIFI_PW_CLOSE) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(WIFI_PW_CLOSE)); //
    EVE_cmd_button(10, 10, 120, 50, font, But_opt, "CLOSE");
    But_opt = (Read_sfk == WIFI_PW_SAVE) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(WIFI_PW_SAVE)); //
    EVE_cmd_button(10 + 120 + 20, 10, 120, 50, font, But_opt, "SAVE");

    EVE_cmd_dl(TAG_MASK(0)); // Disable the tag buffer updates
    EVE_cmd_dl(SCISSOR_XY(0, 80));
    EVE_cmd_dl(SCISSOR_SIZE(800, 110));
    EVE_cmd_dl(CLEAR_COLOR_RGB(255, 255, 255));
    EVE_cmd_dl(CLEAR(1, 1, 1));
    EVE_cmd_dl(COLOR_RGB(0, 0, 0)); // Text Color
    EVE_cmd_text(30, 100, 29, 0, "SSID of AP : ");
    EVE_cmd_text(180, 100, 29, 0, (char *)wifiScanInfo.ap_info[sel_ssid_num].ssid);
    EVE_cmd_text(30, 140, 29, 0, "Password   : ");
    // //ESP_LOGI(TAG, "sel_ap = %s", (char *)wifiScanInfo.ap_info[sel_ssid_num].ssid);
    EVE_cmd_text(180, 140, 29, 0, (const char *)&Buffer.notepad);

    new_offset = EVE_report_cmdoffset();
    if (old_offset > new_offset)
    {
        new_offset += 4096;
    }
    calc = new_offset - old_offset;
    calc += 24; /* adjust for the commands that follow before the end */

    EVE_cmd_dl(DL_DISPLAY); /* instruct the graphics processor to show the list */
    EVE_cmd_dl(CMD_SWAP);   /* make this list active */

    EVE_end_cmd_burst(); /* stop writing to the cmd-fifo */
    EVE_cmd_start();     /* order the command co-processor to start processing its FIFO queue but do not wait for completion */

    return;
}
