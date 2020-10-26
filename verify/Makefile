#BOARD ?= bluepill_f103c8
#BOARD ?= genericSTM32F103RC-enc28j60
#BOARD ?= genericSTM32F103ZE-enc28j60
BOARD ?= disco_f407vg

ifeq ($(BOARD),bluepill_f103c8)
gdb = /dev/cu.usbmodemDDD8B7B81
tty = /dev/cu.usbmodemDDD8B7B83
ram = 0x20004000 # 20 kB
else ifeq ($(BOARD),genericSTM32F103RC-enc28j60)
gdb = /dev/cu.usbmodemE2C2B4DF1
tty = /dev/cu.usbmodemE2C2B4DF3
ram = 0x2000B000 # 48 kB
else ifeq ($(BOARD),genericSTM32F103ZE-enc28j60)
gdb = /dev/cu.usbmodemE4D3BFC11
tty = /dev/cu.usbmodemE4D3BFC13
ram = 0x2000F000 # 64 kB
else ifeq ($(BOARD),disco_f407vg)
tty = /dev/cu.usbmodem1431303
ram = 0x2001F000 # 192 kB
else ifeq ($(BOARD),nucleo_l010rb)
tty = /dev/cu.usbmodem1461203
ram = 0x20004000 # 20 kB
else ifeq ($(BOARD),nucleo_l412kb)
tty = /dev/cu.usbmodem1431203
ram = 0x20009000 # 40 kB
else ifeq ($(BOARD),nucleo_l432kc)
tty = /dev/cu.usbmodem1431203
ram = 0x2000B000 # 48 kB
else ifeq ($(BOARD),nucleo_f767zi)
tty = /dev/cu.usbmodem1431203
ram = 0x2007F000 # 512 kB
endif

src := $(wildcard [a-z][a-z]*.py)
mpy := $(patsubst %.py,%.mpy,$(src))
out := $(patsubst %.py,%.out,$(src))
exp := $(patsubst %.py,%.exp,$(src))

all: check rom.mrfs dog fresh $(exp)

# check that the firmware is present and the board is connected
check: $(tty)
ifdef tty
	@ echo target: $(BOARD) on $(tty)
endif

# dog is like cat, but it sets the baudrate and quits on idle
dog: dog.c

# build an image with all the modules for (simulated) flash use
rom.mrfs: *.py
	@ ../src/mrfs.py -o $@ $^
	@ ls -l $@

fresh:
	@ rm -f $(out)

clean:
	rm -f dog *.mpy *.mty *.out

# compare output to expected (show first 10 diff lines), else show word count
# don't show anything if the .exp file is empty, as it differs from run to run
# (native assertion failures will always show, since they are sent to stderr)
%.exp : %.out %.mpy
	@ if [ -s $@ ]; then (diff $< $@ && rm $<) | head; fi
	@ if [ ! -f $@ ]; then wc -l $<; fi

# 1) store bytecode in ram, 2) reset the µC, 3) capture output to file
%.out : %.mpy rom.mrfs
	@ echo "   $@:"
ifeq ($(BOARD), native)
	@ ../.pio/build/native/program $^ >$@
else ifdef gdb
	@ ./dog $@ <$(tty) & pid=$$!; \
	  arm-none-eabi-gdb -nx --batch -ex "tar ext $(gdb)" -ex "mon swdp" \
	    -ex "at 1" -ex "restore $< binary $(ram)" -ex kill >&/dev/null; \
	  wait $$pid
else
	@ ./dog $@ <$(tty) & pid=$$!; \
	  st-flash --reset write $< $(ram) >&/dev/null; \
	  wait $$pid
endif

%.mpy : %.py
	@ mpy-cross $<

.PRECIOUS: $(mpy) $(out)
