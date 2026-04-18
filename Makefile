.PHONY: all clean recover bootloader firmware debug production

BUILD_TYPE ?= debug

debug:
	$(MAKE) -C bootloader BUILD_TYPE=debug
	@mkdir -p firmware/build
	cd firmware/build && cmake -DCMAKE_BUILD_TYPE=Debug .. && $(MAKE)

production:
	$(MAKE) -C bootloader BUILD_TYPE=release
	@rm -rf firmware/build
	@mkdir -p firmware/build
	cd firmware/build && cmake -DCMAKE_BUILD_TYPE=Release .. && $(MAKE)

clean:
	$(MAKE) -C bootloader clean
	rm -rf firmware/build
	rm -rf out

# Erase flash
recover:
	sudo picotool erase -a