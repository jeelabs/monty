BOARD ?= bluepill_f103c8

ifeq ($(BOARD),bluepill_f103c8)
gdb = /dev/cu.usbmodemDDD8B7B81
tty = /dev/cu.usbmodemDDD8B7B83
ram = 0x20004000
else ifeq ($(BOARD),disco_f407vg)
tty = /dev/cu.usbmodem1461303
ram = 0x2001F000
endif

exe = ../.pio/build/$(BOARD)

src := $(wildcard *.py)
mpy := $(patsubst %.py,%.mpy,$(src))
out := $(patsubst %.py,%.out,$(src))
exp := $(patsubst %.py,%.exp,$(src))

all: check dog fresh $(exp)

# check that the firmware is present and the board is connected
check: $(exe) $(tty)
ifdef tty
	@ echo target: $(BOARD) on $(tty)
endif

# dog is like cat, but it sets the baudrate and quits on idle
dog: dog.c

fresh:
	@ rm -f $(out)

clean:
	rm -f $(mpy) $(out)

# compare output to expected (show only 10 diff lines), else show word count
%.exp : %.out %.mpy
	@ if [ -f $@ ]; then (diff $< $@ && rm $<) | head; else wc $<; fi

# 1) store bytecode in ram, 2) reset the µC, 3) capture output to file
%.out : %.mpy
	@ echo $@
ifeq ($(BOARD), native)
	@ $(exe)/program $< >$@
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
	@ rm $<

%.mpy : %.py
	@ mpy-cross $<

.PRECIOUS: $(out)
