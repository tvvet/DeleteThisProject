#pragma once

#include "driver/ledc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "settings.h"

static adc_oneshot_unit_handle_t adc1Descr;
static adc_cali_handle_t adc1CalibrationDescrDuty = NULL;
static adc_cali_handle_t adc1CalibrationDescrFreq = NULL;
static bool isCalibratedForDuty = false;
static bool isCalibratedForFreq = false;

void initPWM() {
    ledc_timer_config_t ledc_timer;
    ledc_timer.speed_mode       = LEDC_MODE;
    ledc_timer.duty_resolution  = LEDC_DUTY_RES;
    ledc_timer.timer_num        = LEDC_TIMER;
    ledc_timer.freq_hz          = LEDC_FREQUENCY;
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel;
    ledc_channel.speed_mode     = LEDC_MODE;
    ledc_channel.channel        = LEDC_CHANNEL;
    ledc_channel.timer_sel      = LEDC_TIMER;
    ledc_channel.intr_type      = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num       = LEDC_OUTPUT_IO;
    ledc_channel.duty           = 0;
    ledc_channel.hpoint         = 0;

    ledc_channel_config(&ledc_channel);
}

void setPWM(uint32_t duty) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

/*
Для нормального налаштування потрібно змінювати duty відовідно до вимог freq щоб впевнитись що встановлена допустима кобінація.
На esp32-c3 макс розрядність шим 14 біт що дає ~5 гц мінімальну частоту при стандартному джерелу тактування.
Для зменшення мінімальної частоти потрібно також змінювати джерело тактування або ж робити програмний ШИМ через пееривання.
*/
void setFreq(uint32_t freq) {
    ESP_ERROR_CHECK(ledc_set_freq(LEDC_MODE, LEDC_TIMER, freq));
}

static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle) {
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .chan = channel,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
    if (ret == ESP_OK) {
        calibrated = true;
    }
    
    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

void initADC() {
    // Config ADC unit
    adc_oneshot_unit_init_cfg_t init_config1;
    init_config1.unit_id = ADC_unit;
    adc_oneshot_new_unit(&init_config1, &adc1Descr);

    // Channel config for both
    adc_oneshot_chan_cfg_t config;
    config.atten = attenuation;
    config.bitwidth = ADC_BITWIDTH_DEFAULT;

    // Unique config for duty
    adc_oneshot_config_channel(adc1Descr, ADC_channel_duty, &config);
    isCalibratedForDuty = example_adc_calibration_init(ADC_unit, ADC_channel_duty, attenuation, &adc1CalibrationDescrDuty);

    // Unique config for freq
    adc_oneshot_config_channel(adc1Descr, ADC_channel_freq, &config);
    isCalibratedForFreq = example_adc_calibration_init(ADC_unit, ADC_channel_freq, attenuation, &adc1CalibrationDescrFreq);
}

uint32_t get_voltage_mV(uint8_t channel) {
    int adcRawValue = 0;
    int voltage = 0;

    switch(channel) {
        case 0:
            adc_oneshot_read(adc1Descr, ADC_channel_duty, &adcRawValue); 

            if (isCalibratedForDuty) {
                adc_cali_raw_to_voltage(adc1CalibrationDescrDuty, adcRawValue, &voltage);
            } 

            break;
        case 1:
            adc_oneshot_read(adc1Descr, ADC_channel_freq, &adcRawValue); 

            if (isCalibratedForFreq) {
                adc_cali_raw_to_voltage(adc1CalibrationDescrFreq, adcRawValue, &voltage);
            }
            
            break;
    }

    return voltage;
}

void deinitADC() {
    adc_cali_delete_scheme_curve_fitting(adc1CalibrationDescrDuty);
    adc_cali_delete_scheme_curve_fitting(adc1CalibrationDescrFreq);
}