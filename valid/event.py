evt = machine.ticker(10)

evt.set()
assert evt
evt.clear()
assert not evt

i = 0
while i < 10:
    print(i, machine.ticks())
    i += 1
    evt.wait()
    evt.clear()

machine.ticker()
