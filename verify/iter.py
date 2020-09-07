a = iter(range(5))
print(type(a))
print(next(a))
print(next(a))
print(next(a))

b = (i*i for i in range(5))
print(type(b))
print(next(b))
print(next(b))
print(next(b))

#for i in b:
#    print(i)
