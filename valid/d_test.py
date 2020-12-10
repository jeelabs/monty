print('d_test ...')

# A tentative stream wrapper for UART serial ports.
#
# This blocks if needed or raises a timeout error when the timeout is reached.
# When the timeout value is changed, it will be used by all subsequent calls.

# uart API requirements:
#
#   count = uart.read(data, limit=-1, start=0, deadline=-1)
#       blocking receive, return number of bytes read, always > 0
#
#   count = uart.write(data, limit=-1, start=0, deadline=-1)
#       blocking send, return how many bytes have been sent out
#
uart = machine.uart()

# timeout used for next reads and writes (milliseconds, max ≈ 11 days)
#
timeout = 1000000000

# read bytes from stream, return when at least one has been read
def read(self, data, limit=-1, start=0):
    deadline = machine.ticks() + timeout
    if limit < 0:
        limit = cap(data)
    count = 0
    while count == 0:
        count = uart.read(data, limit, start, deadline)
    return count

# read as long as needed to read exactly the number pf bytes specified
def readcount(self, data, count):
    deadline = machine.ticks() + timeout
    start = 0
    while start < count:
        start += uart.read(data, count, start, deadline)
    return count

# read bytes up to an end of line mark (or any other delimiter)
def readline(self, data, delim=10):
    deadline = machine.ticks() + timeout
    start = 0
    while start == 0 or data[start-1] != delim:
        start += uart.read(data, start+1, start, deadline)
    return start # include delimiter

# write specified bytes to the stream, return once all have been written
def write(self, data, limit=-1, start=0):
    deadline = machine.ticks() + timeout
    if limit < 0:
        limit = len(data)
    while start < limit:
        start += uart.write(data, limit, start, deadline)
    return limit
