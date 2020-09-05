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
