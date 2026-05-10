#pragma once

// PWM settings
#define LEDC_TIMER              LEDC_TIMER_0 // Номер таймера
#define LEDC_MODE               LEDC_LOW_SPEED_MODE // Це точно не знаю що робе але для esp32-c3 безальтернативно
#define LEDC_OUTPUT_IO          (8) // номер GPIO який ми застосовуємо
#define LEDC_CHANNEL            LEDC_CHANNEL_0 // Номер канала ШИМ
#define LEDC_DUTY_RES           LEDC_TIMER_14_BIT // Розрядність ШИМ
#define MAX_DUTY                (1u << LEDC_DUTY_RES) - 1
#define LEDC_FREQUENCY          (4000) // Частота ШИМ.

// ADC settings
#define ADC_unit                ADC_UNIT_1 // Яка апаратна ADC оберається для налаштування
#define ADC_channel_duty        ADC_CHANNEL_0 // Данний канал на GPIO 0. Який канал цієї ADC оберається для налаштування
#define ADC_channel_freq        ADC_CHANNEL_1 // Данний канал на GPIO 0. Який канал цієї ADC оберається для налаштування
#define attenuation             ADC_ATTEN_DB_12 // Оберається дільник напруги для ADC задля забезпечення більш широкого діапазону напруг. 

// Log settings
const char *TAG = "EXAMPLE"; // Змінна яка використовується в логуванні

// Program settings
#define VOLTAGE_MIN_MV          500
#define VOLTAGE_MAX_MV          2000
#define periodMs                20
#define MIN_FREQ                5 // За розрахунком при 14 bit resolution це мінімальна частота і при частоті нижче буде помилка. 
#define MAX_FREQ                1000