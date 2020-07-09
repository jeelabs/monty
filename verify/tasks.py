waiting = []

def delay(n):
    for _ in range(n):
        monty.suspend(waiting)

async def task(rate):
    i = 0
    while True:
        delay(rate)
        print('t:', machine.ticks(), '\trate:', rate, ' i:', i)
        i += 1

for i in [2, 3, 5]:
    monty.tasks.append(task(i))

async def loop():
    global waiting, done
    i = 0
    while i < 35:
        i += 1
        for w in waiting:
            monty.tasks.append(w)
        waiting = []
        yield
    machine.timer(0, None) # allows main loop to exit
    yield # TODO can't return out of a coro yet

machine.timer(10, loop())
