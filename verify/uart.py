uart = machine.uart()

n1 = uart.write(b'1234567890\n', -1, 0, -1)
n2 = uart.write(b'1234567abc\n', -1, 7, -1)
n3 = uart.write(b'123defg\n456', 8, 3, -1)
print(n1, n2, n3)

def cwrite(data):
    start = 0
    limit = len(data)
    while start < limit:
        #print('#',start)
        start += uart.write(data, limit, start, -1)
    return limit

for _ in range(10):
    cwrite(b'123456789-123456789+123456789~123456789=123456789\n')

uart.close() # TODO
