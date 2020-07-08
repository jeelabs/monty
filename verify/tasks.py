def delay(n):
    for _ in range(n):
        suspend()

async def task(rate):
    i = 0
    while True:
        delay(rate)
        print('t:', machine.ticks(), '\trate:', rate, ' i:', i)
        i += 1

tasks = [task(2), task(3), task(5)]
done = False

def loop():
    i = 0
    while i < 35:
        i += 1
        for t in tasks:
            next(t)
        yield
    global done
    done = True
    yield # TODO can't return out of a coro yet

machine.timer(10, loop())
while not done: pass
machine.timer(0, None) # allow main loop to exit
