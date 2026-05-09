/*
Defaults:
ADC for duty GPIO 0
ADC for freq GPIO 1 (стандартно 1-1000 гц)(скоріш за все реальна мінімальна поточна частота 5 гц)
PWM GPIO 8
*/

#include "settings.h"
#include "my_utils.h"

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline uint32_t constrain(uint32_t val, uint32_t from, uint32_t to) {
    if(val < from) val = from;
    if(val > to) val = to;
    return val;
}

// Повертає нормалізоване duty
float handleDuty(float normalized, float dt);
float handleFreq(float normalized, float dt);

void app_main(void) {
    initPWM();
    initADC();

    while (1) {
        const float period = periodMs / 1000.f;
            
        // Це все потрібно виносити в функції і т.д. але на це немає ні часу ні потреби ускладнювати та систематизувати в рамках прототипу.
        // Handle duty
        {
            uint32_t mV = get_voltage_mV(0);

            mV = constrain(mV, VOLTAGE_MIN_MV, VOLTAGE_MAX_MV);
            float normalizedVoltage = mapFloat(mV, VOLTAGE_MIN_MV, VOLTAGE_MAX_MV, 0.0f, 1.0f);

            float normalizedDuty = handleDuty(normalizedVoltage, period);
            uint32_t duty = mapFloat(normalizedDuty, 0.0f, 1.0f, 0.0f, MAX_DUTY);
            constrain(duty, 0, MAX_DUTY);

            setPWM(duty);

            printf("duty: %d ", duty);
        }

        // Handle freq
        {
            uint32_t mV = get_voltage_mV(1);

            mV = constrain(mV, VOLTAGE_MIN_MV, VOLTAGE_MAX_MV);
            float normalizedVoltage = mapFloat(mV, VOLTAGE_MIN_MV, VOLTAGE_MAX_MV, 0.0f, 1.0f);

            float normalizedFreq = handleFreq(normalizedVoltage, period);
            uint32_t freq = mapFloat(normalizedFreq, 0.0f, 1.0f, MIN_FREQ, MAX_FREQ);
            constrain(freq, MIN_FREQ, MAX_FREQ);

            setFreq(freq);

            printf("freq: %d ", freq);
        }

        printf("                                         \r");
        
        vTaskDelay(pdMS_TO_TICKS(periodMs));
    }   
    
    deinitADC();
}

// Це твоя основна функція, при написанні тут коду роби вид що всього що вище не існує. Те що вище залишено для наглядності як це працює.
// Повенути нормалізоване duty
float handleDuty(float normalized, float dt) {
    /* Тут можеш реалізувати будь яку обробку нормалізованого сигналу. 
    Тобі байдуже який у нього був діапазон, зараз він від 0 до 1
    В тебе є час з минулої обробки сигналу в секундах щоб було згідно системи СІ
    Повертаєш ти нормалізоване значення вихідного сигналу, тому тобі байдуже який в нього діапазон, для тебе він від 0 до 1
    Тому тобі байдуже на все що відбувається зовні цієї функції, ти навіть не знаєш про ESP IDF, тут чиста логіка, чистий С
    Так тобі буде простіше почати щось писати, не думаючи взагалі не про які інші деталі реалізації програми*/

    return normalized;
}

float handleFreq(float normalized, float dt) {
    // Тут можна наприклад зробити логірифмічну зміну значень частоти для плавного керування на низьких частотах та великого діапазону до 20-25 кГц
    return normalized;
}

