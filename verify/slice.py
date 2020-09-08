s = slice(1,2,3)
print(s)

s = slice(None)
print(s)

s = slice(4)
print(s)

s = slice(None,5)
print(s)

s = slice(None,None,6)
print(s)

print(b'abcde'[1:4])
print("ABCDE"[1:4])
print((1,2,3,4,5)[1:4])
print([6,7,8,9,0][1:4])

print([1,2,3,4,5][::-1])
print([1,2,3,4,5][::-2])

for i in range(-2,-5,-1):
    print(i)
print(b'fghijkl'[-2:-5:-1])

a = array('i', 5);
for i in range(5):
    a[i] = 2 * i + 1
print(a)
print(a[1:4])

try:
    [1,2,3][::2] = [11,22]
except BaseException as e:
    print(e)

l = [1,2,3,4,5]
l[1:4] = [11,22,33,44,55]
print(l)

a[1:4] = [-1,-2,-3,-4,-5]
print(a)
for i in a:
    print(i)

l = [1,2,3,4,5,6]
l[2:4] = "01234"[::-2]
print(l)

try:
    a[1,2:3,4:5:6]
except Exception as e:
    print(e)
