def f(*a):
    print('f',123, a)

f()
f(1)
f(2,3)
f(3,4,5)

def g(a,b=9):
    print('g',a,b)

g(1)
g(2,3)

def h(a,b,p=11,q=12,r=13,s=14):
    print('h',a,b,p,q,r,s)

h(1,2)
h(2,3,4)
h(3,4,5,6)
h(4,5,6,7,8)
h(5,6,7,8,9,10)
