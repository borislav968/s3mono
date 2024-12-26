
#ifndef CONFIG_H
#define CONFIG_H

// Тактовая частота
#define F_CPU 8000000UL

// Скорость UART
#define UART_BAUD_RATE 57600

// Конфигурация МК
FUSES = {
    .low = FUSE_SUT0,
    .high = HFUSE_DEFAULT,
};

// Число шагов РХХ в SECU-3
#define SECU_STEPS 400

// Настройки приода РХХ
#define K_P 14  //пропорциональный коэффициент
#define K_I 1   // интегралный коэффициент (/10)
#define MAX_SUM_ERROR 2048  // макс. накопленная ошибка
#define MIN_ERROR 4         // порог ошибки
#define BUMP_DETECT 12000   // накопл. воздействие регулятора для определения упора


#endif
