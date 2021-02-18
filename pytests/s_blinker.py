import sys, machine

evt = machine.ticker(10)

def delay(n):
    for _ in range(n):
        evt.wait()
        evt.clear()

async def task(rate):
    i = 0
    machine.pins.B3 = "P" # mode: push-pull output
    while True:
        machine.pins.B3 = 1
        delay(10)
        machine.pins.B3 = 0
        delay(rate)
        i += 1

for i in [25]:
    sys.tasks.append(task(i))

async def timeout():
    print(machine.pins)
    for i in range(8):
        delay(9)
        print(i, machine.pins.B3, machine.ticks())
    machine.ticker()

sys.tasks.append(timeout())
