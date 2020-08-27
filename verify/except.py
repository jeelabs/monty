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
        e = exc("abc")
        raise e
    except:
        print(e, e[0])
