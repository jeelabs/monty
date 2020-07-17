print(123)
print(str.count)
print(type(str.count))

class A:
    def f(*a): print(a)

print(124)
print(A.f)
print(type(A.f))
A.f(1,2,4)
g = A.f
g(1,2,4)

print(125)
a=A()
print(a.f)
print(type(a.f))
a.f(1,2,5)
g = a.f
g(1,2,5)

# Monty:
#   main qstr #166 1276b
#   123
#   <Obj <method> at 0x10cd26528>
#   "<method>"
#   124
#   <Obj <callargs> at 0x10cd21950>
#   "<callargs>"
#   (<class A>,1,2,4)
#   (1,2,4)
#   125
#   <Obj <bound-meth> at 0x10cd21930>
#   "<bound-meth>"
#   (<A object at 0x10cd21bd0>,1,2,5)
#   (<A object at 0x10cd21bd0>,1,2,5)
#   done

# µPy:
#   123
#   <function>
#   <class 'function'>
#   124
#   <function f at 0x7fcbe2d061a0>
#   <class 'function'>
#   (1, 2, 4)
#   (1, 2, 4)
#   125
#   <bound_method 7fcbe2d06280 <A object at 7fcbe2d06180>.<function f at 0x7fcbe2d061a0>>
#   <class 'bound_method'>
#   (<A object at 7fcbe2d06180>, 1, 2, 5)
#   (<A object at 7fcbe2d06180>, 1, 2, 5)

# CPy:
#   123
#   <method 'count' of 'str' objects>
#   <class 'method_descriptor'>
#   124
#   <function A.f at 0x1102deee0>
#   <class 'function'>
#   (1, 2, 4)
#   (1, 2, 4)
#   125
#   <bound method A.f of <__main__.A object at 0x1102a1130>>
#   <class 'method'>
#   (<__main__.A object at 0x1102a1130>, 1, 2, 5)
#   (<__main__.A object at 0x1102a1130>, 1, 2, 5)
