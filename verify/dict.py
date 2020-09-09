a = {1:2,3:4,5:6}
print(a)

for i in a:
    print(i)

print(111)
d = a.keys()
print(type(d), len(d))
for i in d:
    print(i)

print(112)
d = a.values()
print(type(d), len(d))
for i in d:
    print(i)

print(113)
d = a.items()
print(type(d), len(d))
for i in d:
    print(i)

a[3] = 44;
a[7] = 88;

print(114)
print(type(d), len(d))
for i in d:
    print(i)

del a[2]
del a[3]

print(115)
print(type(d), len(d))
for i in d:
    print(i)

print(116)
i = iter(a)
print(next(i))
print(next(i))
print(next(i))

try:
    print(next(i))
except BaseException as e:
    print(e);

print(117)
i = iter(a.keys())
print(next(i))
print(next(i))
print(next(i))

print(118)
i = iter(a.values())
print(next(i))
print(next(i))
print(next(i))

print(119)
i = iter(a.items())
print(next(i))
print(next(i))
print(next(i))

[1,2,3].clear() # wrong place
