class A:
    def __init__(self,x):
        print("A init", type(self), x)
    def f(self):
        print("A:f", type(self))
        return 111

print("an", A.__name__)
print("ab", A.__bases__)
a = A(123)
a.f()
#print(a.__dict__)

class B(A):
    def __init__(self,x):
        print("B init", type(self), x)
        super().__init__(-x)

print("bn", B.__name__)
print("bb", B.__bases__)
b = B(456)
b.f()

class C(B):
    def f(self):
        print("C:f", type(self))
        print(super().f())
        return 222

print("cn", C.__name__)
print("cb", C.__bases__)
c = C(789)
print(c.f())

class D: pass

print("dn", D.__name__)
print("db", D.__bases__)

d = D()
