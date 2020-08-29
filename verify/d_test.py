print('d_test ...')

# uart API:
#
#   count = uart.read(data, limit=-1, start=0, deadline=-1)
#       blocking receive, return number of bytes read, always > 0
#
#   count = uart.write(data, limit=-1, start=0, deadline=-1)
#       blocking send, return how many bytes have been sent out
#
uart = machine.uart(1)

# timeout used for next reads and writes (milliseconds, max ≈ 11 days)
#
timeout = 1000000000

def read(data, limit=-1, start=0):
    deadline = machine.ticks() + timeout
    if limit < 0:
        limit = cap(data)
    count = 0
    while count == 0:
        count = uart.read(data, limit, start, deadline)
    return count

def readcount(data, count):
    deadline = machine.ticks() + timeout
    start = 0
    while start < count:
        start += uart.read(data, count, start, deadline)
    return count

def readline(data, delim=10):
    deadline = machine.ticks() + timeout
    start = 0
    while start == 0 or data[start-1] != delim:
        start += uart.read(data, start+1, start, deadline)
    return start # include newline

def write(data, limit=-1, start=0):
    deadline = machine.ticks() + timeout
    if limit < 0:
        limit = len(data)
    while start < limit:
        start += uart.write(data, limit, start, deadline)
    return limit
