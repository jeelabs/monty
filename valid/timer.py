evt = machine.ticker(10)

async def loop():
    i = 0
    while i < 10:
        print(i, machine.ticks())
        i += 1
        sys.suspend(evt) # TODO evt.wait()
    machine.ticker()

sys.tasks.append(loop())
