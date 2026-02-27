/**
 * V1.0.0 - ESP-IDF port
 * ADS1232 library for ESP-IDF
 * ADS1232.h - Library for reading from an ADS1232 24-bit ADC.
 * Original Arduino library by Max Sanchez @ June 2022. https://github.com/hardmax/ADS1232
 * Ported to ESP-IDF
 *
 * MIT License
 * (c) 2022 Max Sanchez
 *
**/

#ifndef ADS1232_H
#define ADS1232_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "driver/gpio.h"
#include "esp_err.h"


/*hx711 관련 코드 호환*/
typedef enum
{
    CALI0_READY = 0,
    CALI100_START = 1,
    CALI100_END = 2,
    CALI0_START = 3
} CALI100STAUS;


/* end */

typedef enum {
	ERROR_NO_ERROR = 0,
	ERROR_TIMEOUT_HIGH,     // Timeout waiting for HIGH
	ERROR_TIMEOUT_LOW,      // Timeout waiting for LOW
	ERROR_WOULD_BLOCK,      // weight not measured, measuring takes too long
	ERROR_STABLE_TIMEOUT,   // weight not stable within timeout
	ERROR_DIVIDED_BY_ZERO    
} ads1232_error_t;

typedef enum {
	ADS1232_SPEED_SLOW = 0,
	ADS1232_SPEED_FAST
} ads1232_speed_t;

/**
 * Device descriptor
 */
typedef struct {
	gpio_num_t pin_dout;
	gpio_num_t pin_sclk;
	gpio_num_t pin_pdwn;
	gpio_num_t pin_speed;
	
	float offset;	// used for tare weight
	float scale;	// used to return weight in grams, kg, ounces, whatever
	
	ads1232_speed_t speed;
} ads1232_t;

/**
 * Initialize the ADS1232 library
 * @param dev Device descriptor
 * @param pin_dout Data output pin
 * @param pin_sclk Serial clock pin
 * @param pin_pdwn Power down pin
 * @param pin_speed Speed pin
 * @param speed Initial speed (SLOW or FAST)
 * @return ESP_OK on success
 */
esp_err_t ads1232_init(ads1232_t *dev, gpio_num_t pin_dout, gpio_num_t pin_sclk, 
                       gpio_num_t pin_pdwn, gpio_num_t pin_speed, ads1232_speed_t speed);

/**
 * Check if chip is ready
 * From the datasheet: When output data is not ready for retrieval, digital output pin DOUT is high. 
 * Serial clock input PD_SCK should be low. When DOUT goes to low, it indicates data is ready for retrieval.
 * @param dev Device descriptor
 * @return true if ready, false otherwise
 */
bool ads1232_is_ready(ads1232_t *dev);

/**
 * Set the speed mode
 * @param dev Device descriptor
 * @param speed Speed mode (SLOW or FAST)
 */
void ads1232_set_speed(ads1232_t *dev, ads1232_speed_t speed);

/**
 * Wait for the chip to be ready and returns a reading
 * @param dev Device descriptor
 * @param value Pointer to store the reading
 * @param calibrating Whether in calibrating mode
 * @return ERROR_NO_ERROR on success, error code otherwise
 */
ads1232_error_t ads1232_read(ads1232_t *dev, int32_t *value, bool calibrating);

/**
 * Returns an average reading
 * @param dev Device descriptor
 * @param value Pointer to store the average value
 * @param times How many times to read
 * @param calibrating Whether in calibrating mode
 * @return ERROR_NO_ERROR on success, error code otherwise
 */
ads1232_error_t ads1232_read_average(ads1232_t *dev, float *value, uint8_t times, bool calibrating);

/**
 * Returns (read_average() - OFFSET), that is the current value without the tare weight
 * @param dev Device descriptor
 * @param value Pointer to store the value
 * @param times How many readings to do
 * @param calibrating Whether in calibrating mode
 * @return ERROR_NO_ERROR on success, error code otherwise
 */
ads1232_error_t ads1232_get_value(ads1232_t *dev, float *value, uint8_t times, bool calibrating);

/**
 * Returns get_value() divided by SCALE, that is the raw value divided by a value obtained via calibration
 * @param dev Device descriptor
 * @param value Pointer to store the units value
 * @param times How many readings to do
 * @param calibrating Whether in calibrating mode
 * @return ERROR_NO_ERROR on success, error code otherwise
 */
ads1232_error_t ads1232_get_units(ads1232_t *dev, float *value, uint8_t times, bool calibrating);

/**
 * Set the OFFSET value for tare weight
 * @param dev Device descriptor
 * @param times How many times to read the tare value
 * @param calibrating Whether in calibrating mode
 * @return ERROR_NO_ERROR on success, error code otherwise
 */
ads1232_error_t ads1232_tare(ads1232_t *dev, uint8_t times, bool calibrating);

/**
 * Set the SCALE value; this value is used to convert the raw data to "human readable" data (measure units)
 * @param dev Device descriptor
 * @param scale Scale value
 */
void ads1232_set_scale(ads1232_t *dev, float scale);

/**
 * Get the current SCALE
 * @param dev Device descriptor
 * @return Current scale value
 */
float ads1232_get_scale(ads1232_t *dev);

/**
 * Set OFFSET, the value that's subtracted from the actual reading (tare weight)
 * @param dev Device descriptor
 * @param offset Offset value
 */
void ads1232_set_offset(ads1232_t *dev, float offset);

/**
 * Get the current OFFSET
 * @param dev Device descriptor
 * @return Current offset value
 */
float ads1232_get_offset(ads1232_t *dev);

/**
 * Put the chip into power down mode
 * @param dev Device descriptor
 */
void ads1232_power_down(ads1232_t *dev);

/**
 * Wake up the chip after power down mode
 * @param dev Device descriptor
 */
void ads1232_power_up(ads1232_t *dev);

float ads1232_getUnitRawData(uint8_t times, int32_t *dat, int32_t *raw_data);
float ads1232_getUintValue(void);
void ads1232_task(void);

void ads1232_calibrate(float standardWeight);

uint8_t isStable(int32_t *dataBuffer, uint8_t bufferSize, int32_t stableBand, int32_t *averageData);

void gpio_test(ads1232_t *dev);
#endif /* ADS1232_H */
