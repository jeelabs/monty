def p(a):
    print(a, a[0], a[1], a[2])

def f(t,n=3):
    a = array(t, n)
    p(a)
    a[1] = 2
    a[2] = 4
    p(a)
    a[0] += -1
    a[2] += -2
    p(a)

f("b")
f("h")
f("i")
f("l")
f("q")

f("B")
f("H")
f("I")
f("L")

f("N",8)
f("T",8)
f("P",8)
