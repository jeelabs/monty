a = iter(range(5))
print(type(a))
print(next(a))
print(next(a))
print(next(a))

b = (100+i*i for i in range(6))
print(type(b))
print(next(b))
print(next(b))
print(next(b))

print(type(iter(b)))

#for i in b:
#    print(i)

for i in (11,22,33):
    print(i)
