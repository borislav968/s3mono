
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "config.h"
#include "uart.h"
#include "sensors.h"
#include "secu.h"
#include "motor.h"

volatile uint8_t t0_counter = 0;    // Счётчик переполнениий таймера 0
volatile uint8_t timer_main = 0;    // Счётчик главного цикла
volatile uint8_t timer_debug = 0;   // Счётчик цикла отладки (UART)

volatile uint16_t * idle_set;       // Положение РХХ


// Настройка таймера 0 (главный цикл)
void init_timer_main () {
    TIMSK |= (1<<TOIE0);            // Разрешение прерывания по переполнению
    TCCR0 |= (1<<CS01) | (1<<CS00); // Делитель частоты 64 (125кГц), переполнение ~2мс
}


// Прерывание от таймера 0 по переполнению
ISR (TIMER0_OVF_vect) {
    t0_counter++;
    if (!(t0_counter % 2)) timer_main++;        // ~4мс
    if (!(t0_counter % 128)) timer_debug++;     // ~260мс
}


int main (void) {

    // Настройка обмена с SECU-3
    init_secu_connection(&idle_set, SECU_STEPS);

    // Глобальное разрешение прерываний
    sei(); 

    // Настройка UART
    uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));

    // Настройка таймера 0 (главный цикл)
    init_timer_main();

    // Настройка работы с датчиками
    init_sensors();

    // Настройка привода РХХ
    init_motor(K_P, K_I, MAX_SUM_ERROR, MIN_ERROR, BUMP_DETECT);
    
    
    // Подготовка терминала
    uart_puts_P("\x1b[2J");               // Очистить экран терминала
    uart_puts_P("\x9b?25l");              // Спрятать курсор
    uart_puts_P("\n\n\rAIN1\tTarget\n\r");  // "Шапка" таблицы
    
    char term_buffer[30];   // Буфер для вывода в терминал
    int16_t reg = 0;            // Воздействие регулятора
    uint16_t target_reg = 0;    // Целевое положение по 1-му датчику
    
    while (1) {
        if (timer_main) {
            target_reg = target_adc(*idle_set, SECU_STEPS);
            secu_set_tps(tps(target_reg));    // Положение ДЗ для SECU-3
            if (pedal_switch())     // Если педаль газа нажата...
                reg = 0;            // регулятор не работает
            else                    // Если педаль не нажата - это режим ХХ,
                reg = regulator(sensor0(), target_reg);  // рассчитывается воздействие регулятора
            motor(reg);             // Управление приводом
            timer_main = 0;
        }
        if (timer_debug) {
            sprintf(term_buffer, "%u   \t%u   \r", sensor0(), target_reg);
            uart_puts(term_buffer);
            timer_debug = 0;
        }
    }
    
}
