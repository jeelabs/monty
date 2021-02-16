spi = machine.spi("A4,A5,A6,A7")
print(type(spi))
spi.enable()
machine.pins.A5 = "D" # pull-down
x = spi.transfer(0x12)
machine.pins.A5 = "U" # pull-up
y = spi.transfer(0x34)
spi.disable()
print(x, y)

try:
    machine.spi("A4,A5,abc,A7")
except Exception as e:
    print(e)
