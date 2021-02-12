def a():
    yield 10
    yield 11
    yield from g()
    yield 3000
    yield 3001

def g():
    yield 200
    yield 201

for i in a():
    print(i)

b = a()
print(type(b))

while True:
    try:
        print(next(b))
    except StopIteration:
        break
