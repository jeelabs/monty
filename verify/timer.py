async def loop():
    i = 0
    while i < 15:
        print(i, machine.ticks())
        i += 1
        sys.suspend(id)
    machine.ticker()

id = machine.ticker(10, loop())
