
#include <avr/io.h>
#include <stdlib.h>

uint16_t _k_p;
uint16_t _k_i;
uint16_t _max_sum_error;
uint16_t _min_error;
uint16_t _bump_detect;

// Настройка привода РХХ
void init_motor (uint16_t k_p, uint16_t k_i, uint16_t max_sum_error, uint16_t min_error, uint16_t bump_detect) {

    DDRB |= (1<<PB0);       // PB0 включает управление мостом
    DDRB |= (1<<PB3);       // PB3 - выход ШИМ
    DDRC |= (1<<PC2);       // PC2 - управление полярностью (направлением)
    
    PORTB |= (1<<PB0);      // включение управления
    
    // Настройка таймера 2
    TCCR2 |= (1<<WGM20);    // ШИМ с правильной фазой
    TCCR2 |= (1<<COM21);    // выход OC2 на PB3
    OCR2 = 0;               // скважность 0
    TCCR2 |= (1<<CS21);     // запуск, частота 8МГц / 8 / 256*2 = ~1.95кГц
    
    _k_p = k_p;
    _k_i = k_i;
    _max_sum_error = max_sum_error;
    _min_error = min_error;
    _bump_detect = bump_detect;
    
}


// Управление электродивгателем
// power: -255..255, знак определяет направление вращения
void motor (int16_t power) {
    // направление: состояние PC2 из знака power
    if (power < 0) PORTC |= (1<<PC2); else PORTC &= ~(1<<PC2);
    // скважность ШИМ
    OCR2 = abs(power) > UINT8_MAX ? UINT8_MAX : abs(power);
}


// Расчёт воздействия регулятора
// Возвращает значения -255..255    
int16_t regulator(uint16_t value, uint16_t control) {

    int16_t error;                  // величина ошибки
    static int16_t sum_error = 0;   // накопленная ошибка
    int16_t p_reg;                  // пропорциональный регулятор
    int16_t i_reg;                  // интегральный регулятор
    int16_t result;                 // временное размещение результатов
    // накопление ненулевого воздествия регулятора (для определения нахождения привода на упоре или заклинивания)
    static int16_t bump_counter = 0;

    // Приведение входных значений к рабочему диапазону
    if (value > 1024) value = 1024;
    if (control > 1024) control = 1024;

    error = control - value;        // вычисление ошибки
    
    if (abs(error) < _min_error) error = 0; // минимальный порог ошибки (если меньше - ошибка игнорируется)
    if (error == 0) {                   // При отсутствии ошибки...
        sum_error = 0;              // накопленная ошибка обнуляется во избеание раскачки.
        bump_counter = 0;           // ...как и счётчик нахождения на упоре
    } else
        sum_error += error;         // Накопление ошибки, когда она ненулевая.

    // Накопленная ошибка ограничивается
    if (sum_error > _max_sum_error) sum_error = _max_sum_error;
    if (sum_error < -(int32_t)_max_sum_error) sum_error = -(int32_t)_max_sum_error;
       
    p_reg = error * _k_p;               // Пропорциональный регулятор
    i_reg = (sum_error * _k_i) / 10;    // Интегральный регулятор

    result = p_reg + i_reg;             // Сумма составляющих

    bump_counter += result / 16;         // Накопление воздействия
    if (abs(bump_counter) >= _bump_detect) result = 0;   // При превышении порога регулятор отключается (перегрузка)

    // Ограничение минимального и максимального значений до -255..255
    return (result > UINT8_MAX) ? UINT8_MAX : (result < -UINT8_MAX) ? -UINT8_MAX : result;

}
