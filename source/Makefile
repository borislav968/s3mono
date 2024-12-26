
# Имя проекта: берётся из имени каталога
PROJECT := $(notdir $(shell cd ..; pwd))

# Тип МК
MCU = atmega8

# Тип программатора (для avrdude)
PROGRAMMER = avrisp2

# Вызов GCC
GCC = avr-gcc -Wall -Os -mmcu=$(MCU)

# Вызов objcopy
OBJCOPY = avr-objcopy

# Подкаталог для сборки
BDIR = build

# Список исходников
SRC = $(wildcard *.c)

# Список объектных файлов
OBJ = $(patsubst %.c, $(BDIR)/%.o, $(SRC))

.PHONY: all hex clean flash fuses

# Сборка ELF-файла
all: $(BDIR)/$(PROJECT).elf

# Сборка образа Flash
hex: $(BDIR)/$(PROJECT).hex

# Очистка каталога
clean:
	rm -rf $(BDIR)

# Запись Fuses в контроллер
fuses:	$(BDIR)/$(PROJECT).elf
	# сохранить секцию .fuse из ELF
	$(OBJCOPY) -j .fuse -O ihex $(BDIR)/$(PROJECT).elf $(BDIR)/fuses.hex --change-section-lma .fuse=0
	# Выделить байт LFUSE
	srec_cat $(BDIR)/fuses.hex -Intel -crop 0x00 0x01 -offset 0x00 -O $(BDIR)/lfuse.hex -Intel
	# Выделить байт HFUSE
	srec_cat $(BDIR)/fuses.hex -Intel -crop 0x01 0x02 -offset -0x01 -O $(BDIR)/hfuse.hex -Intel
	# Запись в МК
	avrdude -c $(PROGRAMMER) -p $(MCU) -V -U hfuse:w:$(BDIR)/hfuse.hex:i -U lfuse:w:$(BDIR)/lfuse.hex:i
	
# Запись Flash в контроллер
flash:	$(BDIR)/$(PROJECT).hex
	avrdude -c $(PROGRAMMER) -p $(MCU) -V -U flash:w:$(BDIR)/$(PROJECT).hex:i
	
# Создание подкаталога для сборки
$(BDIR):
	mkdir $(BDIR)

# Создание образа Flash
$(BDIR)/$(PROJECT).hex: $(BDIR)/$(PROJECT).elf
	# Сохранить секции .text и .data в новый файл
	$(OBJCOPY) -j .text -j .data -O ihex $(BDIR)/$(PROJECT).elf $(BDIR)/$(PROJECT).hex

# Сборка ELF
$(BDIR)/$(PROJECT).elf: $(OBJ)
	$(GCC) $(OBJ) -o $(BDIR)/$(PROJECT).elf

# Компиляция
$(BDIR)/%.o: %.c | $(BDIR)
	$(GCC) -c $< -o $@
