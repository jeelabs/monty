i = 123
j = 0
k = -234
l = 10000000000

print(111)
print(-123,-i)
print(123,+i)
print(-124,~i)
print(True,bool(i))
print(False,not i)
print(123,int(i))
print(123,abs(i))
print(123,hash(i))

print(112)
print(0,-j)
print(0,+j)
print(-1,~j)
print(False,bool(j))
print(True,not j)
print(0,int(j))
print(0,abs(j))
print(0,hash(j))

print(113)
print(234,-k)
print(-234,+k)
print(233,~k)
print(True,bool(k))
print(False,not k)
print(-234,int(k))
print(234,abs(k))
print(-234,hash(k))

print(114)
print(bool(b''))
print(bool(b'abc'))
print(bool(b'abcdefghijklmnopqrstuvwxyz'))
print(hash(b''))
print(hash(b'abc'))
print(hash(b'abcdefghijklmnopqrstuvwxyz'))

print(115)
print(bool(''))
print(bool('abc'))
print(bool('abcdefghijklmnopqrstuvwxyz'))
print(hash(''))
print(hash('abc'))
print(hash('abcdefghijklmnopqrstuvwxyz'))

print(116)
print(False,not True)
print(True,not False)
print(1,int(True))
print(0,int(False))
print(True,bool(True))
print(False,bool(False))
print(1,hash(True))
print(0,hash(False))

print(117)
a = -1073741823
b = -1073741824
c = -1073741825
print(1073741823,-a)
print(1073741824,-b)
print(1073741825,-c)

print(118)
a = 1
b = 1000000
print(536870912,a<<29)
print(1073741824,a<<30)
print(2147483648,a<<31)
print(2147483648,a<<32)
print(4611686018427387904,a<<62)
print(-9223372036854775808,a<<63)
print(1000000000000,b*b)
