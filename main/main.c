
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include "ads1232.h"
#include "eve/EVE_commands.h"
#include "tft.h"
#include "scspiffs.h"
#include "mfrc522.h"
#include "wifi_station.h"
#include "sntp.h"
#include "http.h"
#include "serialLED.h"
#include "intro.h"
#include "ui_wifi_scan.h"
#include "ui_fail.h"
#include "ui_cali.h"

#ifndef CONFIG_USE_WIFI
#define CONFIG_USE_WIFI 1
#endif

const char *ver_str = "Version: 10.00"; /* 히스토리는 intro.h */
static const char *TAG = "main";
int32_t _1sec_count = 0;

extern ads1232_t adv1232;
extern float gVal;

// #define EVE_PDN 25 // 35 (4번핀) input only 설계 잘못됨. 2022.4.15. pullup 상태이므로 일단 그냥둠

void initSystem(void);
void guiHandler(void *pvParameter);
void loadCellHandler(void *pvParameter);
void tag_handler(uint8_t *serial_no);
void timer_init(void);
static void periodic_timer_callback(void *arg);

// static void oneshot_timer_callback(void *arg);
/**
 * @brief gui fuction
 *
 */

SCALE_SCENARIO currentScenario = NO_SCENARIO;
SCALE_SCENARIO newScenario = NO_SCENARIO;

uint8_t PrevTouchTag = NUM_NONE;

void (*CurrentScreenLoop)() = 0;
void (*CurrentScreenCloseFunction)() = 0;
// #define _RS522_SPI_
void app_main(void)
{

#if 1
    const rc522_start_args_t rc522_args = {
        .miso_io = RC522_DEFAULT_MISO,
        .mosi_io = RC522_DEFAULT_MOSI,
        .sck_io = RC522_DEFAULT_SCK,
        .sda_io = RC522_DEFAULT_SDA,
        .callback = &tag_handler};
#endif

    initSystem();
    spi_init();
    //initUART1(); /* mfrc522 용 */
    TFT_init();
    spiffs_init();
    /*
    dout: GPIO_NUM_21
    sclk: GPIO_NUM_22
    pdwn: GPIO_NUM_15
    speed: 
    */
    
    timer_init();
    led_strip_install();

    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_2, GPIO_PULLUP_ONLY);
    gpio_pullup_en(GPIO_NUM_2);

    if (scaleInfo.ethernetConnect)
    {
    #if CONFIG_USE_SPI_ETHERNET
        eth_test_task();
        ESP_LOGI(TAG, "ethernet start!");
    #endif
    }
    else
    {
    #if CONFIG_USE_WIFI
        xTaskCreate(&wifiTask, "wifiTask", 8192, NULL, 3, NULL);
        ESP_LOGI(TAG, "wifi start!");
    #endif
    }

#if 1
    rc522_start(rc522_args);
#endif

    //xTaskCreate(&rfid_seial_task, "rfid_seial_task", 2018 * 5, NULL, 5, NULL); /* mfrc522 serial */
    xTaskCreate(&sLedTask, "sLedTask", configMINIMAL_STACK_SIZE * 5, NULL, 1, NULL);
    xTaskCreate(&guiHandler, "guiHandler", 8192, NULL, 5, NULL);
    xTaskCreate(&loadCellHandler, "loadCellHandler", 4096, NULL, 2, NULL);
    xTaskCreate(&sntp_task, "sntp_task", 4096, NULL, 3, NULL);
    xTaskCreate(&http_task, "http_task", 32768, NULL, 3, NULL);

}

/**
 * @brief
 *
 */
void initSystem(void)
{
    /* 전역 플레그 초기화 */
    memset(taskInfo.worker_name, 0, sizeof(taskInfo.worker_name));
    taskInfo.workerRegister = 0;
    scaleInfo.ethernetConnect = 0; // wifi
    taskInfo.bowl_weight = 0;
    taskInfo.seqChangeFlag = 0;
    taskInfo.cheatingTimeout = DEFAULT_CHATTING_TIMEOUT;
    taskInfo.numAutoZero = DEFAULT_NUM_AUTO_SERO;
    taskInfo.numStableBuffer = DEFAULT_NUM_STABLE_BUFFER;
    taskInfo.zero_cali_band = DEFAULT_ZERO_CALI_BAND;
    taskInfo.current_product_cnt = 0;
}

static void periodic_timer_callback(void *arg)
{
    // int64_t time_since_boot = esp_timer_get_time();
    //ESP_LOGD(TAG, "Periodic timer called, time since boot: %lld us", time_since_boot);
    _1sec_count++;
}

// static void oneshot_timer_callback(void *arg)
// {
//     int64_t time_since_boot = esp_timer_get_time();
//     //ESP_LOGI(TAG, "One-shot timer called, time since boot: %lld us", time_since_boot);
//     esp_timer_handle_t periodic_timer_handle = (esp_timer_handle_t)arg;
//     /* To start the timer which is running, need to stop it first */
//     ESP_ERROR_CHECK(esp_timer_stop(periodic_timer_handle));
//     ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer_handle, 1000000));
//     time_since_boot = esp_timer_get_time();
//     //ESP_LOGI(TAG, "Restarted periodic timer with 1s period, time since boot: %lld us",
//              time_since_boot);
// }

/**
 * @brief
 *
 */
void timer_init(void)
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        /* name is optional, but may help identify the timer when debugging */
        .name = "periodic"};

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    /* The timer has been created but is not running yet */
    // const esp_timer_create_args_t oneshot_timer_args = {
    //     .callback = &oneshot_timer_callback,
    //     /* argument specified here will be passed to timer callback function */
    //     .arg = (void *)periodic_timer,
    //     .name = "one-shot"};
    // esp_timer_handle_t oneshot_timer;
    // ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_timer));

    /* Start the timers */
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000)); // 0.1ms 단위 1초
    // ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer, 5000000));
    //ESP_LOGI(TAG, "Started timers, time since boot: %lld us", esp_timer_get_time());

#if 0
    /* Clean up and finish the example */
    ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
    ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));
    //ESP_LOGI(TAG, "Stopped and deleted timers");
#endif
}

/*
    @guiHandler
*/
void guiHandler(void *pvParameter)
{
    // introDisplay();
    touch_calibrate();
    //ESP_LOGI(TAG, "gui start");
    // CurrentScreenLoop = introGuiStart(IntroTouchCallback, &CurrentScreenCloseFunction);
    CurrentScreenLoop = introGuiStart(IntroTouchCallback);
    currentScenario = newScenario = INTRO_SCENARIO;

    for (;;)
    {
        if (CurrentScreenLoop != 0)
            (*CurrentScreenLoop)();
        if (newScenario != currentScenario)
        {
            //ESP_LOGI(TAG, "newScenario = %d(%d):%d", newScenario, currentScenario, _1sec_count);
            if (CurrentScreenCloseFunction != 0)
                (*CurrentScreenCloseFunction)();

            switch (newScenario)
            {
            case MAIN_SCALE_SCENARIO:
                // CurrentScreenLoop = scaleHome(guiHomeTouchCallback, &CurrentScreenCloseFunction);
                CurrentScreenLoop = scaleHome(guiHomeTouchCallback);
                break;
            case FAILWEIGHT_SCENARIO:
                CurrentScreenLoop = ui_fail_start(failWeightCallback);
                break;
            case WIFI_SCAN_SCNARIO:
                CurrentScreenLoop = ui_wifi_scan_start(wifiScanListCallback);
                break;
            case WIFI_KDB_SCNARIO:
                CurrentScreenLoop = keyBoardStart(wifi_kbd_Callback);
                break;
            case MSG_INIT_SCNARIO:
                CurrentScreenLoop = ui_init_start(initCallback);
                break;
            case MSG_UNREG_SCNARIO:
                CurrentScreenLoop = msg_unreg_start(unReg_Callback);
                break;
            case MSG_START_SCNARIO:
                CurrentScreenLoop = msg_start_start(start_Callback);
                break;
            case CALIBRATION_SCNARIO:
                CurrentScreenLoop = ui_cali_start(caliCallback);
                break;
            default:
                break;
            }
            currentScenario = newScenario;
        }
        //ESP_LOGI(TAG, "t: %d %lld", _1sec_count,esp_timer_get_time());
        DELAY_MS(50);
    }
}

void loadCellHandler(void *pvParameter)
{
    // hx711_Task();
    ads1232_task();
    
}

#if 1
void tag_handler(uint8_t *serial_no)
{
    if(taskInfo.work_step < W_TAGGING)
    {
        taskInfo.tag_flag = 1;
    }
    for (int i = 0; i < 5; i++)
    {
        //taskInfo.LED_step = 10;
        taskInfo.tag_serial[i] = serial_no[i];
        //ESP_LOGI(TAG, "0x%x", serial_no[i]);
    }
    printf("\n");
}
#endif
