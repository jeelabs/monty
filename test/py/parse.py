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

