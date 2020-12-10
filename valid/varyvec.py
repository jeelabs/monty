a = array('V', 3)
print(a)
a[0] = 'one'
a[1] = 'two'
a[2] = 'three'
print(a)
for i in range(len(a)): print(a[i])
del a[1]
print(a)
for i in range(len(a)): print(a[i])

b = array('v', 3)
print(b)
b[0] = b'one'
b[1] = b'two'
b[2] = b'three'
print(b)
for i in range(len(b)): print(b[i])
del b[1]
print(b)
for i in range(len(b)): print(b[i])
