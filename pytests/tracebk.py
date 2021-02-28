import sys

try:
    sys.print_exception
except:
    sys.print_exception = print

try:
    raise Exception(123)
except Exception as e:
    print(type(e), e)
    sys.print_exception(e)

def f(x):
    raise Exception(x)

try:
    f(456)
except Exception as e:
    print(type(e), e)
    sys.print_exception(e)

def g(x):
    f(x)

try:
    g(789)
except Exception as e:
    print(type(e), e)
    sys.print_exception(e)
