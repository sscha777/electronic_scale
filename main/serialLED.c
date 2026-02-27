/**
 * @file serialLED.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-05-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#define CONFIG_LED_STRIP_FLUSH_TIMEOUT 1000
#define CONFIG_LED_STRIP_PAUSE_LENGTH 50

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "serialLED.h"
#include <esp_log.h>
#include <esp_attr.h>
#include <stdlib.h>
#include "eve/EVE_commands.h"
#include "../esp-idf-lib/components/esp_idf_lib_helpers/ets_sys.h"
#include "../esp-idf-lib/components/esp_idf_lib_helpers/esp_idf_lib_helpers.h"
#include "http.h"
static const char *TAG = "serialLED";

#define LED_STRIP_RMT_CLK_DIV 2

#define WS2812_T0H_NS 400
#define WS2812_T0L_NS 1000
#define WS2812_T1H_NS 1000
#define WS2812_T1L_NS 400

#define SK6812_T0H_NS 300
#define SK6812_T0L_NS 900
#define SK6812_T1H_NS 600
#define SK6812_T1L_NS 600

#define APA106_T0H_NS 350
#define APA106_T0L_NS 1360
#define APA106_T1H_NS 1360
#define APA106_T1L_NS 350

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

#define COLOR_SIZE(strip) (3 + ((strip)->is_rgbw != 0))

static rmt_item32_t ws2812_bit0 = {0};
static rmt_item32_t ws2812_bit1 = {0};
static rmt_item32_t sk6812_bit0 = {0};
static rmt_item32_t sk6812_bit1 = {0};
static rmt_item32_t apa106_bit0 = {0};
static rmt_item32_t apa106_bit1 = {0};

static void IRAM_ATTR _rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
                                   size_t wanted_num, size_t *translated_size, size_t *item_num,
                                   const rmt_item32_t *bit0, const rmt_item32_t *bit1)
{
    if (!src || !dest)
    {
        *translated_size = 0;
        *item_num = 0;
        return;
    }
    size_t size = 0;
    size_t num = 0;
    uint8_t *psrc = (uint8_t *)src;
    rmt_item32_t *pdest = dest;
#ifdef LED_STRIP_BRIGHTNESS
    led_strip_t *strip;
    esp_err_t r = rmt_translator_get_context(item_num, (void **)&strip);
    uint8_t brightness = r == ESP_OK ? strip->brightness : 255;
#endif
    while (size < src_size && num < wanted_num)
    {
#ifdef LED_STRIP_BRIGHTNESS
        uint8_t b = brightness != 255 ? scale8_video(*psrc, brightness) : *psrc;
#else
        uint8_t b = *psrc;
#endif
        for (int i = 0; i < 8; i++)
        {
            // MSB first
            pdest->val = b & (1 << (7 - i)) ? bit1->val : bit0->val;
            num++;
            pdest++;
        }
        size++;
        psrc++;
    }
    *translated_size = size;
    *item_num = num;
}

static void IRAM_ATTR ws2812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
                                         size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    _rmt_adapter(src, dest, src_size, wanted_num, translated_size, item_num, &ws2812_bit0, &ws2812_bit1);
}

static void IRAM_ATTR sk6812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
                                         size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    _rmt_adapter(src, dest, src_size, wanted_num, translated_size, item_num, &sk6812_bit0, &sk6812_bit1);
}

static void IRAM_ATTR apa106_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
                                         size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    _rmt_adapter(src, dest, src_size, wanted_num, translated_size, item_num, &apa106_bit0, &apa106_bit1);
}

///////////////////////////////////////////////////////////////////////////////

void led_strip_install()
{
    float ratio = (float)(APB_CLK_FREQ / LED_STRIP_RMT_CLK_DIV) / 1e09;

    ws2812_bit0.duration0 = ratio * WS2812_T0H_NS;
    ws2812_bit0.level0 = 1;
    ws2812_bit0.duration1 = ratio * WS2812_T0L_NS;
    ws2812_bit0.level1 = 0;
    ws2812_bit1.duration0 = ratio * WS2812_T1H_NS;
    ws2812_bit1.level0 = 1;
    ws2812_bit1.duration1 = ratio * WS2812_T1L_NS;
    ws2812_bit1.level1 = 0;

    sk6812_bit0.duration0 = ratio * SK6812_T0H_NS;
    sk6812_bit0.level0 = 1;
    sk6812_bit0.duration1 = ratio * SK6812_T0L_NS;
    sk6812_bit0.level1 = 0;
    sk6812_bit1.duration0 = ratio * SK6812_T1H_NS;
    sk6812_bit1.level0 = 1;
    sk6812_bit1.duration1 = ratio * SK6812_T1L_NS;
    sk6812_bit1.level1 = 0;

    apa106_bit0.duration0 = ratio * APA106_T0H_NS;
    apa106_bit0.level0 = 1;
    apa106_bit0.duration1 = ratio * APA106_T0L_NS;
    apa106_bit0.level1 = 0;
    apa106_bit1.duration0 = ratio * APA106_T1H_NS;
    apa106_bit1.level0 = 1;
    apa106_bit1.duration1 = ratio * APA106_T1L_NS;
    apa106_bit1.level1 = 0;
}

esp_err_t led_strip_init(led_strip_t *strip)
{
    CHECK_ARG(strip && strip->length > 0 && strip->type < LED_STRIP_TYPE_MAX);

    strip->buf = calloc(strip->length, COLOR_SIZE(strip));
    if (!strip->buf)
    {
        ////ESP_LOGE(TAG, "Not enough memory");
        return ESP_ERR_NO_MEM;
    }

    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(strip->gpio, strip->channel);
    config.clk_div = LED_STRIP_RMT_CLK_DIV;

    CHECK(rmt_config(&config));
    CHECK(rmt_driver_install(config.channel, 0, 0));

    sample_to_rmt_t f = NULL;
    switch (strip->type)
    {
    case LED_STRIP_WS2812:
        f = ws2812_rmt_adapter;
        break;
    case LED_STRIP_SK6812:
        f = sk6812_rmt_adapter;
        break;
    case LED_STRIP_APA106:
        f = apa106_rmt_adapter;
        break;
    default:
        break;
    }
    CHECK(rmt_translator_init(config.channel, f));
#ifdef LED_STRIP_BRIGHTNESS
    // No support for translator context prior to ESP-IDF 4.3
    CHECK(rmt_translator_set_context(config.channel, strip));
#endif

    return ESP_OK;
}

esp_err_t led_strip_free(led_strip_t *strip)
{
    CHECK_ARG(strip && strip->buf);
    free(strip->buf);

    CHECK(rmt_driver_uninstall(strip->channel));

    return ESP_OK;
}

esp_err_t led_strip_flush(led_strip_t *strip)
{
    CHECK_ARG(strip && strip->buf);

    CHECK(rmt_wait_tx_done(strip->channel, pdMS_TO_TICKS(CONFIG_LED_STRIP_FLUSH_TIMEOUT)));
    ets_delay_us(CONFIG_LED_STRIP_PAUSE_LENGTH);
    return rmt_write_sample(strip->channel, strip->buf,
                            strip->length * COLOR_SIZE(strip), false);
}

bool led_strip_busy(led_strip_t *strip)
{
    if (!strip)
        return false;
    return rmt_wait_tx_done(strip->channel, 0) == ESP_ERR_TIMEOUT;
}

esp_err_t led_strip_wait(led_strip_t *strip, TickType_t timeout)
{
    CHECK_ARG(strip);

    return rmt_wait_tx_done(strip->channel, timeout);
}

esp_err_t led_strip_set_pixel(led_strip_t *strip, size_t num, rgb_t color)
{
    CHECK_ARG(strip && strip->buf && num <= strip->length);
    size_t idx = num * COLOR_SIZE(strip);
    switch (strip->type)
    {
    case LED_STRIP_WS2812:
    case LED_STRIP_SK6812:
        // GRB
        strip->buf[idx] = color.g;
        strip->buf[idx + 1] = color.r;
        strip->buf[idx + 2] = color.b;
        if (strip->is_rgbw)
            strip->buf[idx + 3] = rgb_luma(color);
        break;
    case LED_STRIP_APA106:
        // RGB
        strip->buf[idx] = color.r;
        strip->buf[idx + 1] = color.g;
        strip->buf[idx + 2] = color.b;
        if (strip->is_rgbw)
            strip->buf[idx + 3] = rgb_luma(color);
        break;
    default:
        ////ESP_LOGE(TAG, "Unknown strip type %d", strip->type);
        return ESP_ERR_NOT_SUPPORTED;
    }
    return ESP_OK;
}

esp_err_t led_strip_set_pixels(led_strip_t *strip, size_t start, size_t len, rgb_t *data)
{
    CHECK_ARG(strip && strip->buf && len && start + len <= strip->length);
    for (size_t i = 0; i < len; i++)
        CHECK(led_strip_set_pixel(strip, i + start, data[i]));
    return ESP_OK;
}

esp_err_t led_strip_fill(led_strip_t *strip, size_t start, size_t len, rgb_t color)
{
    CHECK_ARG(strip && strip->buf && len && start + len <= strip->length);

    for (size_t i = start; i < start + len; i++)
        CHECK(led_strip_set_pixel(strip, i, color));
    return ESP_OK;
}
/**
 * @brief
 *
 * @param strip
 * @param step +4~-4 +:Red -:blue 0: green 5,6,7,8 : x 9: all off 10: g allon
 * @return esp_err_t
 */
esp_err_t led_step_display(led_strip_t *strip, int8_t step)
{
    static int8_t prev_step = 0;

    if (prev_step != step)
    {
        prev_step = step;
    }
    else
    {
        return ESP_OK;
    }
    const rgb_t data[15][9] = {
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x2f}, {.r = 0x00, .g = 0x00, .b = 0x2f}, {.r = 0x00, .g = 0x00, .b = 0x2f}, {.r = 0x00, .g = 0x00, .b = 0x2f}}, /*0*/
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x2f}, {.r = 0x00, .g = 0x00, .b = 0x2f}, {.r = 0x00, .g = 0x00, .b = 0x2f}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*1*/
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x2f}, {.r = 0x00, .g = 0x00, .b = 0x2f}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*2*/
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x2f}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*3*/

        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x1a, .g = 0xa,  .b = 0x04}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*4*/

        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x2f, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*5*/
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x2f, .g = 0x00, .b = 0x00}, {.r = 0x2f, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*6*/
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x2f, .g = 0x00, .b = 0x00}, {.r = 0x2f, .g = 0x00, .b = 0x00}, {.r = 0x2f, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*7*/
        {{.r = 0x2f, .g = 0x00, .b = 0x00}, {.r = 0x2f, .g = 0x00, .b = 0x00}, {.r = 0x2f, .g = 0x00, .b = 0x00}, {.r = 0x2f, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*8*/
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*9*/
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*10*/
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*11*/
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*12*/
        {{.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}, {.r = 0x00, .g = 0x2f, .b = 0x00}},  /*13*/
        {{.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x1a, .g = 0xa,  .b = 0x04}, {.r = 0x1a, .g = 0xa,  .b = 0x04}, {.r = 0x1a, .g = 0xa,  .b = 0x04}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}, {.r = 0x00, .g = 0x00, .b = 0x00}}, /*14*/


    }; /*10*/

    for (size_t i = 0; i < 9; i++)
    {
        CHECK(led_strip_set_pixel(strip, i, data[step][i]));
    }
    return ESP_OK;
}

/**
 * @brief
 *
 */

#define CONFIG_SERIALLED_GPIO 4
#define LED_TYPE LED_STRIP_SK6812
#define LED_GPIO CONFIG_SERIALLED_GPIO
#define CONFIG_LED_STRIP_LEN 9

// static const rgb_t colors[] = {
//     {.r = 0x0f, .g = 0x0f, .b = 0x0f},
//     {.r = 0x00, .g = 0x00, .b = 0x2f},
//     {.r = 0x00, .g = 0x2f, .b = 0x00},
//     {.r = 0x2f, .g = 0x00, .b = 0x00},
//     {.r = 0x00, .g = 0x00, .b = 0x00},
// };

// #define COLORS_TOTAL (sizeof(colors) / sizeof(rgb_t))

static QueueHandle_t event_queue;

void sendLedDisp(uint8_t led, uint32_t DispDelay)
{
    static uint8_t prev_led = 0;
    static uint32_t prev_delay = 0;
    QLED qLed1;

    if (prev_led != led || prev_delay != DispDelay)
    {
        prev_led = qLed1.led = led;
        prev_delay = qLed1.dispDelay = DispDelay;
        xQueueSend(event_queue, &qLed1, 0);
        //ESP_LOGI("sendLedDisp", "led:%d, delay:%d", led, DispDelay);
    }
}

void sLedTask(void *pvParameters)
{
    led_strip_t strip = {
        .type = LED_TYPE,
        .length = CONFIG_LED_STRIP_LEN,
        .gpio = LED_GPIO,
        .buf = NULL,
        .is_rgbw = 0,
#ifdef LED_STRIP_BRIGHTNESS
        .brightness = 255,
#endif
    };

    // size_t c = 0;
    // int8_t level[9] = {-4, -3, -2, -1, 0, 1, 2, 3, 4};
    int8_t intro[18] = {9, 10, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3};
    uint8_t cnt = 0;
    QLED qLed;
    // uint8_t prevStep = 0;

    event_queue = xQueueCreate(8, sizeof(QLED)); // buffersize 8, 1byte

    ESP_ERROR_CHECK(led_strip_init(&strip));

    for (cnt = 0; cnt < 18; cnt++)
    {
        ESP_ERROR_CHECK(led_step_display(&strip, intro[cnt]));
        ESP_ERROR_CHECK(led_strip_flush(&strip));
        DELAY_MS(200);
    }
    ESP_ERROR_CHECK(led_step_display(&strip, intro[0]));
    ESP_ERROR_CHECK(led_strip_flush(&strip));

    while (1)
    {
        // ESP_ERROR_CHECK(led_strip_fill(&strip, 0, strip.length, colors[c]));
        // ESP_ERROR_CHECK(led_strip_flush(&strip));
        // if (prevStep != taskInfo.LED_step)
        qLed.led = 0;
        qLed.dispDelay = 0;
        if (pdTRUE == xQueueReceive(event_queue, &qLed, portMAX_DELAY))
        {
            // prevStep = taskInfo.LED_step;
            ESP_ERROR_CHECK(led_step_display(&strip, qLed.led));
            ESP_ERROR_CHECK(led_strip_flush(&strip));

            ESP_ERROR_CHECK(led_step_display(&strip, qLed.led));
            ESP_ERROR_CHECK(led_strip_flush(&strip));
            DELAY_MS(qLed.dispDelay);
            //ESP_LOGI(TAG, "qled %d,%d", qLed.led, qLed.dispDelay);
        }
        DELAY_MS(10);
    }
}
