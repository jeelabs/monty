print('11')

print(bytes)

try:
    print(bytes.count)
except Exception as e:
    print(e)

try:
    print(b'abc'.count())
except Exception as e:
    print(e)

#print(b'abc'.count(b'def'))

print('22')
