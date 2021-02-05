def ff(x):
    return lambda: x+1
dd = ff(123)
print(type(dd))
print(dd())

def one(n):
    def a():
        print(n)
    n += 7
    return a

print("a")
a = one(11)
a()

def two():
    def b():
        print(m)
    return b

b = two()
print(type(b))
m = 987
b()

def three(n):
    def c():
        def d():
            print(n)
        return d
    return c()

c = three(777)
c()
