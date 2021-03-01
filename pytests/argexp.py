def f(*a):
    print("f", a)


f()
f(1)
f(1,2)
f(1,2,3,*range(5, 10, 2))
f(1,2,3,4,*(5+i*i*i for i in range(4)))

def l(a,b):
    print("l",a,b)

#l(*[11,22])
#l(**{"a":111,"b":222})
