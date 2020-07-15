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

def loop():
    global waiting
    for _ in range(35):
        for w in waiting:
            monty.tasks.append(w)
        waiting = []
        yield
    machine.ticker() # allows main loop to exit
    yield # TODO can't return out of a coro yet

machine.ticker(10, loop())
