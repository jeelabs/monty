print(monty.version)

network.ifconfig('192.168.188.2', '255.255.255.0', '192.168.188.1', '8.8.8.8')

s = network.socket()
print(s)
s.bind(1234)

async def accept(sess):
    print('accept', sess)
    b = sess.read(100)
    print('read1', b, len(b))
    print('\t',b[0],b[1],b[2])
    b = sess.read(100)
    print('read2', b, len(b))
    while True:
        #sess.write('mmmmmmmm\n')
        sess.write(b'mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm\n')
        print('written')

s.listen(accept, 3)

def loop():
    for _ in range(1000):
        network.poll()
        yield
    machine.ticker() # allows main loop to exit
    yield # TODO can't return out of a coro yet

machine.ticker(10, loop())
