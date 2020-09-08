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

def f(t):
    print(t[1])
    try:
        print(t["a"])
    except BaseException as e:
        print(e)

f(b'abc')
f("def")
f([1,2,3])
f((3,4,5))

def g(e):
    try:
        raise(e())
    except RuntimeError as e:
        print('runtime error:', e)
    except ArithmeticError as e:
        print('arithmetic error:', e)
    except Exception as e:
        print('exception:', e)
    except BaseException as e:
        print('base exception:', e)
    except ValueError as e:
        print('value error:', e)

g(ZeroDivisionError)
g(NotImplementedError)
g(EOFError)
g(BaseException)
g(UnicodeError)
