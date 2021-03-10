def a():
    yield 10
    yield 11
    yield from (200+i for i in range(2))
    yield 3000
    yield 3001

for i in a():
    print(i)

b = a()
print(type(b))

while True:
    try:
        print(next(b))
    except StopIteration:
        break
