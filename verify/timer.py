done = False

def loop():
    i = 0
    while i < 10:
        print(i, monty.ticks())
        i += 1
        yield
    global done
    done = True
    yield # TODO can't return out of a coro yet

setTimer(10, loop())
while not done: pass
setTimer(0, None) # allow main loop to exit
