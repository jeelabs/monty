#include <monty.h>

#include <jee.h>
#include "jee-stm32.h"
#include "jee-rf69.h"

#include <cassert>
#include <unistd.h>

//CG2 if dir extend
#define HAS_ARRAY 1
#include <extend.h>

using namespace monty;

#if !HAS_ARRAY
using Array = Bytes; // TODO not really the same thing, but best available?
#endif

//CG: module machine

// machine.pins.B3 = "PL"   : set mode of PB3 to push-pull, low speed
// machine.pins.B3 = 1      : set output pin to 1
// if machine.pins.B3: ...  : get input pin state

struct Pins : Object {
    static Type info;
    auto type () const -> Type const& override { return info; }

    auto attr (Value name, Value&) const -> Value override {
        char const* s = name;
        assert(strlen(s) >= 2);
        jeeh::Pin p (s[0], atoi(s+1));
        return p.read();
    }

    auto setAt (Value arg, Value val) -> Value override {
        assert(arg.isStr() && strlen(arg) >= 2);
        jeeh::Pin p (arg[0], atoi((char const*) arg + 1));
        if (val.isInt())
            p.write(val);
        else {
            assert(val.isStr());
            if (!p.mode((char const*) val))
                return {E::ValueError, "invalid pin mode", val};
        }
        return {};
    }
};

Type Pins::info (Q(0,"<pins>"));

static Pins pins; // there is one static pins object, used via attr access

// spi = machine.spi("A4,A5,A6,A7")
// spi.enable()
// x = spi.transfer(123)
// spi.disable()

struct Spi : Object, jeeh::SpiGpio {
    static Lookup const attrs;
    static Type info;
    auto type () const -> Type const& override { return info; }

    auto xfer (Value v) -> Value {
        assert(v.isInt());
        return transfer(v);
    }

    //CG: wrap Spi enable disable xfer
    auto enable () -> Value { jeeh::SpiGpio::enable(); return {}; }
    auto disable () -> Value { jeeh::SpiGpio::disable(); return {}; }
};

//CG: wrappers Spi

Type Spi::info (Q(0,"<spi>"), &Spi::attrs);

struct RF69 : Object, jeeh::RF69<jeeh::SpiGpio> {
    static Lookup const attrs;
    static Type info;
    auto type () const -> Type const& override { return info; }

    auto recv (ArgVec const& args) -> Value {
        assert(args.size() == 2);
        auto& a = args[1].asType<Array>();
        assert(a.size() >= 4);
        auto r = receive(a.begin()+4, a.size()-4);
        a[0] = rssi;
        a[1] = lna;
        a[2] = afc;
        a[3] = afc >> 8;
        return r+4;
    }

    auto xmit (ArgVec const& args) -> Value {
        assert(args.size() == 2 && args[0].isInt());
        auto& a = args[1].asType<Array>();
        send(args[0], a.begin(), a.size());
        return {};
    }

    auto sleep () -> Value {
        jeeh::RF69<jeeh::SpiGpio>::sleep();
        return {};
    }

    //CG: wrap RF69 recv xmit sleep
};

//CG: wrappers RF69

Type RF69::info (Q(0,"<rf69>"), &RF69::attrs);

//CG1 bind spi
static auto f_spi (ArgVec const& args) -> Value {
    //CG: args arg:s
    auto spi = new Spi;
    auto err = jeeh::Pin::define(arg, &spi->_mosi, 4);
    if (err != nullptr || !spi->isValid())
        return {E::ValueError, "invalid SPI pin", err};
    spi->init();
    return spi;
}

//CG1 bind rf69
static auto f_rf69 (ArgVec const& args) -> Value {
    //CG: args a1:s a2:i a3:i a4:i
    auto rf69 = new RF69;
    auto err = jeeh::Pin::define(a1, &rf69->spi._mosi, 4);
    if (err != nullptr || !rf69->spi.isValid())
        return {E::ValueError, "invalid SPI pin", err};
    rf69->spi.init();
    rf69->init(a2, a3, a4);
    return rf69;
}

static Event tickEvent;
static int ms, tickerId;
static uint32_t start, last;

static auto msNow () -> Value {
    uint32_t t = ticks;
    static uint32_t begin;
    if (begin == 0)
        begin = t;
    return t - begin; // make all runs start out the same way
}

//CG1 bind ticker
static auto f_ticker (ArgVec const& args) -> Value {
    //CG: args ? arg:i
    if (arg > 0) {
        assert(args.size() == 1 && args[0].isInt());
        ms = arg;
        start = msNow(); // set first timeout relative to now
        last = 0;
        tickerId = tickEvent.regHandler();
        assert(tickerId > 0);

        VTableRam().systick = []() {
            ++ticks;
            uint32_t t = msNow(); // TODO messy
            if (ms > 0 && (t - start) / ms != last) {
                last = (t - start) / ms;
                Stacklet::setPending(tickerId);
            }
        };
    } else {
        VTableRam().systick = []() {
            ++ticks;
        };

        tickEvent.deregHandler();
        assert(tickerId > 0);
    }
    return tickEvent;
}

//CG1 bind ticks
static auto f_ticks (ArgVec const& args) -> Value {
    //CG: args
    return msNow();
}

//CG1 bind dog
static auto f_dog (ArgVec const& args) -> Value {
    // TODO optional args
    int count = 4095;
    if (args.size() > 0 && args[0].isInt())
        count = args[0];

    static Iwdg dog;
    dog.reload(count);
    return {};
}

//CG1 bind kick
static auto f_kick (ArgVec const& args) -> Value {
    //CG: args
    Iwdg::kick();
    return {};
}

//CG1 wrappers
static Lookup::Item const machine_map [] = {
    { Q(0,"pins"), pins },
};

//CG: module-end
