def loop():
    i = 0
    while i < 10:
        print(i, machine.ticks())
        i += 1
        yield
    machine.ticker()

machine.ticker(10, loop())
