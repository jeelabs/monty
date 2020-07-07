def loop():
    i = 0
    while True:
        print(i, monty.ticks())
        i += 1
        yield

monty.timer(500, loop())

while True: pass
