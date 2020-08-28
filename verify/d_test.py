print('d_test ...')

uart = machine.uart(1)

# uart API:
#
#   count = uart.send(data, limit, start)
#       send bytes starting at start, return number of bytes written
#
#   uart.done(id)
#       tell the uart which soft-irq to trigger when done

work = []   # list of bytes to send
queue = []  # list of corresponding tasks

def write(data):
    work.append(data)
    sys.suspend(queue)
    sys.trigger(id)

async def handler():
    data, start, limit = b'', 0, 0
    while True:
        yield
        while queue:
            if start >= limit:
                data = work.pop(0)
                start = 0
                limit = len(data)
            start += uart.write(data, limit, start)
            if start >= limit:
                sys.resume(queue.pop(0))

id = sys.register(handler())
uart.done(id)





# uart API:
#
#   offset = uart.send(data, offset)
#       send bytes starting at offset, return new offset
#
#   uart.done(id)
#       tell the uart which soft-irq to trigger when done

work = []   # list of bytes to send
queue = []  # list of corresponding tasks
remain = 0  # number of bytes left to send in work[0]

def write(data):
    global remain
    if remain == 0:
        remain = len(data) - uart.send(data)
        if remain == 0:
            return
    # what if the uart.done irq happens here?
    work.append(data)
    return sys.suspend(queue)

#def write(data):
#    work.append(data)
#    sys.suspend(queue)
#    sys.trigger(id)

async def handler():
    global remain
    while True:
        yield
        while work:
            data = work[0]
            n = len(data)
            remain = n - uart.send(data, n - remain)
            if remain == 0:
                work.pop(0)
                sys.resume(queue.pop(0))

id = sys.register(handler())
uart.done(id)
