B = native
T = gc
V = verify

help:
	# make all    - shorthand for "make run testall verify"
	# make run    - build and run a native (macos/linux) version
	# make test   - build and run the native (macos/linux) C++ tests
	# make verify - build and run the native (macos/linux) script tests
	# make up     - build and upload to an attached board
	# make mon    - build, upload, and view output of an attached board

all: run testall verify

run: gen native $V/features.mpy
	.pio/build/native/program $V/features.mpy $V/rom.mrfs

test: gen platformio.ini
	pio test -c configs/$B.ini -f $T

testall: gen platformio.ini
	pio test -c configs/$B.ini

up: platformio.ini
	pio run -t upload -s

mon: platformio.ini
	pio run -t upload -t monitor -s

many: native bluepill_f103c8 disco_f407vg esp8266 _tinypico
native bluepill_f103c8 disco_f407vg esp8266 _tinypico:
	pio run -c configs/$@.ini -s

monty: gen native
	@ cp -a .pio/build/native/program $@

gen:
	src/codegen.py qstr.h lib/monty/ qstr.cpp

verify:
ifeq ($B,native)
	pio run -c configs/$B.ini -s
else
	pio run -c configs/$B.ini -t upload -s && sleep 0.5
endif
	@ make -C $@ BOARD=$B

cov:
	rm -rf coverage/
	mkdir coverage
	pio run
	for i in $V/*.py; do \
	    .pio/build/native/program $$i $V/rom.mrfs; \
	done | wc
	.pio/build/native/program -c $V/hello.mpy
	lcov -d .pio/build/native/ -c -o coverage/lcov.info
	genhtml -o coverage/ --demangle-cpp coverage/lcov.info
	cd coverage && python -m SimpleHTTPServer 8000

platformio.ini:
	# To get started, please create a "platformio.ini" file.
	#
	# Example 1, to build and upload to a TinyPICO using "make up":
	#     [platformio]
	#     extra_configs = configs/tinypico.ini
	#     [env]
	#     upload_port = /dev/cu.usbserial-01C85A14
	#
	# Example 2, to build, upload, and view output using "make mon":
	#     [platformio]
	#     extra_configs = configs/bluepill_f103c8.ini
	#     [env]
	#     upload_port = /dev/cu.usbmodemDDD8B7B81
	#     monitor_port = /dev/cu.usbmodemDDD8B7B83
	#
	# Make is a thin wrapper around PlatformIO, its use is optional.
	@exit 1

%.mpy : %.py
	mpy-cross $<

docs:
	rsync -av --delete docs/ ~/Nextcloud/monty-docs/
tags:
	ctags -R src/ lib/monty/ test/
clean:
	rm -rf .pio monty

.PHONY: docs tags test verify
