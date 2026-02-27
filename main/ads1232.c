/**
 * V1.0.0 - ESP-IDF port
 * ADS1232 library for ESP-IDF
 * ADS1232.c - Library for reading from an ADS1232 24-bit ADC.
 * Original Arduino library by Max Sanchez @ June 2022. https://github.com/hardmax/ADS1232
 * Ported to ESP-IDF
 *
 * MIT License
 * (c) 2022 Max Sanchez
 *
**/

#include <stdio.h>
#include <string.h>
#include <driver/gpio.h>
#include <stdbool.h>
#include <esp_err.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp32/rom/ets_sys.h"
#include "driver/gpio.h"
#include "./eve/EVE_target.h"
#include "ads1232.h"
#include "scspiffs.h"
#include "http.h"
#include "tft.h"

static const char *TAG = "ads1232";

ads1232_t ads1232;
float gVal = 0.0f;


esp_err_t ads1232_init(ads1232_t *dev, gpio_num_t pin_dout, gpio_num_t pin_sclk, 
                       gpio_num_t pin_pdwn, gpio_num_t pin_speed, ads1232_speed_t speed)
{
	if (dev == NULL) {
		ESP_LOGE(TAG, "ads1232_init: dev is NULL");
		return ESP_ERR_INVALID_ARG;
	}

	dev->pin_dout = pin_dout;
	dev->pin_sclk = pin_sclk;
	dev->pin_pdwn = pin_pdwn;
	dev->pin_speed = pin_speed;
	dev->offset = 0.0f;
	dev->scale = 1.0f;
	dev->speed = speed;

	// Configure pins
	ESP_ERROR_CHECK(gpio_set_pull_mode(pin_dout, GPIO_PULLUP_ONLY));
	ESP_ERROR_CHECK(gpio_set_direction(pin_dout, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_set_direction(pin_sclk, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(pin_pdwn, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(pin_speed, GPIO_MODE_OUTPUT));

	ads1232_set_speed(dev, speed);
	ads1232_power_up(dev);

	ESP_LOGI(TAG, "ADS1232 initialized on DOUT=%d, SCLK=%d, PDWN=%d, SPEED=%s", 
			 pin_dout, pin_sclk, pin_pdwn, speed == ADS1232_SPEED_SLOW ? "SLOW" : "FAST");

	return ESP_OK;
}

bool ads1232_is_ready(ads1232_t *dev)
{
	if (dev == NULL) {
		return false;
	}
	return gpio_get_level(dev->pin_dout) == 0;
}

void ads1232_power_up(ads1232_t *dev)
{
	if (dev == NULL) {
		ESP_LOGI(TAG, "ads1232_power_up: dev is NULL");
		return;
	}
	gpio_set_level(dev->pin_pdwn, 1);
	ets_delay_us(1);
	// Set CLK low to get the ADS1232 out of suspend
	gpio_set_level(dev->pin_sclk, 0);
	ets_delay_us(1);
}

void ads1232_power_down(ads1232_t *dev)
{
	if (dev == NULL) {
		ESP_LOGI(TAG, "ads1232_power_down: dev is NULL");
		return;
	}
	gpio_set_level(dev->pin_pdwn, 0);
	ets_delay_us(1);
	gpio_set_level(dev->pin_sclk, 1);
	ets_delay_us(1);
}

void ads1232_set_speed(ads1232_t *dev, ads1232_speed_t speed)
{
	if (dev == NULL) {
		return;
	}
	dev->speed = speed;
	switch(speed)
	{
		case ADS1232_SPEED_SLOW:
		{
			gpio_set_level(dev->pin_speed, 0);
			break;
		}
		case ADS1232_SPEED_FAST:
		{
			gpio_set_level(dev->pin_speed, 1);
			break;
		}
	}
	ets_delay_us(1);
}

/*
 * Get the raw ADC value. Can block up to 100ms in normal operation.
 * Returns ERROR_NO_ERROR on success, an error code otherwise.
 */
ads1232_error_t ads1232_read(ads1232_t *dev, int32_t *value, bool calibrating)
{
	if (dev == NULL || value == NULL) {
		ESP_LOGE(TAG, "ads1232_read: dev is NULL or value is NULL");
		return ERROR_DIVIDED_BY_ZERO;
	}

	static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	int i = 0;
	int64_t start;
	uint32_t waitingTime;
	*value = 0;
	
	/* A high to low transition on the data pin means that the ADS1232
	 * has finished a measurement (see datasheet page 13).
	 * This can take up to 100ms (the ADS1232 runs at 10 samples per
	 * second!).
	 * Note that just testing for the state of the pin is unsafe.
	 */
	 
	if(calibrating){
		if(dev->speed == ADS1232_SPEED_FAST) waitingTime = 150;
		else waitingTime = 850;
	}
	else{
		if(dev->speed == ADS1232_SPEED_FAST) waitingTime = 20;
		else waitingTime = 150;
	}

	waitingTime += 600; //[ms] Add some extra time ( sometimes takes longer than what datasheet claims! )
	
	start = esp_timer_get_time() / 1000; // Convert to milliseconds

	while(gpio_get_level(dev->pin_dout) != 1)
	{
		if((esp_timer_get_time() / 1000) - start > waitingTime) 
		{
			ESP_LOGE(TAG, "ads1232_read: Timeout waiting for HIGH");
			return ERROR_TIMEOUT_HIGH; // Timeout waiting for HIGH
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}

	start = esp_timer_get_time() / 1000;
	while(gpio_get_level(dev->pin_dout) != 0)
	{
		if((esp_timer_get_time() / 1000) - start > waitingTime) 
		{
			ESP_LOGE(TAG, "ads1232_read: Timeout waiting for LOW");
			return ERROR_TIMEOUT_LOW; // Timeout waiting for LOW
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}

	// Read 24 bits
	for(i = 23; i >= 0; i--) {
		portENTER_CRITICAL(&mux);
		gpio_set_level(dev->pin_sclk, 1);
		ets_delay_us(1);
		*value = (*value << 1) + gpio_get_level(dev->pin_dout);
		gpio_set_level(dev->pin_sclk, 0);
		ets_delay_us(1);
		portEXIT_CRITICAL(&mux);
	}

	
	if(calibrating){
	// 2 extra bits for calibrating
		for(i = 1; i >= 0; i--) {
			portENTER_CRITICAL(&mux);
			gpio_set_level(dev->pin_sclk, 1);
			ets_delay_us(1);
			gpio_set_level(dev->pin_sclk, 0); 
			ets_delay_us(1);
			portEXIT_CRITICAL(&mux);
		}
		//Esperamos que termine calibracion para recien pedir el siguiente valor
		if(dev->speed == ADS1232_SPEED_FAST){
			vTaskDelay(pdMS_TO_TICKS(100));
		} else {
			vTaskDelay(pdMS_TO_TICKS(800));
		}
	}
	
	/* Bit 23 is actually the sign bit. Shift by 8 to get it to the
	 * right position (31), divide by 256 to restore the correct value.
	 */
	*value = (*value << 8) / 256;

	if(!calibrating){
		/* The data pin now is high or low depending on the last bit that
		 * was read.
		 * To get it to the default state (high) we toggle the clock one
		 * more time (see datasheet).
		 */
		portENTER_CRITICAL(&mux);
		gpio_set_level(dev->pin_sclk, 1);
		ets_delay_us(1);
		gpio_set_level(dev->pin_sclk, 0);
		ets_delay_us(1);
		portEXIT_CRITICAL(&mux);
	}

	return ERROR_NO_ERROR; // Success
}

ads1232_error_t ads1232_read_average(ads1232_t *dev, float *value, uint8_t times, bool calibrating)
{
	if (dev == NULL || value == NULL) {
		return ERROR_DIVIDED_BY_ZERO;
		ESP_LOGE(TAG, "ERROR_DIVIDED_BY_ZERO");
	}

	int32_t sum = 0;
	ads1232_error_t err;
	int32_t val = 0;
	//Mandamos calibrar pero no tomamos el dato leido porque este no esta calibrado, recien el dato siguiente es el correcto
	if (calibrating) {
		err = ads1232_read(dev, &val, true);
		if(err != ERROR_NO_ERROR) {
			ESP_LOGE(TAG, "ERROR_READ (calibration init): %d", err);
			return err;
		}
	}
	for (uint8_t i = 0; i < times; i++) {
		err = ads1232_read(dev, &val, false);
		if(err != ERROR_NO_ERROR)
		{
			ESP_LOGE(TAG, "ERROR_READ: %d", err);
			return err;
		}
		sum += val;
		vTaskDelay(pdMS_TO_TICKS(1));
	}
	if(times == 0)
	{
		ESP_LOGE(TAG, "ERROR_DIVIDED_BY_ZERO");
		return ERROR_DIVIDED_BY_ZERO;
	}
	*value = (float)sum / times;

	return ERROR_NO_ERROR;
}

ads1232_error_t ads1232_get_value(ads1232_t *dev, float *value, uint8_t times, bool calibrating)
{
	if (dev == NULL || value == NULL) {
		ESP_LOGE(TAG, "ads1232_get_value: dev is NULL or value is NULL");
		return ERROR_DIVIDED_BY_ZERO;
	}

	float val = 0;
	ads1232_error_t err;
	err = ads1232_read_average(dev, &val, times, calibrating);
	if(err != ERROR_NO_ERROR) {
		ESP_LOGE(TAG, "ads1232_get_value: ERROR_READ: %d", err);
		return err;
	}
	*value = val - dev->offset;
	return ERROR_NO_ERROR;
}

ads1232_error_t ads1232_get_units(ads1232_t *dev, float *value, uint8_t times, bool calibrating)
{
	if (dev == NULL || value == NULL) {
		ESP_LOGE(TAG, "ads1232_get_units: dev is NULL or value is NULL");
		return ERROR_DIVIDED_BY_ZERO;
	}

	float val = 0;
	ads1232_error_t err;
	err = ads1232_get_value(dev, &val, times, calibrating);
	if(err != ERROR_NO_ERROR) {
		ESP_LOGE(TAG, "ads1232_get_units: ERROR_READ: %d", err);
		return err;
	}
	if(dev->scale == 0) {
		ESP_LOGE(TAG, "ads1232_get_units: ERROR_DIVIDED_BY_ZERO");
		return ERROR_DIVIDED_BY_ZERO;
	}
	*value = val / dev->scale;
	return ERROR_NO_ERROR;
}

ads1232_error_t ads1232_tare(ads1232_t *dev, uint8_t times, bool calibrating)
{
	if (dev == NULL) {
		ESP_LOGE(TAG, "ads1232_tare: dev is NULL");
		return ERROR_DIVIDED_BY_ZERO;
	}

	ads1232_error_t err;
	float sum = 0;
	err = ads1232_read_average(dev, &sum, times, calibrating);
	if(err != ERROR_NO_ERROR) 
	{
		ESP_LOGE(TAG, "ERROR_TARE: %d", err);
		return err;
	}
	ads1232_set_offset(dev, sum);
	return ERROR_NO_ERROR;
}

void ads1232_set_scale(ads1232_t *dev, float scale)
{
	if (dev == NULL) {
		ESP_LOGE(TAG, "ERROR_SET_SCALE: dev is NULL");
		return;
	}
	dev->scale = scale;
}

float ads1232_get_scale(ads1232_t *dev)
{
	if (dev == NULL) {
		ESP_LOGE(TAG, "ERROR_GET_SCALE: dev is NULL");
		return 1.0f;
	}
	return dev->scale;
}

void ads1232_set_offset(ads1232_t *dev, float offset)
{
	if (dev == NULL) {
		ESP_LOGE(TAG, "ERROR_SET_OFFSET: dev is NULL");
		return;
	}
	dev->offset = offset;
}

float ads1232_get_offset(ads1232_t *dev)
{
	if (dev == NULL) {
		ESP_LOGE(TAG, "ERROR_GET_OFFSET: dev is NULL");
		return 0.0f;
	}
	return dev->offset;
}


void ads1232_task(void)
{
    float factor = scaleInfo.scale_factor;
    static float prev_g = 0.0f;
    uint32_t stable_count = 0;
    int32_t raw_data = 0;
    int32_t dat = 0;
    int32_t stableData[32] = {0,};
    int32_t avr_data = 0, cal_count = 0;
    ads1232_error_t err;
    static uint8_t cal_stage = 0; // 0: idle, 1: wait weight on, 2: wait weight off
    static int32_t cal_raw_on = 0;
    static int32_t cal_raw_off = 0;
	
	DELAY_MS(3000);
		
	ads1232_init(&ads1232, GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_32, GPIO_NUM_15, ADS1232_SPEED_SLOW);
    ads1232_set_scale(&ads1232, factor);
    ads1232_tare(&ads1232, 1, false);

    while(1)
    {
        if(taskInfo.calFlag == CALI100_START)
        {
            float result = 0.0f;
            // 표준 분봉 올리고 측정 시작
            if (scaleInfo.standardWeight == 0) {
                ESP_LOGE(TAG, "CALI100_START: standardWeight is zero");
                taskInfo.calFlag = CALI0_READY;
                cal_stage = 0;
            }
            if (cal_stage == 0) {
                taskInfo.cali_tmp_factor = 0;
                ESP_LOGI(TAG, "CALI100_START");
            }

            err = ads1232_read_average(&ads1232, &result, scaleInfo.scale_sampling, false);
            if (err == ERROR_NO_ERROR) {
                dat = (int32_t)(result - ads1232_get_offset(&ads1232));
                raw_data = (int32_t)result;
                gVal = (dat) / ads1232_get_scale(&ads1232);
            } else {
                DELAY_MS(100);
                continue;
            }

            stableData[stable_count++] = raw_data;
            if(stable_count >= taskInfo.numStableBuffer)
            {
                stable_count = 0;
            }
			/* 16bit 까지는 안정적이어야 함 따라서 0x1ff 이상은 안정적이어야 함 */

            taskInfo.stable_flag = isStable(stableData, taskInfo.numStableBuffer, 0x1ff, &avr_data);

            if (cal_stage == 0) {
                if (taskInfo.stable_flag && (abs(dat) > taskInfo.zero_band)) {
                    cal_raw_on = raw_data;
                    taskInfo.cali_tmp_factor = gVal;
                    cal_stage = 1;
                    ESP_LOGI(TAG, "CALI100_START: weight captured");
                }
            }
            DELAY_MS(100);
        }
		else if(taskInfo.calFlag == STOP_CALI)
		{
            float result = 0.0f;
			// 표준 분봉 내리고 측정 종료
			ESP_LOGI(TAG, "STOP_CALI");

            if (cal_stage == 0) {
                ESP_LOGW(TAG, "STOP_CALI: start not captured");
                taskInfo.calFlag = CALI0_READY;
                taskInfo.cali_tmp_factor = 0;
                DELAY_MS(100);
                continue;
            }

            err = ads1232_read_average(&ads1232, &result, scaleInfo.scale_sampling, false);
            if (err == ERROR_NO_ERROR) {
                dat = (int32_t)(result - ads1232_get_offset(&ads1232));
                raw_data = (int32_t)result;
                gVal = (dat) / ads1232_get_scale(&ads1232);
            } else {
                DELAY_MS(100);
                continue;
            }

            stableData[stable_count++] = raw_data;
            if(stable_count >= taskInfo.numStableBuffer)
            {
                stable_count = 0;
            }
            taskInfo.stable_flag = isStable(stableData, taskInfo.numStableBuffer, 0x1ff, &avr_data);

            if (taskInfo.stable_flag && (abs(dat) <= 0x1ff)) {
                float scaleFactor = 0.0f;
                cal_raw_off = raw_data;
                scaleFactor = (float)(cal_raw_on - cal_raw_off) / scaleInfo.standardWeight;
                if (scaleFactor <= 0) {
                    ESP_LOGE(TAG, "STOP_CALI: invalid scale factor %f", scaleFactor);
                } else {
                    ads1232_set_offset(&ads1232, (float)cal_raw_off);
                    ads1232_set_scale(&ads1232, scaleFactor);
                    scaleInfo.scale_factor = scaleFactor;
                    saveScaleFactor(scaleFactor);
                    taskInfo.zero_band = (int32_t)((taskInfo.zero_cali_band / 10) * scaleInfo.scale_factor);
                    taskInfo.stable_band = (int32_t)((taskInfo.stable_band01 / 10) * scaleInfo.scale_factor);
                    taskInfo.cali_tmp_factor = scaleFactor;
                    ESP_LOGI(TAG, "STOP_CALI: done, scale factor=%f", scaleFactor);
                }
                taskInfo.calFlag = CALI0_READY;
                cal_stage = 0;
            }
            DELAY_MS(100);
		}
        else if(taskInfo.calFlag == CALI0_START)
        {
            ads1232_tare(&ads1232, 1, false);
            ads1232_set_scale(&ads1232, scaleInfo.scale_factor);
            taskInfo.calFlag = CALI0_READY;
            cal_stage = 0;
			ESP_LOGI(TAG, "CALI0_START");
        }
        else if(taskInfo.calFlag == CALI0_READY)
        {
            if (abs(dat) < taskInfo.zero_band)
            {
                if(taskInfo.stable_flag)
                {
                    cal_count++;
                    if(cal_count > taskInfo.numAutoZero)
                    {
                        cal_count = 0;
                        taskInfo.zero_flag = 1;
                        ads1232_set_offset(&ads1232, (float)avr_data);
                    }
                }
                else
                {
                    taskInfo.zero_flag = 0;
                }
            }
            else
            {
                taskInfo.zero_flag = 0;
            }
            stableData[stable_count++] = raw_data;
            if(stable_count >= taskInfo.numStableBuffer)
            {
                stable_count = 0;
            }

            // 안정성 확인 (isStable 함수는 기존 코드에서 따로 구현되어 있어야 함)
            taskInfo.stable_flag = isStable(stableData, taskInfo.numStableBuffer, taskInfo.stable_band, &avr_data);
            // 평균값 읽기. ads1232_read_average와 offset/scale 보정 사용
            float result = 0.0f;
            err = ads1232_read_average(&ads1232, &result, scaleInfo.scale_sampling, false);
            // 읽기 오류시 진행 skip
            if (err == ERROR_NO_ERROR) {
                dat = (int32_t)(result - ads1232_get_offset(&ads1232));
                raw_data = (int32_t)result;
                gVal = (dat) / ads1232_get_scale(&ads1232);
            }

            if ((((gVal > (prev_g - 0.5f)) && (gVal < (prev_g + 0.5f))) && gVal > 0.99f) || scaleInfo.maxweight < gVal )
            {
                gVal = prev_g;
            }
            prev_g = gVal;
            DELAY_MS(100);
        }
    }
}


float ads1232_getUnitRawData(uint8_t times, int32_t *dat, int32_t *raw_data)
{
    float val = 0.0f;
    ads1232_error_t err;
    err = ads1232_read_average(&ads1232, &val, times, false);
    if(err != ERROR_NO_ERROR) return 0.0f;
    *dat = (int32_t)val;
    *raw_data = (int32_t)val;
    return val;
}

float ads1232_getUintValue(void)
{
    return gVal;
}

void ads1232_calibrate(float standardWeight)
{
	float val = 0.0f;
	ads1232_error_t err;
	float scaleFactor = 0.0f;

	// Validate standard weight
	if (standardWeight == 0) {
		ESP_LOGE(TAG, "Standard weight cannot be zero for calibration.");
		return;
	}

	// Wait for ADS1232 to be ready before tare
	// Give some time for the chip to stabilize
	vTaskDelay(pdMS_TO_TICKS(100));
	
	// Wait for ADS1232 to be ready (DOUT pin goes LOW)
	int timeout_count = 0;
	while(!ads1232_is_ready(&ads1232) && timeout_count < 2000) {
		vTaskDelay(pdMS_TO_TICKS(1));
		timeout_count++;
	}
	ESP_LOGI(TAG, "timeout_count: %d", timeout_count);
	if(timeout_count >= 2000) {
		ESP_LOGW(TAG, "ADS1232 not ready, attempting reset...");
		// Try to reset ADS1232 by power cycling
		ads1232_power_down(&ads1232);
		vTaskDelay(pdMS_TO_TICKS(50));
		ads1232_power_up(&ads1232);
		vTaskDelay(pdMS_TO_TICKS(200));
		
		// Wait again after reset
		timeout_count = 0;
		while(!ads1232_is_ready(&ads1232) && timeout_count < 2000) {
			vTaskDelay(pdMS_TO_TICKS(1));
			timeout_count++;
		}
		if(timeout_count >= 2000) {
			ESP_LOGE(TAG, "ADS1232 still not ready after reset");
			return;
		}
		ESP_LOGI(TAG, "ADS1232 ready after reset");
	}

	// Tare first
	err = ads1232_tare(&ads1232, 5, false);
	if(err != ERROR_NO_ERROR) {
		ESP_LOGE(TAG, "Tare error during calibration: %d", err);
		return;
	}
	
	// Wait a bit after tare before calibration read
	vTaskDelay(pdMS_TO_TICKS(100));

	// Read average value for calibration
	err = ads1232_read_average(&ads1232, &val, 10, true);
	if(err != ERROR_NO_ERROR) {
		ESP_LOGE(TAG, "Calibration read error: %d", err);
		return;
	}

	// Calculate new scale factor
	scaleFactor = (val - ads1232_get_offset(&ads1232)) / standardWeight;
	
	// Validate scale factor (check if it's within reasonable range)
	if (scaleFactor <= 0 || scaleFactor > scaleInfo.maxFactor) {
		ESP_LOGE(TAG, "Invalid scale factor calculated: %f (max allowed: %f)", scaleFactor, scaleInfo.maxFactor);
		return;
	}

	// Set scale factor
	ads1232_set_scale(&ads1232, scaleFactor);
	scaleInfo.scale_factor = scaleFactor; // Update global scale factor
	
	// Save scale factor to file
	saveScaleFactor(scaleFactor);
	
	ESP_LOGI(TAG, "Calibration complete. New scale factor: %f", scaleFactor);
}

uint8_t isStable(int32_t *dataBuffer, uint8_t bufferSize, int32_t stableBand, int32_t *averageData)
{
	int64_t sum = 0;
	int32_t maxVal = dataBuffer[0];
	int32_t minVal = dataBuffer[0];

	for (uint8_t i = 0; i < bufferSize; i++)
	{
		sum += dataBuffer[i];
		if (dataBuffer[i] > maxVal)
			maxVal = dataBuffer[i];
		if (dataBuffer[i] < minVal)
			minVal = dataBuffer[i];
	}

	*averageData = (int32_t)(sum / bufferSize);

	if ((maxVal - minVal) <= stableBand)
	{
		return 1; // Stable
	}
	else
	{
		return 0; // Not stable
	}
}

// gpio test code
void gpio_test(ads1232_t *dev)
{
	if (dev == NULL) {
		ESP_LOGE(TAG, "gpio_test: dev is NULL");
		return;
	}
	//ads1232_t mapping state
	ESP_LOGI(TAG, "gpio_test: dev->pin_dout = %d, dev->pin_sclk = %d, dev->pin_pdwn = %d, dev->pin_speed = %d", dev->pin_dout, dev->pin_sclk, dev->pin_pdwn, dev->pin_speed);

	int i = 0;
	while(i < 10000)
	{	
		gpio_set_level(dev->pin_dout, 1); // dout
		gpio_set_level(dev->pin_sclk, 1); // sclk
		gpio_set_level(dev->pin_speed, 1); // speed
		ads1232_power_up(dev);
		DELAY_MS(1);
		gpio_set_level(dev->pin_dout, 0); // dout
		gpio_set_level(dev->pin_sclk, 0); // sclk
		gpio_set_level(dev->pin_speed, 0); // speed
		ads1232_power_down(dev);
		DELAY_MS(1);
		i++;
	}
	ESP_LOGI(TAG, "gpio_test completed");
}