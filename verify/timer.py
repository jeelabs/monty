def loop():
    i = 0
    while i < 10:
        print(i, machine.ticks())
        i += 1
        yield
    machine.timer() # allows main loop to exit
    yield # TODO can't return out of a coro yet

machine.timer(10, loop())
