class A:
    def __init__(self):
        print("A init")
    def __enter__(self, *args):
        print('enter', args)
        return 123
    def __exit__(self, *args):
        print('exit', args)

print('before')
with A() as x:
    print(x)
print('after')
