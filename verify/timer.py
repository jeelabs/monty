async def loop():
    i = 0
    while i < 10:
        print(i, machine.ticks())
        i += 1
        sys.suspend(1)
    machine.ticker()

machine.ticker(10, loop())
