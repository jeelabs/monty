BOARD ?= bluepill
M = MALLOC_CHECK_=3

all: gen test native runner/demo.mpy
	$M .pio/build/native/program runner/demo.mpy

gen:
	./codegen.py

test:
	$M pio test

runner: native
	@ make -C runner BOARD=$<

native native17:
	pio run -e $@ -s

bluepill f4disco tinypico:
	pio test -e $@
	pio run -e $@ -t upload -s
	make -C runner BOARD=$@

# print just the ELF sections and their sizes
sections:
	@ arm-none-eabi-objdump -h .pio/build/$(BOARD)/firmware.elf | \
	    grep -v '^   '

# print a massaged version of the ELF symbols, restricted to 80-col output
symbols:
	@ arm-none-eabi-objdump -tC .pio/build/$(BOARD)/firmware.elf | \
	  sort -u | awk -n 'BEGIN { FS="\t" } substr($$1, 18, 1) == "." { \
			    i = int("0x" substr($$2,1,9)); \
			    if (i > 0) printf "%.10s%-12s%4d%.54s\n", $$1, \
					substr($$1,15,12), i, substr($$2,9) }'

%.mpy : %.py
	mpy-cross $<

clean:
	rm -rf .pio

.PHONY: test runner
