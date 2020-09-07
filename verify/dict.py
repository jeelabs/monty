a = {1:2,3:4,5:6}
print(a)

for i in a:
    print(i)

d = a.keys()
print(type(d), len(d))
for i in d:
    print(i)

d = a.values()
print(type(d), len(d))
for i in d:
    print(i)

d = a.items()
print(type(d), len(d))
for i in d:
    print(i)

a[3] = 44;
a[7] = 88;

print(type(d), len(d))
for i in d:
    print(i)

del a[2]
del a[3]

print(type(d), len(d))
for i in d:
    print(i)

i = iter(a)
print(next(i))
print(next(i))
print(next(i))

try:
    print(next(i))
except BaseException as e:
    print(e);

i = iter(a.keys())
print(next(i))
print(next(i))
print(next(i))

i = iter(a.values())
print(next(i))
print(next(i))
print(next(i))

i = iter(a.items())
print(next(i))
print(next(i))
print(next(i))
