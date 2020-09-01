print(sys.gcstats())
print(sys.gcstats())
a = []
print(sys.gcstats())
print(sys.gcstats())

t0 = machine.ticks()
for z in range(25):
    for i in range(100):
        l = [i,1,2,3,4,5,6,7,8,9]
t1 = machine.ticks()

print(t1-t0, 'ms')
print(sys.gcstats())
l.clear()
print(sys.gcstats())
