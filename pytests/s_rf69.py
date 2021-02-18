# The RFM69 module is connected to the Nucleo-L432 as follows:
#
#   | Nucleo | STM32 | RFM69 |
#   |--------|-------|-------|
#   |   D2   | PA12  | NSS   |
#   |   D3   | PB0   | SCK   |
#   |   D4   | PB7   | MOSI  |
#   |   D5   | PB6   | MISO  |

machine.dog(200) # avoid hang in init if the RF69 is not present

evt = machine.ticker(10)

rf69 = machine.rf69("B7,B6,B0,A12", 63, 42, 868)
print(type(rf69))

async def loop():
    buf = array('B', 50);
    for _ in range(20):
        evt.wait()
        evt.clear()
        n = rf69.receive(buf)
        print(machine.ticks(), buf[:n])
    machine.ticker()

sys.tasks.append(loop())
