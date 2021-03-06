import sys

print(sys.builtins)
sys.builtins["abc"] = 12345
print(abc)
print(sys.builtins)
