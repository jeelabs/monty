waiting = []

async def delay(n):
    for _ in range(n):
        sys.suspend(id, waiting)

async def task(rate):
    i = 0
    while True:
        delay(rate)
        print('t', machine.ticks(), 'rate', rate, 'i', i)
        i += 1

for i in [2, 3, 5]:
    sys.tasks.append(task(i))

def loop():
    global waiting
    for _ in range(35):
        for w in waiting:
            sys.tasks.append(w)
        waiting = []
        yield
    machine.ticker()

id = machine.ticker(10, loop())
