print(1)
print(10)
print(100)
print(1000)
print(10000)
print(100000)
print(1000000)
print(10000000)
print(100000000)
print(1000000000)
print(10000000000)
print(100000000000)
print(1000000000000)
print(10000000000000)
print(100000000000000)
print(1000000000000000)
print(10000000000000000)
print(100000000000000000)
print(1000000000000000000)
print(-1000000000000000000)

# add a few expressions to improve test coverage
print(1&2, 1|2, 1^2, 1<<2, 1>>2, 1/2, 1%2)
try:
    a = 1 % 0
except ZeroDivisionError as e:
    print(e, e.trace())
