print(123, id(123))
print("id", id("id"))
#print("abcdefghijklmnopqrstuvwxyz", id("abcdefghijklmnopqrstuvwxyz"))

def f(t):
    print(t, dir(t))

f([])
f(list)
f(list.append)
f({1:2,3:4})
f({1,2,3})

class A:
    v = 1
    def m(): pass

f(A)
f(A())

import sys
f(sys)
sys.builtins["abc"] = 123
f(sys.builtins)

import hello
f(sys.modules)
