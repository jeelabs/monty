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
