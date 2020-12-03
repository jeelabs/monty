qid = machine.ticker(10)

def delay(n):
    for _ in range(n):
        sys.suspend(qid)

async def task(rate):
    i = 0
    while qid > 0:
        delay(rate)
        print('t', machine.ticks(), 'rate', rate, 'i', i)
        i += 1

for i in [2, 3, 5]:
    sys.tasks.append(task(i))

async def timeout():
    global qid
    delay(35)
    qid = machine.ticker() # stops ticker and returns -1

sys.tasks.append(timeout())
