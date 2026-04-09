.PHONY: all clean recover bootloader firmware

all: bootloader firmware

bootloader:
	$(MAKE) -C bootloader

firmware:
	@mkdir -p firmware/build
	cd firmware/build && cmake .. && $(MAKE)

clean:
	$(MAKE) -C bootloader clean
	rm -rf firmware/build
	rm -rf out

# Erase flash
recover:
	sudo picotool erase -a