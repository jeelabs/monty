tick = machine.ticker(1000)

def delay(n):
    for _ in range(n):
        tick.wait()
        tick.clear()

async def waiter(evt,num):
    print('waiting for it ...', num)
    evt.wait()
    print('... got it!', num)

async def main():
    # Create an Event object.
    evt = sys.event()

    # Spawn a Task to wait until 'evt' is set.
    waiter_task1 = waiter(evt, 1)
    waiter_task2 = waiter(evt, 2)
    waiter_task3 = waiter(evt, 3)

    sys.tasks.append(waiter_task1)
    sys.tasks.append(waiter_task2)
    sys.tasks.append(waiter_task3)

    # Sleep for 1 second and set the event.
    delay(1)
    evt.set()
    print(111)

    # Wait until the waiter task is finished.
    await waiter_task1
    print(222)
    await waiter_task2
    print(333)
    await waiter_task3
    print(444)

    # Done.
    machine.ticker()

sys.tasks.append(main())
