a = 1000000 * 2000000
b = 3000000 * 4000000
c = 5000000 * -6000000

print(a, b, c)
print(True, a<b)
print(True, a>c)
print(False, a==b)
print(a|b, a&b, a^b)
print(a<<2, a>>2)
print(a*b, b*a, a/b, b/a, a%b, b%a)
print(a+b, b+a, a-b, b-a)
print(-a, -b, -c)
print(~a, ~b, ~c)
print(not a, not b, not c)
print(bool(a), bool(b), bool(c))
print(hash(a), hash(b), hash(c))
print(abs(a), abs(-a), abs(c), abs(-c))
print(int("10000000000"), int("20000000000"), int("-30000000000"))

z = a-a
try:
    a = a / z
except ZeroDivisionError as e:
    print(e, e.trace())
try:
    a = a % z
except ZeroDivisionError as e:
    print(e, e.trace())
