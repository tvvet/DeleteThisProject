#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

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

#ifdef __cplusplus
    }
#endif

static adc_oneshot_unit_handle_t adc1Descr;
static adc_cali_handle_t adc1CalibrationDescrDuty = NULL;
static adc_cali_handle_t adc1CalibrationDescrFreq = NULL;
static bool isCalibratedForDuty = false;
static bool isCalibratedForFreq = false;

void initPWM() {
    /* Тут по мойму не догляду була помилка, я просто перейшов з Designated Initializers на класичну інціалізацію, 
    але відмінність в тому що класична ініціалізація залишає сміття якщо не вказані стандартні значення в структурі, 
    а так як в С їх не має то там просто залишилалось сміття.
    Тому зараз структура ініціалізується нулями через пустий список ініціалізації (uniform initialization) а потім доповнюється значеннями
    
    Для розуміння прикладів інціалізації, все це валідно:
    int a = 5 // Copy init
    int a (5) // Я не памятаю як це називається
    int a {5} // Uniform init / список ініціалізації, точно не скажу бо суті не змінює.

    int a[5]; // Залишає сміття в масиві (ніяк не ініціалізує його)
    int a[5] {0}; або int a[5] {10}; // Ініцалізує масив вказаним значенням 
    int a[5] {}; // те саме що й int a[5] {0};
    int a[5] {1, 2, 3, 4, 5} // Список ініціалізації

    Все це більше розкривається з обєктами і там кожна з них має окремі нюанси хоча все й схоже на те як це працює зі змінними та масивами.
    За все по цій темі розповіати зараз я не буду бо і сам не памятаю всих нюансів, якщо потрібно то спросиш AI. 
    */

    /*
    Якщо не дивись урок про структури то поясню письмово щоб було наглядніше
    Наприклад у нас є екран ПК. У нього є пікселі. У пікселя є 2 координати та 3 світлодіода.
    Як ми це представимо в програмі? Ну наприклад так:
    int width = 1920;
    int height = 1080;
    int pixelNum = width * height;

    unsigned char R[pixelNum];
    unsigned char G[pixelNum];
    unsigned char B[pixelNum];
    unsigned int X[pixelNum];
    unsigned int Y[pixelNum];

    Або так:
    unsigned char R[pixelNum];
    unsigned char G[pixelNum];
    unsigned char B[pixelNum];
    unsigned int Coordinate[height][width];

    Наче навіть краще. Спробуємо зробити ще краще та зрозуміліше. 
    Створимо структуру для Кольору.
    struct RGB {
        unsigned char R;
        unsigned char G;
        unsigned char B;
    };

    Створимо структуру для Координат.
    struct Coordinate {
        unsigned int X;
        unsigned int Y;
    };

    Створимо структуру для Пікселя.
    struct Pixel {
        Coordinate coord;
        RGB color;
    };

    Створюємо наш масив обєктів пікселів
    Pixel[pixelNum];

    Це все, 1 строка в коді замість 4-5 логічно не звязаних змінних в рамках мови.

    По суті ми просто логічно в рамках мови обєднали всі ці змінні 
    і показали що вони повинні оброблятись в певних умовах як характеристики одного обєкта.
    Це стовсується кожної структури окремо. Наприклад:
    
    // Використовуємо для приклада 101-ий піксель в якого індекс 100
    bool onBorder = isPixelOnMonitorBorder(Pixel[100].coord);
    HSV hsvColorScheme = RGBtoHSV(Pixel[100].color); // Умовно в нас є тип для кольорової схеми HSV (здається так називається, див. вікіпедію)
    // Перевіряє щоб піксель був в рамках монітору та мав діапазон кольору 0-255 
    // (хоча це гарантується 1 байтним типом данних...ну просто можемо допустити щось подібне, я хз що в кольорі можна перевірити на валідність)
    bool validPixel = isPixelValid(Pixel[100], maxMonitorWidth, maxMonitorHeight);
    
    Таким чином ми передаємо в функції згруповані дані замість 2-5 розрізнених змінних які можливо переплутати.
    А з точки зору оптимізації ми можемо передати просто вказівник на структуру а не 4-5 копій/вказівників на змінні.
    Варто зауважити що інсують випадки коли для гнучкості відмовляються від певного рівня (концептуально) структур але це вже зовсім інша історія.

    В мові С структури створюються дещо з іншим синтаксисом але суть та сама.
    */
    ledc_timer_config_t ledc_timer {};
    ledc_timer.speed_mode       = LEDC_MODE;
    ledc_timer.duty_resolution  = LEDC_DUTY_RES;
    ledc_timer.timer_num        = LEDC_TIMER;
    ledc_timer.freq_hz          = LEDC_FREQUENCY;
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(
    ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel {};
    ledc_channel.speed_mode     = LEDC_MODE;
    ledc_channel.channel        = LEDC_CHANNEL;
    ledc_channel.timer_sel      = LEDC_TIMER;
    ledc_channel.intr_type      = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num       = LEDC_OUTPUT_IO;
    ledc_channel.duty           = 0;
    ledc_channel.hpoint         = 0;

    ESP_ERROR_CHECK(
    ledc_channel_config(&ledc_channel));
}

void setPWM(uint32_t duty) {
    ESP_ERROR_CHECK(
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

/*
Для нормального налаштування потрібно змінювати duty відовідно до вимог freq щоб впевнитись що встановлена допустима кобінація.
На esp32-c3 макс розрядність шим 14 біт що дає ~5 гц мінімальну частоту при стандартному джерелу тактування.
Для зменшення мінімальної частоти потрібно також змінювати джерело тактування або ж робити програмний ШИМ через пееривання.
*/
void setFreq(uint32_t freq) {
    ESP_ERROR_CHECK(
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, freq));
}

bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle) {
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
    adc_oneshot_unit_init_cfg_t init_config1 {};
    init_config1.unit_id = ADC_unit;
    ESP_ERROR_CHECK(
    adc_oneshot_new_unit(&init_config1, &adc1Descr));

    // Channel config for both
    adc_oneshot_chan_cfg_t config {};
    config.atten = attenuation;
    config.bitwidth = ADC_BITWIDTH_DEFAULT;

    // Unique config for duty
    ESP_ERROR_CHECK(
    adc_oneshot_config_channel(adc1Descr, ADC_channel_duty, &config));
    isCalibratedForDuty = example_adc_calibration_init(ADC_unit, ADC_channel_duty, attenuation, &adc1CalibrationDescrDuty);

    // Unique config for freq
    ESP_ERROR_CHECK(
    adc_oneshot_config_channel(adc1Descr, ADC_channel_freq, &config));
    isCalibratedForFreq = example_adc_calibration_init(ADC_unit, ADC_channel_freq, attenuation, &adc1CalibrationDescrFreq);
}

int get_voltage_mV(uint8_t channel) {
    int adcRawValue = 0;
    int voltage = 0;

    switch(channel) {
        case 0:
            ESP_ERROR_CHECK(
            adc_oneshot_read(adc1Descr, ADC_channel_duty, &adcRawValue)); 

            if (isCalibratedForDuty) {
                ESP_ERROR_CHECK(
                adc_cali_raw_to_voltage(adc1CalibrationDescrDuty, adcRawValue, &voltage));
            } 

            break;
        case 1:
            ESP_ERROR_CHECK(
            adc_oneshot_read(adc1Descr, ADC_channel_freq, &adcRawValue)); 

            if (isCalibratedForFreq) {
                ESP_ERROR_CHECK(
                adc_cali_raw_to_voltage(adc1CalibrationDescrFreq, adcRawValue, &voltage));
            }
            
            break;
    }

    return voltage;
}

void deinitADC() {
    ESP_ERROR_CHECK(
    adc_cali_delete_scheme_curve_fitting(adc1CalibrationDescrDuty));
    ESP_ERROR_CHECK(
    adc_cali_delete_scheme_curve_fitting(adc1CalibrationDescrFreq));
}