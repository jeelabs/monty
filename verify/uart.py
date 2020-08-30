writers = [] # TODO
async def handler():
    print('h')
    while True:
        yield
        print('R!', len(writers))
        #for w in writers:
        #    sys.tasks.append(w)
        #writers.clear()
        if len(writers) > 0:
            sys.tasks.append(writers[0])
        writers.clear()

h = handler()
next(h)
uart = machine.uart(h, writers)

n1 = uart.write(b'1234567890\n', -1, 0, -1)
n2 = uart.write(b'1234567abc\n', -1, 7, -1)
n3 = uart.write(b'123defg\n456', 8, 3, -1)
print(n1, n2, n3)

if False:
    import d_test as console
    console.write(b'console write\n')

    def print(*args):
        sep = b''
        for a in args:
            console.write(sep)
            console.write(a)
            sep = b' '
        console.write(b'\n')

    print(b'abc', b'def')

def cwrite(data, limit=-1, start=0):
    deadline = machine.ticks() + 10000000
    if limit < 0:
        limit = len(data)
    while start < limit:
        print('#',limit,start)
        n = uart.write(data, limit, start, deadline)
        print('<',n)
        start += n
    return limit

for _ in range(10):
    #console.write(b'12345678901234567890123456789012345678901234567890\n')
    cwrite(b'1234567890123456789012345678901234567890123456789\n')

#uart.close() # TODO