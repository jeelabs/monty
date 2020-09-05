for exc in [BaseException,
            Exception,
            StopIteration,
            ArithmeticError,
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
    raise RuntimeError
    print('wrong 1')

try:
    boom1()
except:
    print('gotcha 1')

def boom2():
    print('start 2')
    try:
        raise RuntimeError()
    finally:
        print('finally 2')
    print('wrong boom2')

try:
    boom2()
except:
    print('gotcha 2')
