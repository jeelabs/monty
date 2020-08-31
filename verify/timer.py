qid = machine.ticker(10)

async def loop():
    i = 0
    while i < 15:
        print(i, machine.ticks())
        i += 1
        sys.suspend(qid)
    machine.ticker()

sys.tasks.append(loop())
