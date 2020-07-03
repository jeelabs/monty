def loop():
    i = 0
    while True:
        print(i, getTime())
        i += 1
        yield

setTimer(500, loop())

while True: pass
