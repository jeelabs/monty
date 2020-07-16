def f(*a):
    print('f',123, len(a))
    for i in a:
        print('\t', i)

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

if False:
    h(1,2)
    h(2,3,4)
    h(3,4,5,6)
    h(4,5,6,7,8)
    h(5,6,7,8,9,10)

# current code is wrong, no idea what causes the pair-wise flips ...
#   h 1 2 13 14 11 12
#   h 2 3 4 14 11 12
#   h 3 4 5 6 11 12
#   h 4 5 6 7 8 12
#   h 5 6 7 8 9 10
