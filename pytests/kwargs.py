def f(aa, bb=22, cc=33):
    print("f",aa,bb,cc)

f(111)
f(111,bb="<b>")
f(111,cc="<c>")
f(111,cc="<c>",bb="<b>")

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
