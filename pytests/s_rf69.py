machine.dog(250) # get out of hang in init when RF69 is not present

evt = machine.ticker(10)

rf69 = machine.rf69("B7,B6,B0,A12", 63, 42, 868)
print(type(rf69))

async def loop():
    buf = array('B', 50);
    for _ in range(20):
        evt.wait()
        evt.clear()
        n = rf69.receive(buf)
        print(machine.ticks(), buf[:n])
    machine.ticker()

sys.tasks.append(loop())
