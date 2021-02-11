# see https://stackoverflow.com/questions/12637768/

def gen():
    for i in range(10):
        X = yield i
        print("  inside ", X)

m = gen()
print("1 outside", next(m))
print("2 outside", next(m))
print("3 outside", next(m))
print("4 outside", next(m))

print("5 result:", m.send(0)) # Start generator
print("6 result:", m.send(77))
print("7 result:", m.send(88))
print("8 result:", m.send(99))
print("9 result:", m.send(0))
