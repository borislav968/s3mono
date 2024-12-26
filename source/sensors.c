
#include <avr/io.h>
#include <avr/interrupt.h>


// Входы АЦП
volatile uint16_t ain0 = 0;
volatile uint16_t ain1 = 0;

// Настройка работы с датчиками
void init_sensors () {
    // АЦП
    ADMUX = (1<<REFS0);     // опорное напряжение от вывода AVCC (5В)
    ADCSRA |= (1<<ADPS2) | (1<<ADPS1);  // делитель 64 (125кГц)
    ADCSRA |= (1<<ADIE);    // разрешение прерывания от АЦП
    ADCSRA |= (1<<ADFR);    // режим постоянной работы
    ADCSRA |= (1<<ADEN);    // включение АЦП
    ADCSRA |= (1<<ADSC);    // запуск первого преобразования
}


#define min0 0
#define max0 1013
#define max_reg 840
#define max_tps0 127

#define min1 148
#define max1 864
#define max_tps1 128


// Прерывание от АЦП
ISR (ADC_vect) {
    static uint8_t counter = 0;     // счётчик измерений
    // Результат первого измерения после переключения каналов недостоверен
    if (counter++ & 1) return;  // ...поэтому игнорируется
    // Результат каждого второго измерения можно использовать
    if (counter & 2) {          // бит 1 определяет, на каком из каналов АЦП измеряем
        ain0 = ADC;             // получение результата в ain0
        if (ain0 < min0) ain0 = min0;
        if (ain0 > max0) ain0 = max0;
        ADMUX |= (1<<MUX0);     // переключение на 1-й канал
    } else {
        ain1 = ADC;             // получение результата в ain1
        if (ain1 < min1) ain1 = min1;
        if (ain1 > max1) ain1 = max1;
        ADMUX &= ~(1<<MUX0);    // переключение на 0-й канал
    }    
}

// Вычисление положения заслонки
uint8_t tps (uint16_t reg_tps) {
    uint8_t tps0, tps1;
    uint8_t position;
    uint8_t reg_part;
    // интерполяция первого датчика
    tps0 = ((uint32_t)max_tps0 * (ain0 - min0)) / (max0 - min0);
    // интерполяция второго датчика
    tps1 = ((uint32_t)max_tps1 * (ain1 - min1)) / (max1 - min1);
    // проверка допустимости значения reg_tps
    if (reg_tps > ain0) reg_tps = ain0;
    // положение регулятора в 1/255
    reg_part = ((uint32_t)max_tps0 * (reg_tps - min0)) / (max0 - min0);
    // положение заслонки с учётом регулятора в 1/255
    position = tps0 + tps1 - reg_part;
    // приведение к диапазону 1..255
    position = ((uint32_t)position * 255) / (255 - reg_part);

    return position;
}

// Вычисление целевого положения по 1-му ДПДЗ
// value: текущее число шагов, range - полное число шагов
uint16_t target_adc(uint16_t value, uint16_t range) {
    if (value > range) return range;
    return ((uint32_t)value * (max_reg - min0)) / range;
}

uint16_t sensor0 () {
    return ain0;
}

uint16_t sensor1 () {
    return ain1;
}

// Состояние концевика заслонки
uint8_t pedal_switch () {
    return (PIND & (1<<PD5)) ? 1 : 0;
}




