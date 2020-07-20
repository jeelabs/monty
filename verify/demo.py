print("version",monty.version)
print("tasks",len(monty.tasks))
t=machine.ticks()
while machine.ticks() == t: pass
print("ticks",machine.ticks())
