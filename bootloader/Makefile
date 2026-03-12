CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

HACL_DIR = ../libs/hacl-star/dist/portable-gcc-compatible

MAKEFLAGS += -j$(nproc)
CFLAGS  = -mcpu=cortex-m33 -mthumb -mfloat-abi=soft \
          -Os \
          -ffreestanding -nostdlib \
          -ffunction-sections -fdata-sections \
		  -I$(HACL_DIR) \
		  -I../libs/hacl-star/dist/karamel/include \
          -I../libs/hacl-star/dist/karamel/krmllib/dist/minimal

LDFLAGS = -T bootloader.ld -nostdlib -Wl,--gc-sections

TARGET  = bootloader
BUILD_DIR = build
OBJS    = $(BUILD_DIR)/crt0.o $(BUILD_DIR)/embedded_start_block.o $(BUILD_DIR)/embedded_end_block.o $(BUILD_DIR)/main.o $(BUILD_DIR)/mem.o
LIB_OBJS = $(BUILD_DIR)/Hacl_Ed25519.o $(BUILD_DIR)/Hacl_Hash_SHA2.o \
           $(BUILD_DIR)/Hacl_Curve25519_51.o $(BUILD_DIR)/Hacl_EC_Ed25519.o

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).uf2

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) $(LIB_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(HACL_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/crt0.o: crt0.S
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/embedded_start_block.o: embedded_start_block.S
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/embedded_end_block.o: embedded_end_block.S
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/main.o: main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/mem.o: mem.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(TARGET).uf2: $(BUILD_DIR)/$(TARGET).elf
	picotool seal $< $(BUILD_DIR)/sealed_$(TARGET).elf
	$(OBJCOPY) -O binary $(BUILD_DIR)/sealed_$(TARGET).elf $(BUILD_DIR)/sealed_$(TARGET).bin
	python3 ../tools/uf2conv.py $(BUILD_DIR)/sealed_$(TARGET).bin -o $@ --family RP2350_ARM_S --base 0x10000000

clean:
	rm -rf $(BUILD_DIR)