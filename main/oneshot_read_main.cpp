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

inline uint32_t constrain(const uint32_t val, const uint32_t from, const uint32_t to) {
    if(val < from) return from;
    if(val > to) return to;
    return val;
}

// Повертає нормалізоване duty
float handleDuty(float normalized, float dt);
float handleFreq(float normalized, float dt);

extern "C" void app_main(void) {
    initPWM();
    initADC();

    while (1) {
        const float period = periodMs / 1000.f;
            
        // Це все потрібно виносити в функції і т.д. але на це немає ні часу ні потреби ускладнювати та систематизувати в рамках прототипу.
        // Handle duty
        {
            int mV = get_voltage_mV(0);

            mV = constrain(mV, VOLTAGE_MIN_MV, VOLTAGE_MAX_MV);
            float normalizedVoltage = mapFloat(mV, VOLTAGE_MIN_MV, VOLTAGE_MAX_MV, 0.0f, 1.0f);

            float normalizedDuty = handleDuty(normalizedVoltage, period);
            constrain(normalizedDuty, 0, 1.f);
            uint32_t duty = mapFloat(normalizedDuty, 0.0f, 1.0f, 0.0f, MAX_DUTY);

            setPWM(duty);

            printf(">duty: %lu\n", duty);
        }

        // Handle freq
        {
            int mV = get_voltage_mV(1);

            mV = constrain(mV, VOLTAGE_MIN_MV, VOLTAGE_MAX_MV);
            float normalizedVoltage = mapFloat(mV, VOLTAGE_MIN_MV, VOLTAGE_MAX_MV, 0.0f, 1.0f);

            float normalizedFreq = handleFreq(normalizedVoltage, period);
            constrain(normalizedFreq, 0, 1.f);
            uint32_t freq = mapFloat(normalizedFreq, 0.0f, 1.0f, MIN_FREQ, MAX_FREQ);

            setFreq(freq);

            printf(">freq: %lu \n", freq);
        }
        
        vTaskDelay(pdMS_TO_TICKS(periodMs));
    }   
    
    deinitADC();
}

// Це твоя основна функція, при написанні тут коду роби вид що всього що вище не існує. Те що вище залишено для наглядності як це працює.
// Повенути нормалізоване duty
float handleDuty(float normalized, float dt) {
    float deadzoneMin = middleDuty - deadDuty;
    float deadzoneMax = middleDuty + deadDuty;

    if (normalized <= deadzoneMin) {
        return mapFloat(normalized, 0, deadzoneMin, -1.0f, 0.0f);
    }
    else if (normalized >= deadzoneMax) {
        return mapFloat(normalized, deadzoneMax, 1, 0.0f, 1.0f);
    }
    else {
        return 0.0f;
    }
}

float handleFreq(float normalized, float dt) {
    // Тут можна наприклад зробити логірифмічну зміну значень частоти для плавного керування на низьких частотах та великого діапазону до 20-25 кГц
    return normalized;
}

