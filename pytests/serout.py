import sys, machine

evt = machine.ticker(100)

def delay(n):
    for _ in range(n):
        evt.wait()
        evt.clear()

async def task(rate, msg):
    while True:
        print(msg)
        delay(1)
        print(msg)
        delay(rate)

sys.ready.append(task(1, "1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."))
sys.ready.append(task(2, "22..22..22..22..22..22..22..22..22..22.."))
sys.ready.append(task(3, "333...333...333...333...333...333...333."))

async def timeout():
    global evt
    delay(20)
    machine.ticker()

sys.ready.append(timeout())
