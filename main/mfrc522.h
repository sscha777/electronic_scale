
#if 1

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/spi_master.h"

#define RC522_DEFAULT_MISO (12)
#define RC522_DEFAULT_MOSI (13)
#define RC522_DEFAULT_SCK (14)
#define RC522_DEFAULT_SDA (26)
#define RC522_DEFAULT_SPI_HOST (HSPI_HOST)
#define RC522_DEFAULT_SCAN_INTERVAL_MS (500)
#define RC522_DEFAULT_TACK_STACK_SIZE (4 * 1024)
#define RC522_DEFAULT_TACK_STACK_PRIORITY (4)

typedef void(*rc522_tag_callback_t)(uint8_t*);

typedef struct {
    int miso_io;                    /*<! MFRC522 MISO gpio (Default: 25) */
    int mosi_io;                    /*<! MFRC522 MOSI gpio (Default: 23) */
    int sck_io;                     /*<! MFRC522 SCK gpio  (Default: 19) */
    int sda_io;                     /*<! MFRC522 SDA gpio  (Default: 22) */
    spi_host_device_t spi_host_id;  /*<! Default VSPI_HOST (SPI3) */
    rc522_tag_callback_t callback;  /*<! Scanned tags handler */
    uint16_t scan_interval_ms;      /*<! How fast will ESP32 scan for nearby tags, in miliseconds. Default: 125ms */
    size_t task_stack_size;         /*<! Stack size of rc522 task (Default: 4 * 1024) */
    uint8_t task_priority;          /*<! Priority of rc522 task (Default: 4) */
} rc522_config_t;

typedef rc522_config_t rc522_start_args_t;


/**
 * @brief Initialize RC522 module.
 *        To start scanning tags - call rc522_resume or rc522_start2 function.
 * @param config Configuration
 * @return ESP_OK on success
 */
esp_err_t rc522_init(rc522_config_t* config);

/**
 * @brief Convert serial number (array of 5 bytes) to uint64_t number
 * @param sn Serial number
 * @return Serial number in number representation. If fail, 0 will be retured
 */
uint64_t rc522_sn_to_u64(uint8_t* sn);

/**
 * @brief Check if RC522 is inited
 * @return true if RC522 is inited
 */
bool rc522_is_inited();

/**
 * @brief This function will call rc522_init function and immediately start to scan tags by calling rc522_resume function.
 *        NOTE: This function will be refactored in future to just start scanning without
 *        initialization (same as rc522_resume). For initialization rc522_init will be required to call before this function.
 * @param start_args Configuration
 * @return ESP_OK on success
 */
esp_err_t rc522_start(rc522_start_args_t start_args);

/**
 * @brief Start to scan tags. If already started, ESP_OK will just be returned.
 *        NOTE: This function is implemented because in time of implementation rc522_start function is intented for
 *        initialization and scanning in once. In future, when rc522_start gonna be refactored to just start to scan tags
 *        without initialization, this function will be just alias of rc522_start.
 * @return ESP_OK on success
 */
esp_err_t rc522_start2();

/**
 * @brief Start to scan tags. If already started, ESP_OK will just be returned.
 * @return ESP_OK on success
 */
#define rc522_resume() rc522_start2()

/**
 * @brief Pause scan tags. If already paused, ESP_OK will just be returned.
 * @return ESP_OK on success
 */
esp_err_t rc522_pause();

/**
 * @brief Destroy RC522 and free all resources
 */
void rc522_destroy();

#ifdef __cplusplus
}
#endif

#else

//------------------MFRC522 register ---------------
#define COMMAND_WAIT 0x02
#define COMMAND_READBLOCK 0x03
#define COMMAND_WRITEBLOCK 0x04
#define MFRC522_HEADER 0xAB

#define STATUS_ERROR 0
#define STATUS_OK 1

#define MIFARE_KEYA 0x00
#define MIFARE_KEYB 0x01

void initUART1(void);
void rfid_seial_task(void *pvParameters);
esp_err_t rc522_uart_set(uint8_t addr, uint8_t tx_data);
esp_err_t rc522_uart_req(uint8_t addr, uint8_t dat, TickType_t ticks_to_wait);
uint8_t rc522_uart_getchar(uint8_t addr);

#endif

