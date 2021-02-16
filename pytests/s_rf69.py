evt = machine.ticker(10)

rf69 = machine.rf69("A4,A5,A6,A7", 63, 42, 868)
print(machine.ticks(), type(rf69))

async def loop():
    buf = array('B', 50);
    for _ in range(20):
        n = rf69.receive(buf)
        print(machine.ticks(), buf[:n])
        evt.wait()
        evt.clear()
    machine.ticker()

sys.tasks.append(loop())
