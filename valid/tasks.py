evt = machine.ticker(10)

def delay(n):
    for _ in range(n):
        sys.suspend(evt) # TODO evt.wait()

async def task(rate):
    i = 0
    while evt:
        delay(rate)
        print('t', machine.ticks(), 'rate', rate, 'i', i)
        i += 1

for i in [2, 3, 5]:
    sys.tasks.append(task(i))

async def timeout():
    global evt
    delay(35)
    machine.ticker() # stops ticker and returns -1
    evt = None

sys.tasks.append(timeout())
