all: platformio.ini
	pio run -t upload -t monitor -s
