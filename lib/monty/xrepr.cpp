// repr.cpp - repr, printing, and buffering

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

extern "C" int printf (const char*, ...);

using namespace Monty;

// non-recursive version for debugging, does not affect the VM state
void Value::dump (const char* msg) const {
    if (msg != 0)
        printf("%s ", msg);
    switch (tag()) {
        case Value::Nil: printf("<N>"); break;
        case Value::Int: printf("<I %d>", (int) *this); break;
        case Value::Str: printf("<S '%s'>", (const char*) *this); break;
        case Value::Obj: printf("<O %s at %p>", obj().type().name, &obj()); break;
    }
    if (msg != 0)
        printf("\n");
}

void Printer::write (uint8_t const* ptr, size_t len) const {
    for (size_t i = 0; i < len; ++i)
        ::printf("%c", ptr[i]); // TODO yuck
}

// formatted output, adapted from JeeH

int Printer::splitInt (uint32_t val, int base, uint8_t* buf) {
    int i = 0;
    do {
        buf[i++] = val % base;
        val /= base;
    } while (val != 0);
    return i;
}

void Printer::putFiller (int n, char fill) {
    while (--n >= 0)
        putc(fill);
}

void Printer::putInt (int val, int base, int width, char fill) {
    uint8_t buf [33];
    int n;
    if (val < 0 && base == 10) {
        n = splitInt(-val, base, buf);
        if (fill != ' ')
            putc('-');
        putFiller(width - n - 1, fill);
        if (fill == ' ')
            putc('-');
    } else {
        n = splitInt(val, base, buf);
        putFiller(width - n, fill);
    }
    while (n > 0) {
        uint8_t b = buf[--n];
        putc("0123456789ABCDEF"[b]);
    }
}

void Printer::printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char const* s;
    while (*fmt) {
        char c = *fmt++;
        if (c == '%') {
            char fill = *fmt == '0' ? '0' : ' ';
            int width = 0, base = 0;
            while (base == 0) {
                c = *fmt++;
                switch (c) {
                    case 'b':
                        base =  2;
                        break;
                    case 'o':
                        base =  8;
                        break;
                    case 'd':
                        base = 10;
                        break;
                    case 'p':
                        fill = '0';
                        width = 8;
                        // fall through
                    case 'x':
                        base = 16;
                        break;
                    case 'c':
                        putFiller(width - 1, fill);
                        c = va_arg(ap, int);
                        // fall through
                    case '%':
                        putc(c);
                        base = 1;
                        break;
                    case 's':
                        s = va_arg(ap, char const*);
                        width -= strlen(s);
                        while (*s)
                            putc(*s++);
                        putFiller(width, fill);
                        // fall through
                    default:
                        if ('0' <= c && c <= '9')
                            width = 10 * width + c - '0';
                        else
                            base = 1; // stop scanning
                }
            }
            if (base > 1) {
                int val = va_arg(ap, int);
                putInt(val, base, width, fill);
            }
        } else
            putc(c);
    }
    va_end(ap);
}

Value Array::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Bool::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value BoundMeth::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Bytes::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Callable::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Class::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Context::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Dict::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Fixed::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Function::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value List::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Lookup::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Module::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value None::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Printer::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Set::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Slice::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Str::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Tuple::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}

Value Type::repr (Monty::Printer& pr) const {
    assert(false); // TODO
    return {};
}
