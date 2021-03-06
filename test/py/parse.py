print(1, argtest("",[],1,2,3,"abc","def",6,foo=123,bar=[],baz={1,2}))
print(2, argtest("",[],1,2,3,"abc","def",  foo=123,bar=[],baz={1,2}))
print(3, argtest("",[],1,2,3,"abc",        foo=123,bar=[],baz={1,2}))
print(4, argtest("",[],1,2,3,              foo=123,bar=[],baz={1,2}))
print(5, argtest("",[],1,2,                foo=123,bar=[],baz={1,2}))

try:
    print(argtest("",[],1,                  foo=123,bar=[],baz={1,2}))
except Exception as e:
    print(e, e.trace())

print(6, argtest("",[],1,2,3,"abc","def",6,foo=123,bar=[],baz={1,2}))
print(7, argtest("",[],1,2,3,"abc","def",6,        bar=[],baz={1,2}))
print(8, argtest("",[],1,2,3,"abc","def",6,               baz={1,2}))
print(9, argtest("",[],1,2,3,"abc","def",6,                        ))

try:
    print(argtest("",[],1,2,huh=123))
except Exception as e:
    print(e, e.trace())

print(10, argtest("",[],1,2,3))
print(11, argtest("",[],1,2,3,"abc"))
print(12, argtest("",[],1,2,3,"abc","def"))
print(13, argtest("",[],1,2,3,"abc","def",6))
print(14, argtest("",[],1,2,3,"abc","def",6,7))
print(15, argtest("",[],1,2,3,"abc","def",6,7,8))

print(20, argtest(9,[],1,2,3))
print(21, argtest(9,[],1,2,3,"abc"))
print(22, argtest(9,[],1,2,3,"abc","def"))
print(23, argtest(9,[],1,2,3,"abc","def",6))
print(24, argtest(9,[],1,2,3,"abc","def",6,7))
print(25, argtest(9,[],1,2,3,"abc","def",6,7,8))

import machine
try:
    machine.ticks(1)
except Exception as e:
    print(e, e.trace())
