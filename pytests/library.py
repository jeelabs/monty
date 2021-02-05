print('11')

print(bytes)

print(bytes.count)

try:
    print(b'abc'.count())
except Exception as e:
    print(e)

print(b'abc'.count(b'def'))

print('22')
