// repr.cpp - repr, printing, and buffering

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

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

void Buffer::write (uint8_t const* ptr, size_t len) const {
    for (size_t i = 0; i < len; ++i)
        printf("%c", ptr[i]); // TODO yuck
}

auto Buffer::operator<< (Value v) -> Buffer& {
    if (sep)
        putc(' ');
    sep = true;
    switch (v.tag()) {
        case Value::Nil: print("Nil"); break;
        case Value::Int: print("%d", (int) v); break;
        case Value::Str: print("\"%s\"", (const char*) v); break;
        case Value::Obj: v.obj().repr(*this); break;
    }
    return *this;
}

// formatted output, adapted from JeeH

int Buffer::splitInt (uint32_t val, int base, uint8_t* buf) {
    int i = 0;
    do {
        buf[i++] = val % base;
        val /= base;
    } while (val != 0);
    return i;
}

void Buffer::putFiller (int n, char fill) {
    while (--n >= 0)
        putc(fill);
}

void Buffer::putInt (int val, int base, int width, char fill) {
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

void Buffer::print(const char* fmt, ...) {
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

Value Array::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Bool::repr (Monty::Buffer& buf) const {
    buf.puts(this == &falseObj ? "false" : "true");
    return {};
}

Value BoundMeth::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Buffer::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Bytes::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Callable::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Class::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Context::repr (Monty::Buffer& buf) const {
    return Object::repr(buf);
}

Value Dict::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Fixed::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Function::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Inst::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Iter::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value List::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Lookup::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Module::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value None::repr (Monty::Buffer& buf) const {
    buf.puts("null");
    return {};
}

auto Object::repr (Buffer& buf) const -> Value {
    buf.print("<%s at %p>", type().name, this);
    return {};
}

Value Range::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Set::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Slice::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Str::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Tuple::repr (Monty::Buffer& buf) const {
    return Object::repr(buf); // TODO
}

Value Type::repr (Monty::Buffer& buf) const {
    buf.print("<type %s>", name);
    return {};
}
