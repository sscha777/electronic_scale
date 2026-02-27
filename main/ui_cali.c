
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <machine/endian.h>
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
#include "ui_cali.h"
#include "esp_wifi.h"

extern const char *ver_str;

static uint8_t caliTouchTag = 0;

static const char *TAG = "ui_cali";
static void (*uiCaliTouchCallback)(UI_CALI_EVENT event) = 0;
extern uint32_t mem_hg_head_org_addr, mem_hg_100g_addr, mem_hg_current_addr;
extern uint32_t mem_hg_0g_addr, mem_hg_bowl_addr;
extern SCALE_SCENARIO newScenario;

void *ui_cali_start(void (*touchcallback)(UI_CALI_EVENT event))
{
    uiCaliTouchCallback = touchcallback;

    return readCaliRoof;
}

void readCaliRoof(void)
{
    uint8_t TouchTag = TouchInput();
    uint8_t tagFlag = 0;
    static uint8_t preTag = 0;
    caliTouchTag = TouchTag;
    if (preTag != TouchTag)
    {
        if (TouchTag == 0)
        {
            tagFlag = 1;
        }
        else
        {
            preTag = TouchTag;
        }
    }

    if (preTag >= ZERO_CALI && preTag <= CALI_OK && tagFlag)
    {
        //ESP_LOGI(TAG, ".TOUCH(%d)", preTag);
        tagFlag = 0;
        (*uiCaliTouchCallback)(preTag);
        preTag = 0;
    }

    uiCaliScreen();
}

void uiCaliScreen(void)
{
    float gram;
    uint16_t But_opt;
    static uint16_t old_offset, new_offset;
    uint32_t calc;

    EVE_start_cmd_burst(); /* start writing to the cmd-fifo as one stream of bytes, only sending the address once */

    EVE_cmd_dl(CMD_DLSTART); /* start the display list */
    EVE_cmd_dl(VERTEX_FORMAT(0));

    EVE_cmd_dl(DL_CLEAR_RGB | BLACK);                   /* set the default clear color to white */
    EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG); /* clear the screen - this and the previous prevent artifacts between lists, Attributes are the color, stencil and tag buffers */
    /* 영점조절 제목 */
    EVE_cmd_setbitmap(mem_hg_head_org_addr, EVE_ARGB1555, 245, 57);
    EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
    EVE_cmd_dl(VERTEX_FORMAT(0));
    EVE_cmd_dl(VERTEX2F(253, 19));
    EVE_cmd_dl(DL_END);
    /* 현재무게 */
    EVE_cmd_setbitmap(mem_hg_current_addr, EVE_ARGB1555, 136, 45);
    EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
    EVE_cmd_dl(VERTEX_FORMAT(0));
    EVE_cmd_dl(VERTEX2F(135, 120));
    EVE_cmd_dl(DL_END);
    /* 0g 영점조절 */
    EVE_cmd_setbitmap(mem_hg_0g_addr, EVE_ARGB1555, 126, 45);
    EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
    EVE_cmd_dl(VERTEX_FORMAT(0));
    EVE_cmd_dl(VERTEX2F(148, 180));
    EVE_cmd_dl(DL_END);
    /* 기준무게 영점조절 */
    EVE_cmd_setbitmap(mem_hg_100g_addr, EVE_ARGB1555, 155, 50);
    EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
    EVE_cmd_dl(VERTEX_FORMAT(0));
    EVE_cmd_dl(VERTEX2F(125, 240));
    EVE_cmd_dl(DL_END);
    /* 분동  button */

    /* 용기무게 */
    EVE_cmd_setbitmap(mem_hg_bowl_addr, EVE_ARGB1555, 201, 40);
    EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
    EVE_cmd_dl(VERTEX_FORMAT(0));
    EVE_cmd_dl(VERTEX2F(77, 320));
    EVE_cmd_dl(DL_END);

    EVE_cmd_dl(LINE_WIDTH(1 * 16));
    EVE_cmd_dl(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F(285, 160));
    EVE_cmd_dl(VERTEX2F(485, 160));
    EVE_cmd_dl(DL_END);

    EVE_cmd_dl(LINE_WIDTH(1 * 16));
    EVE_cmd_dl(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F(285, 220));
    EVE_cmd_dl(VERTEX2F(485, 220));
    EVE_cmd_dl(DL_END);

    EVE_cmd_dl(LINE_WIDTH(1 * 16));
    EVE_cmd_dl(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F(285, 280));
    EVE_cmd_dl(VERTEX2F(485, 280));
    EVE_cmd_dl(DL_END);

    EVE_cmd_dl(LINE_WIDTH(1 * 16));
    EVE_cmd_dl(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F(285, 340));
    EVE_cmd_dl(VERTEX2F(485, 340));
    EVE_cmd_dl(DL_END);

    EVE_cmd_dl(TAG_MASK(1));
    EVE_cmd_fgcolor(0x505050);
    But_opt = (caliTouchTag == ZERO_CALI) ? EVE_OPT_FLAT : 0; // button color change if the button during press
    EVE_cmd_dl(TAG(ZERO_CALI));
    EVE_cmd_button(493, 170, 120, 50, 29, But_opt, "0g");

    But_opt = (caliTouchTag == _100_CALI) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(_100_CALI));
    EVE_cmd_button(493, 230, 120, 50, 29, But_opt, "LOAD");

    But_opt = (caliTouchTag == STOP_CALI) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(STOP_CALI));
    EVE_cmd_button((493+120+5), 230, 120, 50, 29, But_opt, "UNLOAD");

    But_opt = (caliTouchTag == BOWL_CALI) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(BOWL_CALI));
    EVE_cmd_button(493, 290, 120, 50, 29, But_opt, "U");

    But_opt = (caliTouchTag == CALI_OK) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(CALI_OK));
    EVE_cmd_button(334, 400, 120, 50, 29, But_opt, "OK");

    gram = ads1232_getUintValue();

    char str[10] = {
        0,
    };

    EVE_color_rgb(255, 255, 255);
    sprintf(str, "%6.2f", gram);
    EVE_cmd_text(410, 125, 30, EVE_OPT_RIGHTX, str);
    EVE_cmd_text(434, 125, 30, 0, "g");
    float gTmp = 0;
    sprintf(str, "%6.0f", gTmp);
    EVE_cmd_text(410, 125 + 60, 30, EVE_OPT_RIGHTX, str);
    EVE_cmd_text(434, 125 + 60, 30, 0, "g");

    if (taskInfo.calFlag == CALI100_START || taskInfo.calFlag == STOP_CALI)
    {
        gTmp = scaleInfo.standardWeight;
        sprintf(str, "%6.2f", taskInfo.cali_tmp_factor);
        EVE_cmd_text(410, 125 + (60 * 2), 30, EVE_OPT_RIGHTX, str);
        EVE_cmd_text(434, 125 + (60 * 2), 30, 0, "g");
    }
    else
    {
        gTmp = scaleInfo.standardWeight;
        sprintf(str, "%6.1f", gTmp);
        EVE_cmd_text(410, 125 + (60 * 2), 30, EVE_OPT_RIGHTX, str);
        EVE_cmd_text(434, 125 + (60 * 2), 30, 0, "g");
    }

    sprintf(str, "%6.0f", taskInfo.bowl_weight);
    EVE_cmd_text(410, 125 + (60 * 3), 30, EVE_OPT_RIGHTX, str);
    EVE_cmd_text(434, 125 + (60 * 3), 30, 0, "g");

    EVE_cmd_text(10, 450, 24, 0, "IP :");
    EVE_cmd_text(80, 450, 24, 0, scaleInfo.ip);

    EVE_cmd_text(630, 450, 24, 0, ver_str);

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

    // //ESP_LOGI(TAG, "ui_fail_weight()");
    return;
}

void caliCallback(UI_CALI_EVENT event)
{
    int32_t dat, raw_data;
    switch (event)
    {
    case ZERO_CALI:
        taskInfo.calFlag = CALI0_START;
        ESP_LOGI(TAG, "ZERO_CALI");
        break;
    case _100_CALI:
        taskInfo.calFlag = CALI100_START;
        ESP_LOGI(TAG, "100_CALI");
        break;
    case STOP_CALI:
        taskInfo.calFlag = STOP_CALI;
        ESP_LOGI(TAG, "STOP_CALI");
        break;

    case BOWL_CALI:

        if (taskInfo.stable_flag)
        {
            /* 용기무게 raw_data*/
            taskInfo.bowl_weight = ads1232_getUnitRawData(scaleInfo.scale_sampling, &dat, &raw_data);
            taskInfo.intBowlWeight = dat;
            if (taskInfo.intBowlWeight >= taskInfo.zero_band)
            {
                taskInfo.bowl_flag = 1;
                ESP_LOGI(TAG, "bowl = %f", taskInfo.bowl_weight);
            }
            else
            {
                taskInfo.bowl_flag = 0;
                taskInfo.intBowlWeight = 0;
                taskInfo.bowl_weight = 0;
                ESP_LOGI(TAG, "bowl = %f", taskInfo.bowl_weight);
            }
        }

        ESP_LOGI(TAG, "bowl = %f", taskInfo.bowl_weight);
        break;

    case CALI_OK:
        newScenario = MAIN_SCALE_SCENARIO;
        break;
    }

    EVE_sound_play(AUDIO_CLICK, AUDIO_VOLUME);
}
