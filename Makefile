help:
	# make run   - build and run a native (macos/linux) version
	# make up    - build and upload to an attached board
	# make mon   - build, upload, and view output of an attached board

run: verify/demo.mpy
	src/codegen.py lib/monty/
	pio run -c configs/native.ini -s
	.pio/build/native/program $<

up: platformio.ini
	pio run -t upload -s

mon: platformio.ini
	pio run -t upload -t monitor -s

many:
	pio run -c configs/native.ini -s
	pio run -c configs/bluepill_f103c8.ini -s
	pio run -c configs/esp8266.env -e d1_mini -s
	pio run -c configs/tinypico.ini -s

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
	# Make a thin wrapper around PlatformIO, its use is optional.
	@exit 1

%.mpy : %.py
	mpy-cross $<

clean:
	rm -rf .pio
