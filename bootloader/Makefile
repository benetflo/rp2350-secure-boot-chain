CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

HACL_DIR = ../libs/hacl-star/dist/portable-gcc-compatible

MAKEFLAGS += -j$(nproc)

BUILD_TYPE ?= debug

CFLAGS += -mcpu=cortex-m33 -mthumb -mfloat-abi=soft -Os \
        -ffreestanding -nostdlib -ffunction-sections -fdata-sections \
        -I$(HACL_DIR) \
        -I../libs/hacl-star/dist/karamel/include \
        -I../libs/hacl-star/dist/karamel/krmllib/dist/minimal \
        $(if $(filter release,$(BUILD_TYPE)),-DNDEBUG,) \

LDFLAGS = -T bootloader.ld -nostdlib -Wl,--gc-sections

TARGET = bootloader
BUILD  = build

OBJS = $(BUILD)/crt0.o $(BUILD)/embedded_start_block.o $(BUILD)/embedded_end_block.o \
        $(BUILD)/main.o $(BUILD)/mem.o $(BUILD)/boot_utils.o

LIB_OBJS = $(BUILD)/Hacl_Ed25519.o $(BUILD)/Hacl_Hash_SHA2.o \
            $(BUILD)/Hacl_Curve25519_51.o $(BUILD)/Hacl_EC_Ed25519.o

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).uf2

$(BUILD)/$(TARGET).elf: $(OBJS) $(LIB_OBJS)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Generic rule for HACL sources
$(BUILD)/%.o: $(HACL_DIR)/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

# Generic rule for local C and ASM sources
$(BUILD)/%.o: %.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.S
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

# Seal and sign bootloader
UF2_OUT := ../out/bootloader
SIGNED_DIR := $(UF2_OUT)/signed
UNSIGNED_DIR := $(UF2_OUT)/unsigned

$(BUILD)/$(TARGET).uf2: $(BUILD)/$(TARGET).elf
    # Create output directories
	mkdir -p $(SIGNED_DIR)
	mkdir -p $(UNSIGNED_DIR)

	picotool seal --sign \
        $(BUILD)/$(TARGET).elf \
        $(BUILD)/signed_$(TARGET).elf \
        ../bootloader_private_key.pem \
        ../otp.json

	$(OBJCOPY) -O binary \
        $(BUILD)/signed_$(TARGET).elf \
        $(BUILD)/signed_$(TARGET).bin

	python3 ../tools/uf2conv.py \
        $(BUILD)/signed_$(TARGET).bin \
        -o $(BUILD)/signed_$(TARGET).uf2 \
        --family RP2350_ARM_S --base 0x10000000

    # unsigned versions
	$(OBJCOPY) -O binary \
        $(BUILD)/$(TARGET).elf \
        $(BUILD)/unsigned_$(TARGET).bin

	python3 ../tools/uf2conv.py \
        $(BUILD)/$(TARGET).elf \
        -o $(BUILD)/unsigned_$(TARGET).uf2 \
        --family RP2350_ARM_S --base 0x10000000

    # Copy to output structure
	cp $(BUILD)/signed_$(TARGET).uf2 $(SIGNED_DIR)/bootloader.uf2
	cp $(BUILD)/signed_$(TARGET).bin $(SIGNED_DIR)/bootloader.bin

	cp $(BUILD)/unsigned_$(TARGET).uf2 $(UNSIGNED_DIR)/bootloader.uf2
	cp $(BUILD)/unsigned_$(TARGET).bin $(UNSIGNED_DIR)/bootloader.bin

clean:
	rm -rf $(BUILD)
