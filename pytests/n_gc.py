import sys, machine

print(sys.gcmax(), sys.gcstats())
print(sys.gcmax(), sys.gcstats())
a = []
print(sys.gcmax(), sys.gcstats())
print(sys.gcmax(), sys.gcstats())

def f():
    t0 = machine.ticks()
    for z in range(25):
        for i in range(100):
            l = [i,1,2,3,4,5,6,7,8,9]
    t1 = machine.ticks()
    print(t1-t0, 'ms')

f()
print(sys.gcmax(), sys.gcstats())

sys.gc()
print(sys.gcmax(), sys.gcstats())

del a
del l
del t0
del t1
del z
del i

sys.gc()
print(sys.gcmax(), sys.gcstats())
