#include <jee.h>

namespace jeeh {
    struct Pin {
        uint32_t _base;
        uint8_t _port;
        uint8_t _pin;
        
        Pin (char port, int pin) : _base (Periph::gpio+0x400*(port-'A')),
                                    _port (port-'A'), _pin (pin) {}
#if STM32F1
        enum { crl=0x00, crh=0x04, idr=0x08, odr=0x0C, bsrr=0x10, brr=0x14 };

        void mode (Pinmode m) {
            // enable GPIOx and AFIO clocks
            MMIO32(Periph::rcc+0x18) |= (1 << _port) | (1<<0);

            auto mval = static_cast<int>(m);
            if (mval == 0b1000 || mval == 0b1100) {
                uint16_t mask = 1 << pin;
                MMIO32(_base+bsrr) = mval & 0b0100 ? mask : mask << 16;
                mval = 0b1000;
            }

            uint32_t cr = pin & 8 ? crh : crl;
            int shift = 4 * (pin & 7);
            MMIO32(_base+cr) = (MMIO32(_base+cr) & ~(0xF << shift))
                                                 | (mval << shift);
        }
#else
        enum { moder=0x00, typer=0x04, ospeedr=0x08, pupdr=0x0C, idr=0x10,
                odr=0x14, bsrr=0x18, afrl=0x20, afrh=0x24, brr=0x28 };

        void mode (Pinmode m, int alt =0) const {
            // enable GPIOx clock
#if STM32F3
            Periph::bitSet(Periph::rcc+0x14, _port);
#elif STM32F4 | STM32F7
            Periph::bitSet(Periph::rcc+0x30, _port);
#elif STM32G0
            Periph::bitSet(Periph::rcc+0x34, _port);
#elif STM32H7
            Periph::bitSet(Periph::rcc+0xE0, _port);
#elif STM32L0
            Periph::bitSet(Periph::rcc+0x2C, _port);
#elif STM32L4
            Periph::bitSet(Periph::rcc+0x4C, _port);
#endif
            auto mval = static_cast<int>(m);
            MMIO32(_base+moder) = (MMIO32(_base+moder) & ~(3 << 2*_pin))
                                                | ((mval>>3) << 2*_pin);
            MMIO32(_base+typer) = (MMIO32(_base+typer) & ~(1 << _pin))
                                            | (((mval>>2)&1) << _pin);
            MMIO32(_base+pupdr) = (MMIO32(_base+pupdr) & ~(3 << 2*_pin))
                                                 | ((mval&3) << 2*_pin);
            MMIO32(_base+ospeedr) = (MMIO32(_base+ospeedr) & ~(3 << 2*_pin))
                                                         | (0b11 << 2*_pin);
            uint32_t afr = _pin & 8 ? afrh : afrl;
            int shift = 4 * (_pin & 7);
            MMIO32(_base+afr) = (MMIO32(_base+afr) & ~(0xF << shift))
                                                    | (alt << shift);
        }
#endif

        int read () const {
            return (MMIO32(_base+idr) >> _pin) & 1;
        }

        void write (int v) const {
            auto mask = 1 << _pin;
            MMIO32(_base+bsrr) = v ? mask : mask << 16;
        }

        // shorthand
        void toggle () const { write(read() ^ 1); }
        operator int () const { return read(); }
        void operator= (int v) const { write(v); }
    };
}
