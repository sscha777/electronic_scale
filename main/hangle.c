#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pgmspace.h"
#include "esp_log.h"
#include "eve/EVE.h"
#include "eve/EVE_target.h"
#include "eve/EVE_commands.h"
#include "tft_data.h"
#include "ads1232.h"
#include "scspiffs.h"
#include "tft.h"
#include "ASCFont.h"
#include "KSFont.h"
#include "hangle.h"

// static const char *TAG = "HGL";

/*한글폰트용 GRAM 어드레스 */
extern uint32_t mem_hangle_font, mem_hangle_font_img;
// extern uint32_t hfont_addr;

uint8_t HANFontImage[256] = {
    0,
};

uint8_t *getHAN_font(uint8_t HAN1, uint8_t HAN2, uint8_t HAN3)
{
    const uint8_t cho[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0};
    const uint8_t cho2[] = {0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5};
    const uint8_t jong[] = {0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1};
    uint16_t utf16;
    uint8_t first, mid, last;
    uint8_t firstType = 0, midType = 0, lastType = 0;
    uint8_t i;
    uint8_t *pB, *pF;

    /*------------------------------
      UTF-8 을 UTF-16으로 변환한다.

      UTF-8 1110xxxx 10xxxxxx 10xxxxxx
    */
    utf16 = ((HAN1 & 0x0f) << 12 | (HAN2 & 0x3f) << 6 | (HAN3 & 0x3f));

    /*------------------------------
      초,중,종성 코드를 분리해 낸다.

      unicode = {[(초성 * 21) + 중성] * 28}+ 종성 + 0xAC00

            0   1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
      초성 ㄱ   ㄲ ㄴ ㄷ ㄸ ㄹ ㅁ ㅂ ㅃ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ
      중성 ㅏ   ㅐ ㅑ ㅒ ㅓ ㅔ ㅕ ㅖ ㅗ ㅘ ㅙ ㅚ ㅛ ㅜ ㅝ ㅞ ㅟ ㅠ ㅡ ㅢ ㅣ
      종성 없음 ㄱ ㄲ ㄳ ㄴ ㄵ ㄶ ㄷ ㄹ ㄺ ㄻ ㄼ ㄽ ㄾ ㄿ ㅀ ㅁ ㅂ ㅄ ㅅ ㅆ ㅇ ㅈ ㅊ ㅋ ㅌ ㅍ ㅎ
    ------------------------------*/
    utf16 -= 0xac00;
    last = utf16 % 28;
    utf16 /= 28;
    mid = utf16 % 21;
    first = utf16 / 21;
    first++;
    mid++;

    /*------------------------------
      초,중,종성 해당 폰트 타입(벌)을 결정한다.
    ------------------------------*/
    /*
     초성 19자:ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ
     중성 21자:ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ
     종성 27자:ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅆㅇㅈㅊㅋㅌㅍㅎ

     초성
        초성 1벌 : 받침없는 'ㅏㅐㅑㅒㅓㅔㅕㅖㅣ' 와 결합
        초성 2벌 : 받침없는 'ㅗㅛㅡ'
        초성 3벌 : 받침없는 'ㅜㅠ'
        초성 4벌 : 받침없는 'ㅘㅙㅚㅢ'
        초성 5벌 : 받침없는 'ㅝㅞㅟ'
        초성 6벌 : 받침있는 'ㅏㅐㅑㅒㅓㅔㅕㅖㅣ' 와 결합
        초성 7벌 : 받침있는 'ㅗㅛㅜㅠㅡ'
        초성 8벌 : 받침있는 'ㅘㅙㅚㅢㅝㅞㅟ'

     중성
        중성 1벌 : 받침없는 'ㄱㅋ' 와 결합
        중성 2벌 : 받침없는 'ㄱㅋ' 이외의 자음
        중성 3벌 : 받침있는 'ㄱㅋ' 와 결합
        중성 4벌 : 받침있는 'ㄱㅋ' 이외의 자음

     종성
        종성 1벌 : 중성 'ㅏㅑㅘ' 와 결합
        종성 2벌 : 중성 'ㅓㅕㅚㅝㅟㅢㅣ'
        종성 3벌 : 중성 'ㅐㅒㅔㅖㅙㅞ'
        종성 4벌 : 중성 'ㅗㅛㅜㅠㅡ'
    */

    if (!last)
    {
        // 받침 없는 경우
        firstType = cho[mid];
        if (first == 1 || first == 24)
            midType = 0;
        else
            midType = 1;
    }
    else
    {
        // 받침 있는 경우
        firstType = cho2[mid];
        if (first == 1 || first == 24)
            midType = 2;
        else
            midType = 3;
        lastType = jong[mid];
    }
    memset(HANFontImage, 0, 32);

    // 초성
    pB = HANFontImage;
    pF = (uint8_t *)KSFont + (firstType * 20 + first) * 32;

    i = 32;
    while (i--)
        *pB++ = pgm_read_byte(pF++);

    // 중성
    pB = HANFontImage;
    pF = (uint8_t *)KSFont + (8 * 20 + midType * 22 + mid) * 32;

    i = 32;
    while (i--)
        *pB++ |= pgm_read_byte(pF++);

    // 종성
    if (last)
    {
        pB = HANFontImage;
        pF = (uint8_t *)KSFont + (((8 * 20) + (4 * 22) + (lastType * 28) + last) * 32);
        i = 32;
        while (i--)
            *pB++ |= pgm_read_byte(pF++);
    }
    return HANFontImage;
}

// #define POINT_FONT_SIZE 2
#define POINT_FONT_SIZE 3

/**
 * @brief buf :32 => 2배 스케일링 32 *4 = 128
 *
 * @param buf
 * @param source
 * @param size : 32,16(입력)
 */
void x2scale(uint8_t *buf, uint8_t *source, uint32_t size)
{
    // 4*32
    uint32_t cnt = 0;
    uint8_t s1, s2;

    for (int x = 0; x < size; x++)
    {
        //ESP_LOGI(TAG, "[%d]%x %x", x, source[x], source[x + 1]);
        x++;
    }

    for (int i = 0; i < size; i++)
    {
        s1 = source[i];
        for (int j = 0; j < 8; j++)
        {
            if (j < 4)
            {
                if ((s1 << j) & 0x80)
                {
                    buf[cnt] |= (0xc0 >> (j * 2));
                }
            }
            else
            {
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 1] |= (0xc0 >> ((j - 4) * 2));
                }
            }
        }
        i++;
        s2 = source[i];
        for (int j = 0; j < 8; j++)
        {
            if (j < 4)
            {
                if ((s2 << j) & 0x80)
                {
                    buf[cnt + 2] |= (0xc0 >> (j * 2));
                }
            }
            else
            {
                if ((s2 << j) & 0x80)
                {
                    buf[cnt + 3] |= (0xc0 >> ((j - 4) * 2));
                }
            }
        }
        buf[cnt + 4] = buf[cnt];
        buf[cnt + 5] = buf[cnt + 1];
        buf[cnt + 6] = buf[cnt + 2];
        buf[cnt + 7] = buf[cnt + 3];
        ////ESP_LOGI(TAG, "[%d]%x %x : %x %x %x %x / %x %x %x %x", cnt, source[i - 1], source[i], buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        cnt += 8;
    }
}

void x2scale_16(uint8_t *buf, uint8_t *source, uint32_t size)
{
    // 4*32
    uint32_t cnt = 0;
    uint8_t s1;

    for (int x = 0; x < size; x++)
    {
        //ESP_LOGI(TAG, "[%d]%x", x, source[x]);
    }
    for (int i = 0; i < size; i++)
    {
        s1 = source[i];
        for (int j = 0; j < 8; j++)
        {
            if (j < 4)
            {
                if ((s1 << j) & 0x80)
                {
                    buf[cnt] |= (0xc0 >> (j * 2));
                }
            }
            else
            {
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 1] |= (0xc0 >> ((j - 4) * 2));
                }
            }
        }

        buf[cnt + 2] = buf[cnt];
        buf[cnt + 3] = buf[cnt + 1];
        ////ESP_LOGI(TAG, "[%d]%x %x : %x %x %x %x / %x %x %x %x", cnt, source[i - 1], source[i], buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        cnt += 4;
    }
}

#if 1
void x3scale(uint8_t *buf, uint8_t *source, uint32_t size)
{
    // 4*32
    uint32_t cnt = 0;
    uint8_t s1, s2;
    int i, j;

    // for (int x = 0; x < size; x++)
    // {
    //     //ESP_LOGI(TAG, "[%d]%x %x", x, source[x], source[x + 1]);
    //     x++;
    // }

    for (i = 0; i < size; i++)
    {
        s1 = source[i];
        for (j = 0; j < 8; j++)
        {
            switch (j)
            {
            case 0:
            case 1:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt] |= (0xe0 >> (j * 3));
                }
                break;
            case 2:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt] |= 0x03;
                    buf[cnt + 1] |= 0x80;
                }
                break;
            case 3:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 1] |= 0x70;
                }
                break;
            case 4:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 1] |= 0x0e;
                }
                break;
            case 5:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 1] |= 0x01;
                    buf[cnt + 2] |= 0xc0;
                }
                break;
            case 6:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 2] |= 0x38;
                }
                break;
            case 7:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 2] |= 0x07;
                }
                break;
            }
        }
        i++;
        s2 = source[i];
        for (j = 0; j < 8; j++)
        {
            switch (j)
            {
            case 0:
            case 1:
                if ((s2 << j) & 0x80)
                {
                    buf[cnt + 3] |= (0xe0 >> (j * 3));
                }
                break;
            case 2:
                if ((s2 << j) & 0x80)
                {
                    buf[cnt + 3] |= 0x03;
                    buf[cnt + 4] |= 0x80;
                }
                break;
            case 3:
                if ((s2 << j) & 0x80)
                {
                    buf[cnt + 4] |= 0x70;
                }
                break;
            case 4:
                if ((s2 << j) & 0x80)
                {
                    buf[cnt + 4] |= 0x0e;
                }
                break;
            case 5:
                if ((s2 << j) & 0x80)
                {
                    buf[cnt + 4] |= 0x01;
                    buf[cnt + 5] |= 0xc0;
                }
                break;
            case 6:
                if ((s2 << j) & 0x80)
                {
                    buf[cnt + 5] |= 0x38;
                }
                break;
            case 7:
                if ((s2 << j) & 0x80)
                {
                    buf[cnt + 5] |= 0x07;
                }
                break;
            }
        }
        buf[cnt + 12] = buf[cnt + 6] = buf[cnt];
        buf[cnt + 13] = buf[cnt + 7] = buf[cnt + 1];
        buf[cnt + 14] = buf[cnt + 8] = buf[cnt + 2];
        buf[cnt + 15] = buf[cnt + 9] = buf[cnt + 3];
        buf[cnt + 16] = buf[cnt + 10] = buf[cnt + 4];
        buf[cnt + 17] = buf[cnt + 11] = buf[cnt + 5];
        // //ESP_LOGI(TAG, "[%d]%x %x : %x %x %x %x / %x %x %x %x", cnt, source[i - 1], source[i], buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        cnt += 18;
    }
}

void x3scale_16(uint8_t *buf, uint8_t *source, uint32_t size)
{
    // 4*32
    uint32_t cnt = 0;
    uint8_t s1;

    // for (int x = 0; x < size; x++)
    // {
    //     //ESP_LOGI(TAG, "[%d]%x", x, source[x]);
    // }
    for (int i = 0; i < size; i++)
    {
        s1 = source[i];
        for (int j = 0; j < 8; j++)
        {
            switch (j)
            {
            case 0:
            case 1:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt] |= (0xe0 >> (j * 3));
                }
                break;
            case 2:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt] |= 0x03;
                    buf[cnt + 1] |= 0x80;
                }
                break;
            case 3:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 1] |= 0x70;
                }
                break;
            case 4:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 1] |= 0x0e;
                }
                break;
            case 5:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 1] |= 0x01;
                    buf[cnt + 2] |= 0xc0;
                }
                break;
            case 6:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 2] |= 0x38;
                }
                break;
            case 7:
                if ((s1 << j) & 0x80)
                {
                    buf[cnt + 2] |= 0x07;
                }
                break;
            }
        }

        buf[cnt + 6] = buf[cnt + 3] = buf[cnt];
        buf[cnt + 7] = buf[cnt + 4] = buf[cnt + 1];
        buf[cnt + 8] = buf[cnt + 5] = buf[cnt + 2];
        // //ESP_LOGI(TAG, "[%d]%x %x : %x %x %x %x / %x %x %x %x", cnt, source[i - 1], source[i], buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        cnt += 9;
    }
}

#endif

/**
 * @brief Get the Hangle Mem object
 *
 * @param position
 * @return uint32_t
 */
uint32_t getHangleMem(uint8_t position)
{
    uint32_t ret = 0;

    switch (position)
    {
    case HG_MEM_WORKER:
        ret = mem_hangle_font;
        break;
    case HG_MEM_WORK_NAME:
        ret = mem_hangle_font + 2048; /* (50/3)*128*/
        break;
    case HG_MEM_MSG1:
        ret = mem_hangle_font + 4096; /* (50/3)*128*/
        break;
    default:
        ret = mem_hangle_font;
        break;
    }
    return ret;
}

/**
 * @brief G메모리에 한글 이미지 쓰기
 *
 * @param pChar hangle 50byte
 * @param position worker_name, work_name, message
 */
void hangle_mem_write(char *pChar, uint8_t position)
{
    uint8_t *pFs;
    uint8_t pBuf[300] = {
        0,
    };

    uint8_t c, c2, c3;
    uint32_t char_h_pos = 0;
    uint32_t hg_cnt = 0;
    uint32_t hfont_addr;

    hfont_addr = getHangleMem(position);

    while (*pChar)
    {
        c = *(uint8_t *)pChar++;

        // /r 개행 문자인지 확인
        if (c == 0x2F && *(uint8_t *)pChar++ == 0x72)
        {
            // XPOS = 0;
            // YPOS = YPOS + (16 * POINT_FONT_SIZE);
        }
        //---------- 한글 ---------
        else if (c >= 0x80)
        {
            c2 = *(uint8_t *)pChar++;
            c3 = *(uint8_t *)pChar++;

            pFs = getHAN_font(c, c2, c3);
            memset(&pBuf[0], 0, 256);
            // x2scale(&pBuf[0], pFs, 32);
            // EVE_memWrite_buffer(hfont_addr + hg_cnt, &pBuf[0], 128);
            x3scale(&pBuf[0], pFs, 32);
            EVE_memWrite_buffer(hfont_addr + hg_cnt, &pBuf[0], 288);
            spi_wait_for_pending_transactions();
            // hg_cnt += 128;
            // char_h_pos += 32;
            hg_cnt += 288;
            char_h_pos += 48;
        }
        //---------- ASCII ---------
        else
        {
            pFs = (uint8_t *)ASCfontSet + ((uint8_t)c - 0x20) * 16;
            memset(&pBuf[0], 0, 256);
            // x2scale_16(&pBuf[0], pFs, 16);
            // EVE_memWrite_buffer(hfont_addr + hg_cnt, &pBuf[0], 64);
            x3scale_16(&pBuf[0], pFs, 16);
            EVE_memWrite_buffer(hfont_addr + hg_cnt, &pBuf[0], 144);
            spi_wait_for_pending_transactions();
            // hg_cnt += 64;
            // char_h_pos += 16;
            hg_cnt += 144;
            char_h_pos += 24;
        }
    }
}

void drawHangle(uint32_t XPOS, uint32_t YPOS, char *pChar, uint32_t color, uint8_t position)
{
    uint8_t c;
    uint32_t char_h_pos = 0;
    uint32_t hg_cnt = 0;
    uint32_t hfont_addr;

    hfont_addr = getHangleMem(position);

    while (*pChar)
    {
        c = *(uint8_t *)pChar++;
        // /r 개행 문자인지 확인
        // EVE_cmd_bgcolor(color);
        if (c == 0x2F && *(uint8_t *)pChar++ == 0x72)
        {
            XPOS = 0;
            YPOS = YPOS + (16 * POINT_FONT_SIZE);
        }
        //---------- 한글 ---------
        else if (c >= 0x80)
        {
            c = *(uint8_t *)pChar++;
            c = *(uint8_t *)pChar++;

            // EVE_cmd_setbitmap(hfont_addr + hg_cnt, EVE_L1, 32, 32);
            EVE_cmd_setbitmap(hfont_addr + hg_cnt, EVE_L1, 48, 48);
            EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
            EVE_cmd_dl(VERTEX_FORMAT(0));
            EVE_cmd_dl(VERTEX2F(XPOS + char_h_pos, YPOS));
            EVE_cmd_dl(DL_END);
            hg_cnt += 288;
            char_h_pos += 48;
        }
        //---------- ASCII ---------
        else
        {
            // EVE_cmd_setbitmap(hfont_addr + hg_cnt, EVE_L1, 16, 32);
            EVE_cmd_setbitmap(hfont_addr + hg_cnt, EVE_L1, 24, 48);
            EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
            EVE_cmd_dl(VERTEX_FORMAT(0));
            EVE_cmd_dl(VERTEX2F(XPOS + char_h_pos, YPOS));
            EVE_cmd_dl(DL_END);
            hg_cnt += 144;
            char_h_pos += 24;
        }
    }
    // //ESP_LOGI(TAG,"haddr = 0x%x",hfont_addr);
}

#if 0
void matrixPrint2(uint32_t XPOS, uint32_t YPOS, char *pChar, uint32_t color, uint8_t position)
{
    // uint8_t rg = 3; //<b1> red, <b0> green
    uint8_t *pFs;
    uint8_t pBuf[256] = {
        0,
    };

    uint8_t c, c2, c3;
    uint32_t char_h_pos = 0;
    uint32_t hg_cnt = 0;
    uint32_t hfont_addr;

    while (*pChar)
    {
        c = *(uint8_t *)pChar++;

        // /r 개행 문자인지 확인
        EVE_color_rgb(255, 255, 255);
        if (c == 0x2F && *(uint8_t *)pChar++ == 0x72)
        {
            XPOS = 0;
            YPOS = YPOS + (16 * POINT_FONT_SIZE);
        }
        //---------- 한글 ---------
        else if (c >= 0x80)
        {
            c2 = *(uint8_t *)pChar++;
            c3 = *(uint8_t *)pChar++;

            pFs = getHAN_font(c, c2, c3);
            memset(&pBuf[0], 0, 256);
            x2scale(&pBuf[0], pFs, 32);
            EVE_memWrite_buffer(hfont_addr + hg_cnt, &pBuf[0], 128);
            DELAY_MS(10);
            EVE_cmd_setbitmap(hfont_addr + hg_cnt, EVE_L1, 32, 32);
            EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
            EVE_cmd_dl(VERTEX_FORMAT(0));
            EVE_cmd_dl(VERTEX2F(XPOS + char_h_pos, YPOS));
            EVE_cmd_dl(DL_END);
            // hg_cnt += 32;
            hg_cnt += 128;
            // char_h_pos += 16;
            char_h_pos += 32;
        }
        //---------- ASCII ---------
        else
        {
            pFs = (uint8_t *)ASCfontSet + ((uint8_t)c - 0x20) * 16;
            // memcpy(&pBuf[0], pFs, 16);
            memset(&pBuf[0], 0, 256);
            x2scale(&pBuf[0], pFs, 16);
            // EVE_memWrite_buffer(hfont_addr + hg_cnt, &pBuf[0], 16);
            EVE_memWrite_buffer(hfont_addr + hg_cnt, &pBuf[0], 64);
            DELAY_MS(10);
            EVE_cmd_setbitmap(hfont_addr + hg_cnt, EVE_L1, 16, 32);
            EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
            EVE_cmd_dl(VERTEX_FORMAT(0));
            EVE_cmd_dl(VERTEX2F(XPOS + char_h_pos, YPOS));
            EVE_cmd_dl(DL_END);
            // hg_cnt += 16;
            hg_cnt += 64;
            char_h_pos += 16;
        }
    }
    // DELAY_MS(500); // debug
}

void matrixPrint(uint32_t XPOS, uint32_t YPOS, char *pChar, uint32_t color, uint8_t position)
{
    // uint8_t rg = 3; //<b1> red, <b0> green
    uint8_t *pFs;
    uint8_t pBuf[256] = {
        0,
    };
    uint8_t c, c2, c3;
    uint32_t char_h_pos = 0;
    uint32_t hg_cnt = 0;
    uint32_t hfont_addr;

    if (position)
    {
        hfont_addr = mem_hangle_font;
    }
    else
    {
        hfont_addr = mem_hangle_font + 1600;
    }

    while (*pChar)
    {
        c = *(uint8_t *)pChar++;

        // /r 개행 문자인지 확인
        EVE_color_rgb(255, 255, 255);
        if (c == 0x2F && *(uint8_t *)pChar++ == 0x72)
        {
            XPOS = 0;
            YPOS = YPOS + (16 * POINT_FONT_SIZE);
        }
        //---------- 한글 ---------
        else if (c >= 0x80)
        {
            c2 = *(uint8_t *)pChar++;
            c3 = *(uint8_t *)pChar++;

            pFs = getHAN_font(c, c2, c3);
            memcpy(&pBuf[0], pFs, 32);
            EVE_memWrite_buffer(hfont_addr + hg_cnt, &pBuf[0], 32);
            EVE_cmd_setbitmap(hfont_addr + hg_cnt, EVE_L1, 16, 16);
            EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
            EVE_cmd_dl(VERTEX_FORMAT(0));
            EVE_cmd_dl(VERTEX2F(XPOS + char_h_pos, YPOS));
            EVE_cmd_dl(DL_END);
            hg_cnt += 32;
            char_h_pos += 16;
        }
        //---------- ASCII ---------
        else
        {
            pFs = (uint8_t *)ASCfontSet + ((uint8_t)c - 0x20) * 16;
            memcpy(&pBuf[0], pFs, 16);
            EVE_memWrite_buffer(hfont_addr + hg_cnt, &pBuf[0], 16);
            EVE_cmd_setbitmap(hfont_addr + hg_cnt, EVE_L1, 8, 16);
            EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
            EVE_cmd_dl(VERTEX_FORMAT(0));
            EVE_cmd_dl(VERTEX2F(XPOS + char_h_pos, YPOS));
            EVE_cmd_dl(DL_END);
            hg_cnt += 16;
            char_h_pos += 8;
        }
    }
}


static uint8_t c1;  // Last character buffer

uint8_t utf8ascii(uint8_t ascii) {
    if ( ascii<128 )   // Standard ASCII-set 0..0x7F handling  
    {   c1=0;
        return( ascii );
    }

    // get previous input
    uint8_t last = c1;   // get last char
    c1=ascii;         // remember actual character

    switch (last)     // conversion depnding on first UTF8-character
    {   case 0xC2: return  (ascii);  break;
        case 0xC3: return  (ascii | 0xC0);  break;
        case 0x82: if(ascii==0xAC) return(0x80);       // special case Euro-symbol
    }

    return  (0);                                     // otherwise: return zero, if character has to be ignored
}

void utf8ascii(char* s)
{      
        int k=0;
        char c;
        for (int i=0; i<strlen(s); i++)
        {
                c = utf8ascii(s[i]);
                if (c!=0)
                        s[k++]=c;
        }
        s[k]=0;
}

/* sample app structure definitions */
typedef struct SAMAPP_Bitmap_header
{
	ft_uint8_t Format;
	ft_int16_t Width;
	ft_int16_t Height;
	ft_int16_t Stride;
	ft_int32_t Arrayoffset;
}SAMAPP_Bitmap_header_t;

unsigned char gao[] = {0, 0x30, 0, 0, 0x18, 0, 0, 0x10, 0x0c, 0x7f, 0xff, 0xfe, 0, 0, 0, 0x02, 0x01, 0x80, 0x03, 0xff, 0xc0, 0x03, 0x01, 0x80, 0x03, 0x01, 0x80, 0x03, 0xff, 0x80, 0x02, 0x01, 0, 0x20, 0, 0x18, 0x3f, 0xff, 0xfc, 0x30, 0, 0x18, 0x31, 0x03, 0x18, 0x31, 0xff, 0x98, 0x31, 0x83, 0x18, 0x31, 0x83, 0x18, 0x31, 0xff, 0x18, 0x31, 0x82, 0x18, 0x31, 0, 0x18, 0x30, 0, 0xf8, 0x30, 0, 0x30, 0x20, 0, 0x20};
SAMAPP_Bitmap_header_t SAMAPP_Bitmap_RawData_Header[] =
    {
        /* format,width,height,stride,arrayoffset */
        {RGB565, 40, 40, 40 * 2, 0},
        {PALETTED, 40, 40, 40, 0},
        {PALETTED, 480, 272, 480, 0},
        {L1, 24, 24, 3, 0},
};

ft_void_t SAMAPP_GPU_BitmapFont()
{
    SAMAPP_Bitmap_header_t *p_bmhdr;
    ft_int16_t BMoffsetx, BMoffsety;
    p_bmhdr = (SAMAPP_Bitmap_header_t *)&SAMAPP_Bitmap_RawData_Header[3];
    /* Copy raw data into address 0 followed by generation of bitmap */
    Ft_Gpu_Hal_WrMemFromFlash(phost, RAM_G, &gao[p_bmhdr->Arrayoffset], p_bmhdr->Stride * p_bmhdr->Height);
    Ft_App_WrDlCmd_Buffer(phost, CLEAR(1, 1, 1)); // clear screen
    Ft_App_WrDlCmd_Buffer(phost, COLOR_RGB(255, 255, 255));
    Ft_App_WrDlCmd_Buffer(phost, BITMAP_SOURCE(RAM_G));
    Ft_App_WrDlCmd_Buffer(phost, BITMAP_LAYOUT(p_bmhdr->Format, p_bmhdr->Stride, p_bmhdr->Height));
    Ft_App_WrDlCmd_Buffer(phost, BITMAP_SIZE(NEAREST, BORDER, BORDER, p_bmhdr->Width, p_bmhdr->Height));
    Ft_App_WrDlCmd_Buffer(phost, BEGIN(BITMAPS)); // start drawing bitmaps
    BMoffsetx = ((FT_DispWidth / 4) - (p_bmhdr->Width / 2));
    BMoffsety = ((FT_DispHeight / 2) - (p_bmhdr->Height / 2));
    Ft_App_WrDlCmd_Buffer(phost, VERTEX2II(BMoffsetx, BMoffsety, 0, 0));
    Ft_App_WrDlCmd_Buffer(phost, COLOR_RGB(255, 64, 64)); // red at (200, 120)
    BMoffsetx = ((FT_DispWidth * 2 / 4) - (p_bmhdr->Width / 2));
    BMoffsety = ((FT_DispHeight / 2) - (p_bmhdr->Height / 2));
    Ft_App_WrDlCmd_Buffer(phost, VERTEX2II(BMoffsetx, BMoffsety, 0, 0));
    Ft_App_WrDlCmd_Buffer(phost, COLOR_RGB(64, 180, 64)); // green at (216, 136)
    BMoffsetx += (p_bmhdr->Width / 2);
    BMoffsety += (p_bmhdr->Height / 2);
    Ft_App_WrDlCmd_Buffer(phost, VERTEX2II(BMoffsetx, BMoffsety, 0, 0));
    Ft_App_WrDlCmd_Buffer(phost, COLOR_RGB(255, 255, 64)); // transparent yellow at (232, 152)
    Ft_App_WrDlCmd_Buffer(phost, COLOR_A(150));
    BMoffsetx += (p_bmhdr->Width / 2);
    BMoffsety += (p_bmhdr->Height / 2);
    Ft_App_WrDlCmd_Buffer(phost, VERTEX2II(BMoffsetx, BMoffsety, 0, 0));
    Ft_App_WrDlCmd_Buffer(phost, COLOR_A(255));
    Ft_App_WrDlCmd_Buffer(phost, COLOR_RGB(255, 255, 255));
    Ft_App_WrDlCmd_Buffer(phost, VERTEX2F(-10 * 16, -10 * 16)); // for -ve coordinates use
    Ft_App_WrDlCmd_Buffer(phost, END());
    Ft_App_WrDlCmd_Buffer(phost, DISPLAY());
    /* Download the DL into DL RAM */
    Ft_App_Flush_DL_Buffer(phost);
    /* Do a swap */
    SAMAPP_GPU_DLSwap(DLSWAP_FRAME);
    SAMAPP_ENABLE_DELAY();
}

#endif