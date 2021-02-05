class A:
    def f(self,*args):
        print('f:', type(self), args)

print(A)
a = A()
print(type(a))

a.f(3,4,5)
g = a.f
print(type(g))
a.f(6,7,8)
g(6,7,8)

A.f(11,22,33)
