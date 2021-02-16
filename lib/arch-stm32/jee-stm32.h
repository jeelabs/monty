#include <jee.h>

namespace jeeh {
    struct Pin {
        uint32_t _base =0;
        uint8_t _port =0;
        uint8_t _pin =0;
        
        Pin () {}
        Pin (char port, int pin) : _base (Periph::gpio+0x400*(port-'A')),
                                    _port (port-'A'), _pin (pin) {}

        auto isValid () const -> bool {
            return _base != 0 && _port < 16 && _pin < 16;
        }

#if STM32F1
        enum { crl=0x00, crh=0x04, idr=0x08, odr=0x0C, bsrr=0x10, brr=0x14 };

        void mode (int mval, int /*ignored*/ =0) const {
            // enable GPIOx and AFIO clocks
            MMIO32(Periph::rcc+0x18) |= (1 << _port) | (1<<0);

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

        void mode (int mval, int alt =0) const {
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

        auto read () const -> int {
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

        // mode string: [AFDUPO][LNHV][<n>][,]
        auto mode (char const* desc) const -> bool {
            int a = 0, m = 0;
            for (auto s = desc; *s != 0; ++s)
                switch (*s) {
                    case 'A': m = (int) Pinmode::in_analog; break;
                    case 'F': m = (int) Pinmode::in_float; break;
                    case 'D': m = (int) Pinmode::in_pulldown; break;
                    case 'U': m = (int) Pinmode::in_pullup; break;

                    case 'P': m = (int) Pinmode::out; break; // push-pull
                    case 'O': m = (int) Pinmode::out_od; break;

                    case 'L': m = m & 0x1F; break;          // low speed
                    case 'N': break;                        // normal speed
                    case 'H': m = (m & 0x1F) | 0x40; break; // high speed
                    case 'V': m = m | 0x60; break;          // very high speed

                    default:  if (*s < '0' || *s > '9' || a > 1)
                                  return false;
                              m |= 0x10; // alt mode
                              a = 10 * a + *s - '0';
                    case ',': break; // valid as terminator
                }
            mode(m, a);
            return true;
        }

        // pin definition string: [A-P][<n>]:[<mode>][,]
        static auto define (char const* desc) -> Pin {
            if (desc != nullptr) {
                uint8_t port = *desc++;
                uint8_t pin = 0;
                while ('0' <= *desc && *desc <= '9')
                    pin = 10 * pin + *desc++ - '0';
                Pin p (port, pin);
                if (p.isValid() && (*desc++ != ':' || p.mode(desc)))
                    return p;
            }
            return {};
        }

        // define multiple pins, returns ptr to error or nullptr
        static auto define (char const* d, Pin* v, int n) -> char const* {
            while (--n >= 0) {
                *v = define(d);
                if (!v->isValid())
                    break;
                ++v;
                auto p = strchr(d, ',');
                if (n == 0)
                    return *d != 0 ? p : nullptr; // point to comma if more
                if (p == nullptr)
                    break;
                d = p+1;
            }
            return d;
        }
    };

    struct SpiGpio {
        Pin _mosi, _miso, _sclk, _nsel;
        uint8_t _cpol =0;

        SpiGpio () {}
        SpiGpio (Pin mo, Pin mi, Pin ck, Pin ss, int cp =0)
            : _mosi (mo), _miso (mi), _sclk (ck), _nsel (ss), _cpol (cp) {}

        void init () {
            _nsel.mode((int) Pinmode::out); disable();
            _sclk.mode((int) Pinmode::out); _sclk = _cpol;
            _miso.mode((int) Pinmode::in_float);
            _mosi.mode((int) Pinmode::out);
        }

        auto isValid () const -> bool {
            return _mosi.isValid() && _nsel.isValid(); // check first & last
        }

        void enable () const { _nsel = 0; }
        void disable () const { _nsel = 1; }

        auto transfer (uint8_t v) const -> uint8_t {
            for (int i = 0; i < 8; ++i) {
                _mosi = v & 0x80;
                v <<= 1;
                _sclk = !_cpol;
                v |= _miso;
                _sclk = _cpol;
            }
            return v;
        }

        void transfer (uint8_t* buf, int len) const {
            enable();
            for (int i = 0; i < len; ++i)
                buf[i] = transfer(buf[i]);
            disable();
        }

        void send (uint8_t const* buf, int len) const {
            enable();
            for (int i = 0; i < len; ++i)
                transfer(buf[i]);
            disable();
        }

        void receive (uint8_t* buf, int len) const {
            enable();
            for (int i = 0; i < len; ++i)
                buf[i] = transfer(0);
            disable();
        }
    };
}
