This has been configured to run on a [Nucleo-L432KC][L432].

To build and run (this echos the underlying cmd used to do the work):

```text
$ inv
pio run -c ../pio-examples.ini -s
$
```

The on-board LED will toggle as fast as stacklet switching allows.  
This can be measured with a frequency counter on pin "D13".

[L432]: https://www.st.com/en/evaluation-tools/nucleo-l432kc.html
