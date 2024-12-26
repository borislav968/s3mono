
#ifndef SECU_H
#define SECU_H

#include <stdint.h>

// Настройка обмена с SECU-3
void init_secu_connection( volatile uint16_t** steps, uint16_t steps_max );


// Выход сигнала положения ДПДЗ для SECU
// value 0..255 соответствует напряжению 0..5В на выходе
void secu_set_tps (uint8_t value);

#endif
