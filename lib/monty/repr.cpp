// repr.cpp - repr, printing, and buffering

#include "monty.h"
#include <cassert>
#include <cstdarg>

using namespace monty;

// non-recursive version for debugging, does not affect the VM state
void Value::dump (char const* msg) const {
    if (msg != 0)
        printf("%s ", msg);
    switch (tag()) {
        case Value::Nil: printf("<N>"); break;
        case Value::Int: printf("<I %d>", (int) *this); break;
        case Value::Str: printf("<S \"%s\">", (char const*) *this); break;
        case Value::Obj: printf("<O %s at %p>",
                                 (char const*) obj().type()._name, &obj());
                         break;
    }
    if (msg != 0)
        printf("\n");
}

static void putcEsc (Buffer& buf, char const* fmt, uint8_t ch) {
    buf.putc('\\');
    switch (ch) {
        case '\t': buf.putc('t'); break;
        case '\n': buf.putc('n'); break;
        case '\r': buf.putc('r'); break;
        default:   buf.print(fmt, ch); break;
    }
}

static void putsEsc (Buffer& buf, Value s) {
    buf.putc('"');
    for (auto p = (uint8_t const*) (char const*) s; *p != 0; ++p) {
        if (*p == '\\' || *p == '"')
            buf.putc('\\');
        if (*p >= ' ')
            buf.putc(*p);
        else
            putcEsc(buf, "u%04x", *p);
    }
    buf.putc('"');
}

Buffer::~Buffer () {
    for (uint32_t i = 0; i < _fill; ++i)
        printf("%c", begin()[i]); // TODO yuck
}

void Buffer::write (uint8_t const* ptr, uint32_t len) {
    if (_fill + len > cap())
        adj(_fill + len + 50);
    memcpy(end(), ptr, len);
    _fill += len;
}

auto Buffer::operator<< (Value v) -> Buffer& {
    switch (v.tag()) {
        case Value::Nil: print("_"); break;
        case Value::Int: print("%d", (int) v); break;
        case Value::Str: putsEsc(*this, v); break;
        case Value::Obj: v.obj().repr(*this); break;
    }
    return *this;
}

// formatted output, adapted from JeeH

auto Buffer::splitInt (uint32_t val, int base, uint8_t* buf) -> int {
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

void Buffer::print(char const* fmt, ...) {
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

auto Bool::repr (Buffer& buf) const -> Value {
    buf << (this == &falseObj ? "false" : "true");
    return {};
}

auto Bytes::repr (Buffer& buf) const -> Value {
    buf << '\'';
    for (auto b : *this) {
        if (b == '\\' || b == '\'')
            buf << '\\';
        if (b >= ' ')
            buf.putc(b);
        else
            putcEsc(buf, "x%02x", b);
    }
    buf << '\'';
    return {};
}

auto Class::repr (Buffer& buf) const -> Value {
    buf.print("<class %s>", (char const*) at("__name__"));
    return {};
}

auto Closure::repr (Buffer& buf) const -> Value {
    return Object::repr(buf); // don't print as a list
}

auto Dict::repr (Buffer& buf) const -> Value {
    buf << '{';
    for (uint32_t i = 0; i < _fill; ++i) {
        if (i > 0)
            buf << ',';
        buf << (*this)[i] << ':' << (*this)[_fill+i];
    }
    buf << '}';
    return {};
}

auto Exception::repr (Buffer& buf) const -> Value {
    buf.puts(bases._items[(int) extra().code].k);
    return Tuple::repr(buf);
}

auto Inst::repr (Buffer& buf) const -> Value {
    buf.print("<%s object at %p>", (char const*) type()._name, this);
    return {};
}

auto Int::repr (Buffer& buf) const -> Value {
    uint64_t val = _i64;
    if (_i64 < 0) {
        buf.putc('-');
        val = -_i64;
    }

    // need to print in pieces which fit into a std int
    int v1 = val / 1000000000000;
    int v2 = (val / 1000000) % 1000000;
    int v3 = val % 1000000;

    if (v1 > 0)
        buf.print("%d%06d%06d", v1, v2, v3);
    else if (v2 > 0)
        buf.print("%d%06d", v2, v3);
    else
        buf.print("%d", v3);
    return {};
}

auto List::repr (Buffer& buf) const -> Value {
    buf << '[';
    for (uint32_t i = 0; i < _fill; ++i) {
        if (i > 0)
            buf << ',';
        buf << (*this)[i];
    }
    buf << ']';
    return {};
}

auto Module::repr (Buffer& buf) const -> Value {
    buf.print("<module '%s'>", (char const*) _name);
    return {};
}

auto None::repr (Buffer& buf) const -> Value {
    buf << "null";
    return {};
}

auto Object::repr (Buffer& buf) const -> Value {
    buf.print("<%s at %p>", (char const*) type()._name, this);
    return {};
}

auto Range::repr (Buffer& buf) const -> Value {
    buf.print("range(%d,%d,%d)", _from, _to, _by);
    return {};
}

auto Set::repr (Buffer& buf) const -> Value {
    buf << '{';
    for (uint32_t i = 0; i < _fill; ++i) {
        if (i > 0)
            buf << ',';
        buf << (*this)[i];
    }
    buf << '}';
    return {};
}

auto Slice::repr (Buffer& buf) const -> Value {
    buf << "slice(" << _off << ',' << _num << ',' << _step << ')';
    return {};
}

auto Str::repr (Buffer& buf) const -> Value {
    putsEsc(buf, (char const*) begin());
    return {};
}

auto Super::repr (Buffer& buf) const -> Value {
    buf << "<super: ...>";
    return {};
}

auto Tuple::repr (Buffer& buf) const -> Value {
    buf << '(';
    for (uint32_t i = 0; i < _fill; ++i) {
        if (i > 0)
            buf << ',';
        buf << (*this)[i];
    }
    buf << ')';
    return {};
}

auto Type::repr (Buffer& buf) const -> Value {
    buf.print("<type %s>", (char const*) _name);
    return {};
}

auto Event::repr (Buffer& buf) const -> Value {
    return Object::repr(buf); // don't print as a list
}

auto Stacklet::repr (Buffer& buf) const -> Value {
    return Object::repr(buf); // don't print as a list
}

auto Buffer::repr (Buffer& buf) const -> Value {
    return Object::repr(buf); // don't print as bytes
}
