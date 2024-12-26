

#ifndef SENSORS_H
#define SENSORS_H

void init_sensors ();

uint8_t tps (uint16_t reg_tps);

uint16_t target_adc(uint16_t value, uint16_t range);

uint16_t sensor0();
uint16_t sensor1();

// Состояние концевика заслонки
// 1 - педаль нажата, 0 - отпущена
uint8_t pedal_switch();

#endif
