class A: pass
class B(A):
    def f(self):
        print(super)
        print(super())

print(B)
print(B.f)

b = B()
b.f()

print(b)
print(b.f)

print(1234567890123456789)
