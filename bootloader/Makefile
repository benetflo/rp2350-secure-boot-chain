CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS  = -mcpu=cortex-m33 -mthumb -mfloat-abi=soft \
          -Os -g \
          -ffreestanding -nostdlib \
          -ffunction-sections -fdata-sections

LDFLAGS = -T bootloader.ld -nostdlib -Wl,--gc-sections

TARGET  = bootloader
OBJS    = crt0.o embedded_start_block.o embedded_end_block.o main.o

all: $(TARGET).elf $(TARGET).uf2

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

crt0.o: crt0.S
	$(CC) $(CFLAGS) -c -o $@ $<

embedded_start_block.o: embedded_start_block.S
	$(CC) $(CFLAGS) -c -o $@ $<

embedded_end_block.o: embedded_end_block.S
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET).uf2: $(TARGET).elf
	picotool seal $< sealed_$(TARGET).elf
	$(OBJCOPY) -O binary sealed_$(TARGET).elf sealed_$(TARGET).bin
	python3 ../tools/uf2conv.py sealed_$(TARGET).bin -o $@ --family RP2350_ARM_S --base 0x10000000

clean:
	rm -f *.o *.elf *.uf2 *.bin
