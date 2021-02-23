Connect a [Nucleo-L432KC][L432] as follows:

![](image.jpg)

The 6 extra LEDs on A0..A5 each have a series resistor, and A6 is tied to GND.

To build and run (this echos the underlying cmd used to do the work):

```text
$ inv
pio run -c ../pio-examples.ini -s
$
```

All the LEDs will start blinking, each one at a different rate.  
The on-board LED will blink even if no other LEDs are connected.

[L432]: https://www.st.com/en/evaluation-tools/nucleo-l432kc.html
