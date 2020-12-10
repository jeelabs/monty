a, b, c = [11,22,33]
print(a,b,c)

try:
    a, b, c = [11,22]
except BaseException as e:
    print(e)

try:
    a, b, c = [11,22,33,44]
except BaseException as e:
    print(e)

a, b, *c = [11,22]
print(a,b,c)

a, b, *c = [11,22,33]
print(a,b,c)

a, b, *c = [11,22,33,44]
print(a,b,c)

try:
    a, b, *c = [11]
except BaseException as e:
    print(e)

a, *b, c = [11,22]
print(a,b,c)

a, *b, c = [11,22,33]
print(a,b,c)

a, *b, c = [11,22,33,44]
print(a,b,c)

*a, b, c = [11,22]
print(a,b,c)

*a, b, c = [11,22,33]
print(a,b,c)

*a, b, c = [11,22,33,44]
print(a,b,c)
