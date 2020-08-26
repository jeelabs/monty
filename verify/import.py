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

print(sys.modules)
