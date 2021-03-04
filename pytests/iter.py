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

c = [111,222,333]
print(type(iter(c)))
for i in c:
    print(i)

print(tuple())
print(tuple([1,2,3]))
print(tuple({1,2,3}))
print(tuple((i*i for i in range(6))))

print(list())
print(list([1,2,3]))
print(list({1,2,3}))
print(list((i*i for i in range(6))))

print(set())
print(set([1,2,3]))
print(set({1,2,3}))
print(set((i*i for i in range(5))))

print(dict())
print(dict({1:11,2:22,3:33}))
print(dict([(11,111),(22,222),(33,333)]))
print(dict(((i,i*i) for i in range(6))))
