
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
#include "ui_fail.h"

static const char *TAG = "ui_fail";
static void (*uiFailTouchCallback)(UI_FAIL_EVENT event) = 0;
static uint8_t failTouchTag = 0;
extern UI_FAIL_EVENT wifiScanInfo;
extern SCALE_SCENARIO newScenario;
extern uint32_t mem_ui_fail_addr;
extern uint32_t mem_work_end_addr;
/**
 * @brief 불량 등록 UI
 *
 */
void *ui_fail_start(void (*touchcallback)(UI_FAIL_EVENT event))
{
    uiFailTouchCallback = touchcallback;

    return readFailWeight;
}

void readFailWeight(void)
{
    uint8_t TouchTag = TouchInput();
    uint8_t tagFlag = 0;
    static uint8_t preTag = 0;

    failTouchTag = TouchTag;
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

    if (preTag >= WORK_OK && preTag <= FAIL_OK && tagFlag)
    {
        tagFlag = 0;
        (*uiFailTouchCallback)(preTag);
        preTag = 0;
    }

    if (taskInfo.work_step == W_DEFACTIVE_REG)
    {
        ui_msg_job_complete();
    }
    else
    {
        ui_fail_screen();
    }

    return;
}

void failWeightCallback(UI_FAIL_EVENT event)
{
    float temp;
    switch (event)
    {
    case WORK_OK:
        taskInfo.prev_seq = taskInfo.work_seq;
         memset(taskInfo.prev_work_name, 0, sizeof(taskInfo.prev_work_name));
         strcpy(taskInfo.prev_work_name, taskInfo.work_name);
         taskInfo.work_step = W_DEFACTIVE_OK;
        //ESP_LOGI(TAG, "WORK_OK");
        break;
    case FAIL_CANCLE:
        taskInfo.work_step = NO_JOB_NO_WORKER;
        newScenario = MAIN_SCALE_SCENARIO;
        //ESP_LOGI(TAG, "FAIL_CANCLE");
        break;
    case FAIL_OK:
        taskInfo.work_step = W_POST_DEFACTIVE;
        temp = ads1232_getUintValue() - taskInfo.bowl_weight;
        taskInfo.defactive_weight = (uint32_t)temp;
        newScenario = MAIN_SCALE_SCENARIO;
        //ESP_LOGI(TAG, "FAIL_OK");
        break;
    }
    EVE_sound_play(AUDIO_CLICK, AUDIO_VOLUME);
}

void ui_msg_job_complete(void)
{
    uint16_t But_opt;
    static uint16_t old_offset, new_offset;
    uint32_t calc;

    EVE_start_cmd_burst(); /* start writing to the cmd-fifo as one stream of bytes, only sending the address once */

    EVE_cmd_dl(CMD_DLSTART); /* start the display list */
    EVE_cmd_dl(VERTEX_FORMAT(0));

    EVE_cmd_dl(DL_CLEAR_RGB | STARTEND);                /* set the default clear color to white */
    EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG); /* clear the screen - this and the previous prevent artifacts between lists, Attributes are the color, stencil and tag buffers */
    // EVE_cmd_bgcolor(0x0000a2e8);

    EVE_cmd_setbitmap(mem_work_end_addr, EVE_ARGB1555, 188, 64); // 188, 'x', 64,
    EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
    EVE_cmd_dl(VERTEX_FORMAT(0));
    EVE_cmd_dl(VERTEX2F(310, 132));
    EVE_cmd_dl(DL_END);
    // EVE_color_rgb(100, 255, 100);
    // EVE_cmd_text(EVE_HSIZE * 0.4, EVE_VSIZE * 0.5, 33, EVE_OPT_CENTER, "Work Complete!");
    EVE_cmd_fgcolor(0x505050);
    EVE_cmd_bgcolor(0x505050);

    But_opt = (failTouchTag == WORK_OK) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(WORK_OK));
    EVE_cmd_button(309, 303, 160, 60, 30, But_opt, "OK");

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

void ui_fail_screen(void)
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

    EVE_cmd_setbitmap(mem_ui_fail_addr, EVE_ARGB1555, 490, 157);
    EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
    EVE_cmd_dl(VERTEX_FORMAT(0));
    EVE_cmd_dl(VERTEX2F(168, 60));
    EVE_cmd_dl(DL_END);

    // if (taskInfo.work_name[0] != 0)
    // {
    //     drawHangle(160, 115, taskInfo.work_name, PURPLE, HG_MEM_WORK_NAME);
    // }
    // else
    // {
    //     EVE_cmd_text(160, 115, 32, EVE_OPT_RIGHTX, "---");
    // }

    gram = ads1232_getUintValue();
    if (gram < 0)
    {
        gram = 0;
    }

    char str[10] = {
        0,
    };

    sprintf(str, "%6.0f", gram);
    EVE_cmd_text(572, 265, 31, EVE_OPT_RIGHTX, str);
    EVE_cmd_text(603, 265, 31, 0, "g");
    EVE_color_rgb(255, 255, 255);
    EVE_cmd_dl(LINE_WIDTH(1 * 16));
    EVE_cmd_dl(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F(169, 333));
    EVE_cmd_dl(VERTEX2F(657, 333));
    EVE_cmd_dl(DL_END);

    EVE_cmd_fgcolor(0x505050);
    EVE_cmd_bgcolor(0x505050);

    But_opt = (failTouchTag == FAIL_CANCLE) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(FAIL_CANCLE));
    EVE_cmd_button(331, 387, 160, 60, 30, But_opt, "Cancle");

    But_opt = (failTouchTag == FAIL_OK) ? EVE_OPT_FLAT : 0;
    EVE_cmd_dl(TAG(FAIL_OK));
    EVE_cmd_button(500, 387, 160, 60, 30, But_opt, "OK");

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