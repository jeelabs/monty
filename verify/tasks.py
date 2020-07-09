waiting = []

def delay(n):
    for _ in range(n):
        print('a',monty.tasks[0])
        monty.suspend(waiting)

async def task(rate):
    i = 0
    while True:
        delay(rate)
        print('t:', machine.ticks(), '\trate:', rate, ' i:', i)
        i += 1

for i in [2, 3, 5]:
    t = task(i)
    print('t', i, t)
    monty.tasks.append(t)

async def loop():
    global waiting, done
    i = 0
    while i < 35:
        i += 1
        for w in waiting:
            print('w',w)
            monty.tasks.append(w)
        waiting = []
        yield
        print('l',i)
    machine.timer(0, None) # allow main loop to exit
    yield # TODO can't return out of a coro yet

machine.timer(10, loop())
