for exc in [BaseException,
            Exception,
            StopIteration,
            ArithmeticError,
            ZeroDivisionError,
            AssertionError,
            AttributeError,
            EOFError,
            ImportError,
            LookupError,
            IndexError,
            KeyError,
            MemoryError,
            NameError,
            OSError,
            RuntimeError,
            NotImplementedError,
            TypeError,
            ValueError,
            UnicodeError]:
    try:
        raise exc("abc",1,2,3)
    except BaseException as e:
        print(e)

def boom1():
    print('start 1')
    raise RuntimeError()
    print('oops! 1')

try:
    boom1()
except:
    print('bingo 1')

def boom2():
    print('start 2')
    try:
        raise RuntimeError()
    finally:
        print('final 2')
    print('oops! 2')

try:
    boom2()
except:
    print('bingo 2')

try:
    a = 1 // 0
except BaseException as e:
    print(e)

try:
    print(xx)
except BaseException as e:
    print(e)

class A: pass
a = A()
try:
    print(a.yy)
except BaseException as e:
    print(e)

a = {}
try:
    print(a['zz'])
except BaseException as e:
    print(e)
