def f(aa, bb=22, cc=33):
    print("f",aa,bb,cc)

f(111)
f(111,bb="<b>")
f(111,cc="<c>")
f(111,cc="<c>",bb="<b>")

try:
    f()
except Exception as e:
    print(e, e.trace())

def g(aa, bb, cc=33, dd=44):
    print("g",aa,bb,cc,dd)

g(1,2)
g(1,2,dd="<d>")
g(1,2,3)
g(1,2,3,4)

def h(aa, bb, *args):
    print("h",aa,bb,args)

h(1,2)
h(1,2,3)
h(1,2,3,4)

try:
    h(1)
except Exception as e:
    print(e, e.trace())

def k(a,b,*,c=33,d=44):
    print("k",a,b,c,d)

k(1,2)
k(1,2,c="<c>")

try:
    k(1,2,3)
except Exception as e:
    print(e, e.trace())

def l(a,b,**kw):
    print("l",a,b,kw)

l(1,2,c="<c>",d="<d>")
l(1,2,d="<d>",c="<c>")

def m(a,b,*c,d,**kw):
    print("m",a,b,c,d,kw)

m(1,2,3,4,d="<d>",e="<e>")
m(5,6,7,8,e="<e>",d="<d>")

try:
    m(1,2,3,4,e="<e>")
except Exception as e:
    print(e, e.trace())
