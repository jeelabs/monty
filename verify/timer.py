done = False

def loop():
    i = 0
    while i < 10:
        print(i, machine.ticks())
        i += 1
        yield
    global done
    done = True
    yield # TODO can't return out of a coro yet

machine.timer(10, loop())
while not done: pass
machine.timer(0, None) # allow main loop to exit
