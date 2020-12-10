print(123)
import hello
print(456)
print(type(hello))
print(hello.__name__)
print(sys.modules)

from args import f
f(9,8,7)

from args import g
g(987)

try:
    print(h)
except Exception as e:
    print(e)

print(sys.modules)

del f
del g
try:
    print(f)
except Exception as e:
    print(e)

from args import *
f(99,88,77)
g(998877)
h(99,88,77,66)
