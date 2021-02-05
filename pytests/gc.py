print(sys.gc_avail(), sys.gc_stats())
print(sys.gc_avail(), sys.gc_stats())
a = []
print(sys.gc_avail(), sys.gc_stats())
print(sys.gc_avail(), sys.gc_stats())

def f():
    t0 = machine.ticks()
    for z in range(25):
        for i in range(100):
            l = [i,1,2,3,4,5,6,7,8,9]
    t1 = machine.ticks()
    print(t1-t0, 'ms')

f()
print(sys.gc_avail(), sys.gc_stats())

sys.gc_now()
print(sys.gc_avail(), sys.gc_stats())

del a
del l
del t0
del t1
del z
del i

sys.gc_now()
print(sys.gc_avail(), sys.gc_stats())
