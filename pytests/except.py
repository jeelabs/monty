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
        print(e, e.trace())

def boom1():
    print('start 1')
    raise RuntimeError()
    print('oops! 1')

try:
    boom1()
except Exception as e:
    print('bingo 1', e, e.trace())

def boom2():
    print('start 2')
    try:
        raise RuntimeError()
    finally:
        print('final 2')
    print('oops! 2')

try:
    boom2()
except Exception as e:
    print('bingo 2', e, e.trace())

try:
    a = 1 // 0
except BaseException as e:
    print(e, e.trace())

try:
    print(xx)
except BaseException as e:
    print(e, e.trace())

class A: pass
a = A()
try:
    print(a.yy)
except BaseException as e:
    print(e, e.trace())

a = {}
try:
    print(a['zz'])
except BaseException as e:
    print(e, e.trace())

def f(t):
    print(t[1])
    try:
        print(t["a"])
    except BaseException as e:
        print(e, e.trace())

f(b'abc')
f("def")
f([1,2,3])
f((3,4,5))

def g(e):
    try:
        raise(e())
    except RuntimeError as e:
        print('runtime error:', e, e.trace())
    except ArithmeticError as e:
        print('arithmetic error:', e, e.trace())
    except Exception as e:
        print('exception:', e, e.trace())
    except BaseException as e:
        print('base exception:', e, e.trace())
    except ValueError as e:
        print('value error:', e, e.trace())

g(ZeroDivisionError)
g(NotImplementedError)
g(EOFError)
g(BaseException)
g(UnicodeError)

try:
    print("abcde".blah())
except Exception as e:
    print(e, e.trace())

assert 1 == 1

try:
    assert 1 == 0
except Exception as e:
    print(type(e), e, e.trace())

try:
    assert 1 == 0, "this should fail"
except Exception as e:
    print(type(e), e, e.trace())

try:
    raise Exception
except Exception as e:
    print(type(e), e, e.trace())
