import machine

spi = machine.spi("B7,B6,B0,A12")
print(type(spi))
spi.enable()
machine.pins.B6 = "D" # pull-down
x = spi.xfer(0x12)
machine.pins.B6 = "U" # pull-up
y = spi.xfer(0x34)
spi.disable()
print(x, y)

try:
    machine.spi("A4,A5,abc,A7")
except Exception as e:
    print(e)
