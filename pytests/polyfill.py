str.wow = 111
print("wow", str.wow)

def blah(*args):
    print('blah', args)
    return len(args)

print(blah(1,2,3))

str.whee = blah
print("whee", "abc".whee(1,2,3,4,5))

def even(i):
    return (i & 1) == 0

int.even = even
int.odd = lambda x: not x.even()

print("call:")
for i in range(4):
    print(i, even(i))

print("attr:")
for i in range(4):
    print(i, i.even(), i.odd())
