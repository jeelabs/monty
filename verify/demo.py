print("version",monty.version)
print("tasks",len(monty.tasks))
t=machine.ticks()
while machine.ticks() == t: pass
print("ticks",machine.ticks())

a=[1,2,3,4,5]
print(a[1])
#print(a[1:3])
#print(a[::2])
b=slice(1,4,2)
print(b)
#print(a[b])
#a[2:3] = [11,22,33]
print(a)
