help:
	# make all    - shorthand for "make run testall verify"
	# make run    - build and run a native (macos/linux) version
	# make test   - build and run the native (macos/linux) C++ tests
	# make verify - build and run the native (macos/linux) script tests
	# make up     - build and upload to an attached board
	# make mon    - build, upload, and view output of an attached board

all: run testall verify

run: gen native verify/demo.mpy
	.pio/build/native/program verify/demo.mpy

test: gen platformio.ini
	pio test -e native

testall: gen platformio.ini
	pio test -e native -f '*'

up: platformio.ini
	pio run -t upload -s

mon: platformio.ini
	pio run -t upload -t monitor -s

many: native bluepill_f103c8 disco_f407vg esp8266 tinypico
native bluepill_f103c8 disco_f407vg esp8266 tinypico:
	pio run -c configs/$@.ini -s

gen:
	src/codegen.py qstr.h lib/monty/ qstr.cpp

verify:
	pio run -c configs/native.ini -s
	@ make -C $@ BOARD=native

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
	rm -rf .pio

.PHONY: docs tags test verify
