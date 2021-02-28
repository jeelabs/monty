import sys, machine

evt = machine.ticker(10)

async def loop():
    i = 0
    while i < 10:
        print(i, machine.ticks())
        i += 1
        evt.wait()
        evt.clear()
    machine.ticker()

sys.ready.append(loop())
