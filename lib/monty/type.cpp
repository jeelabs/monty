// type.cpp - collection types and type system

#include "monty.h"
#include <cassert>
#include <cstdarg>

using namespace monty;

Tuple const Tuple::emptyObj;
Value const monty::Empty {Tuple::emptyObj};

Type Inst::info (Q(168,"<instance>"));

Lookup const Bytes::attrs;
Lookup const Class::attrs;
Lookup const Inst::attrs;
Lookup const Set::attrs;
Lookup const Str::attrs;
Lookup const Super::attrs;
Lookup const Tuple::attrs;
Lookup const Type::attrs;

void monty::markVec (Vector const& vec) {
    for (auto e : vec)
        e.marker();
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
        case Value::Obj: v->repr(*this); break;
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

auto Buffer::repr (Buffer& buf) const -> Value {
    return Object::repr(buf); // don't print as bytes
}

Bytes::Bytes (void const* ptr, uint32_t len) {
    insert(0, len);
    memcpy(begin(), ptr, len);
}

auto Bytes::unop (UnOp op) const -> Value {
    switch (op) {
        case UnOp::Boln: return Value::asBool(size());
        case UnOp::Hash: return Q::hash(begin(), size());
        default:         break;
    }
    return Object::unop(op);
}

auto Bytes::binop (BinOp op, Value rhs) const -> Value {
    auto& val = rhs.asType<Bytes>();
    assert(size() == val.size());
    switch (op) {
        case BinOp::Equal:
            return Value::asBool(size() == val.size() &&
                                    memcmp(begin(), val.begin(), size()) == 0);
        default:
            break;
    }
    return Object::binop(op, rhs);
}

auto Bytes::getAt (Value k) const -> Value {
    if (!k.isInt())
        return sliceGetter(k);
    return (*this)[k];
}

auto Bytes::copy (Range const& r) const -> Value {
    auto n = r.len();
    auto v = new Bytes;
    v->insert(0, n);
    for (uint32_t i = 0; i < n; ++i)
        (*v)[i] = (*this)[r.getAt(i)];
    return v;
}

auto Bytes::create (ArgVec const& args, Type const*) -> Value {
    assert(args._num == 1);
    Value v = args[0];
    if (v.isInt()) {
        auto o = new Bytes ();
        o->insert(0, v);
        return o;
    }
    const void* p = 0;
    uint32_t n = 0;
    if (v.isStr()) {
        p = (char const*) v;
        n = strlen((char const*) p);
    } else {
        auto ps = v.ifType<Str>();
        auto pb = v.ifType<Bytes>();
        if (ps != 0) {
            p = (char const*) *ps;
            n = strlen((char const*) p); // TODO
        } else if (pb != 0) {
            p = pb->begin();
            n = pb->size();
        } else
            assert(false); // TODO iterables
    }
    return new Bytes (p, n);
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

Str::Str (char const* s, int n) {
    assert(n >= 0 || s != nullptr);
    if (n < 0)
        n = strlen(s);
    insert(0, n);
    adj(n+1);
    assert((int) cap() > n);
    if (s != nullptr)
        memcpy(begin(), s, n);
    else
        memset(begin(), 0, n);
    begin()[n] = 0;
}

auto Str::unop (UnOp op) const -> Value {
    switch (op) {
        case UnOp::Intg: return Int::conv((char const*) begin());
        default:         break;
    }
    return Bytes::unop(op);
}

auto Str::binop (BinOp op, Value rhs) const -> Value {
    auto l = (char const*) begin();
    char const* r = nullptr;
    if (rhs.isStr())
        r = rhs;
    else {
        auto o = rhs.ifType<Str>();
        if (o != nullptr)
            r = *o;
    }
    switch (op) {
        case BinOp::Equal:
            return Value::asBool(r != 0 && strcmp(l, r) == 0);
        case BinOp::Add: {
            assert(r != nullptr);
            auto nl = strlen(l), nr = strlen(r);
            auto o = new struct Str (nullptr, nl + nr);
            memcpy((char*) o->begin(), l, nl);
            memcpy((char*) o->begin() + nl, r, nr);
            return o;
        }
        default:
            break;
    }
    return Bytes::binop(op, rhs);
}

auto Str::getAt (Value k) const -> Value {
    if (!k.isInt())
        return sliceGetter(k);
    int idx = k;
    if (idx < 0)
        idx += size();
    return new Str ((char const*) begin() + idx, 1); // TODO utf-8
}

auto Str::create (ArgVec const& args, Type const*) -> Value {
    assert(args._num == 1 && args[0].isStr());
    return new Str (args[0]);
}

auto Str::repr (Buffer& buf) const -> Value {
    putsEsc(buf, (char const*) begin());
    return {};
}

void VaryVec::atAdj (uint32_t idx, uint32_t len) {
    assert(idx < _fill);
    auto olen = atLen(idx);
    if (len == olen)
        return;
    auto ofill = _fill;
    _fill = pos(_fill);
    if (len > olen)
        ByteVec::insert(pos(idx+1), len - olen);
    else
        ByteVec::remove(pos(idx) + len, olen - len);
    _fill = ofill;

    for (uint32_t i = idx + 1; i <= _fill; ++i)
        pos(i) += len - olen;
}

void VaryVec::atSet (uint32_t idx, void const* ptr, uint32_t len) {
    atAdj(idx, len);
    memcpy(begin() + pos(idx), ptr, len);
}

void VaryVec::insert (uint32_t idx, uint32_t num) {
    assert(idx <= _fill);
    if (cap() == 0) {
        ByteVec::insert(0, 2);
        pos(0) = 2;
        _fill = 0;
    }

    auto ofill = _fill;
    _fill = pos(_fill);
    ByteVec::insert(2 * idx, 2 * num);
    _fill = ofill + num;

    for (uint32_t i = 0; i <= _fill; ++i)
        pos(i) += 2 * num;
    for (uint32_t i = 0; i < num; ++i)
        pos(idx+i) = pos(idx+num);
}

void VaryVec::remove (uint32_t idx, uint32_t num) {
    assert(idx + num <= _fill);
    auto diff = pos(idx+num) - pos(idx);

    auto ofill = _fill;
    _fill = pos(_fill);
    ByteVec::remove(pos(idx), diff);
    ByteVec::remove(2 * idx, 2 * num);
    _fill = ofill - num;

    for (uint32_t i = 0; i <= _fill; ++i)
        pos(i) -= 2 * num;
    for (uint32_t i = idx; i <= _fill; ++i)
        pos(i) -= diff;
}

auto Lookup::operator[] (char const* key) const -> Value {
    for (uint32_t i = 0; i < _count; ++i)
        if (strcmp(key, _items[i].k) == 0)
            return _items[i].v;
    return {};
}

auto Lookup::getAt (Value k) const -> Value {
    assert(k.isStr());
    return (*this)[k];
}

void Lookup::marker () const {
    for (uint32_t i = 0; i < _count; ++i)
        _items[i].v.marker();
}

Tuple::Tuple (ArgVec const& args) : _fill (args._num) {
    memcpy((Value*) data(), args.begin(), args._num * sizeof (Value));
}

auto Tuple::getAt (Value k) const -> Value {
    if (!k.isInt())
        return sliceGetter(k);
    return data()[k];
}

auto Tuple::copy (Range const& r) const -> Value {
    int n = r.len();
    Vector avec; // TODO messy way to create tuple via sized vec with no data
    avec.insert(0, n);
    auto v = Tuple::create({avec, n, 0});
    auto p = (Value*) (&v.asType<Tuple>() + 1);
    for (int i = 0; i < n; ++i)
        p[i] = getAt(r.getAt(i));
    return v;
}

void Tuple::marker () const {
    for (uint32_t i = 0; i < _fill; ++i)
        data()[i].marker();
}

auto Tuple::create (ArgVec const& args, Type const*) -> Value {
    if (args._num == 0)
        return Empty; // there's one unique empty tuple
    return new (args._num * sizeof (Value)) Tuple (args);
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

List::List (ArgVec const& args) {
    insert(0, args._num);
    for (int i = 0; i < args._num; ++i)
        (*this)[i] = args[i];
}

auto List::pop (int idx) -> Value {
    auto n = relPos(idx);
    assert(size() > n);
    Value v = (*this)[n];
    remove(n);
    return v;
}

void List::append (Value v) {
    auto n = size();
    insert(n);
    (*this)[n] = v;
}

auto List::getAt (Value k) const -> Value {
    if (!k.isInt())
        return sliceGetter(k);
    auto n = relPos(k);
    assert(n < size());
    return (*this)[n];
}

auto List::setAt (Value k, Value v) -> Value {
    if (!k.isInt())
        return sliceSetter(k, v);
    auto n = relPos(k);
    assert(n < size());
    return (*this)[n] = v;
}

auto List::copy (Range const& r) const -> Value {
    auto n = r.len();
    auto v = new List;
    v->insert(0, n);
    for (uint32_t i = 0; i < n; ++i)
        (*v)[i] = (*this)[r.getAt(i)];
    return v;
}

auto List::store (Range const& r, Object const& v) -> Value {
    assert(r._by == 1);
    int olen = r.len();
    int nlen = v.len();
    if (nlen < olen)
        remove(r._from + nlen, olen - nlen);
    else if (nlen > olen)
        insert(r._from + olen, nlen - olen);
    for (int i = 0; i < nlen; ++i)
        (*this)[r.getAt(i)] = v.getAt(i);
    return {};
}

auto List::create (ArgVec const& args, Type const*) -> Value {
    return new List (args);
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

auto Set::find (Value v) const -> uint32_t {
    for (auto& e : *this)
        if (v == e)
            return &e - begin();
    return size();
}

auto Set::Proxy::operator= (bool f) -> bool {
    auto n = s.size();
    auto pos = s.find(v);
    if (pos < n && !f)
        s.remove(pos);
    else if (pos == n && f) {
        s.insert(pos);
        s[pos] = v;
    }
    return pos < n;
}

auto Set::binop (BinOp op, Value rhs) const -> Value {
    if (op == BinOp::Contains)
        return Value::asBool(find(rhs) < size());
    return Object::binop(op, rhs);
}

auto Set::getAt (Value k) const -> Value {
    assert(k.isInt());
    auto f = (*this)[k];
    return Value::asBool(f);
}

auto Set::setAt (Value k, Value v) -> Value {
    assert(k.isInt());
    auto f = (*this)[k] = v.truthy();
    return Value::asBool(f);
}

auto Set::create (ArgVec const& args, Type const*) -> Value {
    auto p = new Set (args);
    p->adj(args._num);
    for (int i = 0; i < args._num; ++i)
        p->has(args[i]) = true; // TODO append, no need to lookup
    return p;
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

// dict invariant: items layout is: N keys, then N values, with N == d.size()
auto Dict::Proxy::operator= (Value v) -> Value {
    Value w;
    auto n = d.size();
    auto pos = d.find(k);
    if (v.isNil()) {
        if (pos < n) {
            d._fill = 2*n;    // don't wipe existing vals
            d.remove(n+pos);  // remove value
            d.remove(pos);    // remove key
            d._fill = --n;    // set length to new key count
        }
    } else {
        if (pos == n) { // move all values up and create new gaps
            d._fill = 2*n;    // don't wipe existing vals
            d.insert(2*n);    // create slot for new value
            d.insert(n);      // same for key, moves all vals one up
            d._fill = ++n;    // set length to new key count
            d[pos] = k;       // store the key
        } else
            w = d[n+pos];
        assert(d.cap() >= 2*n);
        d[n+pos] = v;
    }
    return w;
}

auto Dict::at (Value k) const -> Value {
    auto n = size();
    auto pos = find(k);
    return pos < n ? (*this)[n+pos] :
            _chain != nullptr ? _chain->getAt(k) : Value {};
}

auto Dict::keys () -> Value {
    return new DictView (*this, 0);
}

auto Dict::values () -> Value {
    return new DictView (*this, 1);
}

auto Dict::items () -> Value {
    return new DictView (*this, 2);
}

void Dict::marker () const {
    auto& v = (Vector const&) *this;
    for (uint32_t i = 0; i < 2*_fill; ++i) // note: twice the fill
        v[i].marker();
    mark(_chain);
}

auto Dict::create (ArgVec const&, Type const*) -> Value {
    // TODO pre-alloc space to support fast add, needs vals midway cap iso len
    return new Dict;
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

auto DictView::getAt (Value k) const -> Value {
    assert(k.isInt());
    int n = k;
    if (_vtype == 1)
        n += _dict._fill;
    if (_vtype <= 1)
        return _dict[n];
    Vector avec;
    avec.insert(0, 2);
    avec[0] = _dict[n];
    avec[1] = _dict[n+_dict._fill];
    return Tuple::create({avec, 2, 0});
}

auto Type::call (ArgVec const& args) const -> Value {
    return _factory(args, this);
}

auto Type::noFactory (ArgVec const&, const Type*) -> Value {
    assert(false);
    return {};
}

auto Type::create (ArgVec const& args, Type const*) -> Value {
    assert(args._num == 1);
    Value v = args[0];
    switch (v.tag()) {
        case Value::Nil: break;
        case Value::Int: return "int";
        case Value::Str: return "str";
        case Value::Obj: return v->type()._name;
    }
    return {};
}

auto Type::repr (Buffer& buf) const -> Value {
    buf.print("<type %s>", (char const*) _name);
    return {};
}

Class::Class (ArgVec const& args) : Type (args[1], nullptr, Inst::create) {
    assert(2 <= args._num && args._num <= 3); // no support for multiple inheritance
    if (args._num > 2)
        _chain = &args[2].asType<Class>();

    at(Q( 23,"__name__")) = args[1];
    at(Q(169,"__bases__")) = Tuple::create({args._vec, args._num-2, args._off+2});

    args[0]->call({args._vec, args._num - 2, args._off + 2});
}

auto Class::create (ArgVec const& args, Type const*) -> Value {
    assert(args._num >= 2 && args[0].isObj() && args[1].isStr());
    return new Class (args);
}

auto Class::repr (Buffer& buf) const -> Value {
    buf.print("<class %s>", (char const*) at("__name__"));
    return {};
}

Super::Super (ArgVec const& args) {
    assert(args._num == 2);
    _sclass = args[0];
    _sinst = args[1];
}

auto Super::create (ArgVec const& args, Type const*) -> Value {
    return new Super (args);
}

auto Super::repr (Buffer& buf) const -> Value {
    buf << "<super: ...>";
    return {};
}

Inst::Inst (ArgVec const& args, Class const& cls) : Dict (&cls) {
    Value self;
    Value init = attr(Q( 17,"__init__"), self);
    if (!init.isNil()) {
        // stuff "self" before the args passed in TODO is this always ok ???
        args[-1] = this;
        init->call({args._vec, args._num + 1, args._off - 1});
    }
}

auto Inst::create (ArgVec const& args, Type const* t) -> Value {
    Value v = t;
    return new Inst (args, v.asType<Class>());
}

auto Inst::repr (Buffer& buf) const -> Value {
    buf.print("<%s object at %p>", (char const*) type()._name, this);
    return {};
}
