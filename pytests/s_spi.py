spi = machine.spi("A4,A5,A6,A7")
print(type(spi))
spi.enable()
x = spi.transfer(0x12)
spi.disable()
print(x)

try:
    machine.spi("A4,A5,abc,A7")
except Exception as e:
    print(e)
