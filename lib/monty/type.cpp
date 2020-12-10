// type.cpp - collection types and type system

#include "monty.h"
#include <cassert>

using namespace monty;

Tuple const Tuple::emptyObj;
Value const monty::Empty {Tuple::emptyObj};

auto Iterator::next() -> Value {
    if (ipos < 0)
        return iobj.next();
    if (ipos >= (int) iobj.len())
        return E::StopIteration;
    // TODO duplicate code, see opForIter in pyvm.h
    if (&iobj.type() == &Dict::info || &iobj.type() == &Set::info)
        return ((List&) iobj)[ipos++]; // avoid keyed access
    return iobj.getAt(ipos++);
}

auto Range::len () const -> uint32_t {
    assert(by != 0);
    auto n = (to - from + by + (by > 0 ? -1 : 1)) / by;
    return n < 0 ? 0 : n;
}

auto Range::getAt (Value k) const -> Value {
    return from + k * by;
}

auto Range::create (ArgVec const& args, Type const*) -> Value {
    assert(1 <= args.num && args.num <= 3);
    int a = args.num > 1 ? (int) args[0] : 0;
    int b = args.num == 1 ? args[0] : args[1];
    int c = args.num > 2 ? (int) args[2] : 1;
    return new Range (a, b, c);
}

auto Slice::asRange (int sz) const -> Range {
    int from = off.isInt() ? (int) off : 0;
    int to = num.isInt() ? (int) num : sz;
    int by = step.isInt() ? (int) step : 1;
    if (from < 0)
        from += sz;
    if (to < 0)
        to += sz;
    if (by < 0) {
        auto t = from - 1;
        from = to - 1;
        to = t;
    }
    return {from, to, by};
}

auto Slice::create (ArgVec const& args, Type const*) -> Value {
    assert(1 <= args.num && args.num <= 3);
    Value a = args.num > 1 ? args[0] : Null;
    Value b = args.num == 1 ? args[0] : args[1];
    Value c = args.num > 2 ? args[2] : Null;
    return new Slice (a, b, c);
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
    assert(args.num == 1);
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
        case UnOp::Int:  return Int::conv((char const*) begin());
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
    assert(args.num == 1 && args[0].isStr());
    return new Str (args[0]);
}

void VaryVec::atAdj (uint32_t idx, uint32_t len) {
    assert(idx < fill);
    auto olen = atLen(idx);
    if (len == olen)
        return;
    auto ofill = fill;
    fill = pos(fill);
    if (len > olen)
        ByteVec::insert(pos(idx+1), len - olen);
    else
        ByteVec::remove(pos(idx) + len, olen - len);
    fill = ofill;

    for (uint32_t i = idx + 1; i <= fill; ++i)
        pos(i) += len - olen;
}

void VaryVec::atSet (uint32_t idx, void const* ptr, uint32_t len) {
    atAdj(idx, len);
    memcpy(begin() + pos(idx), ptr, len);
}

void VaryVec::insert (uint32_t idx, uint32_t num) {
    assert(idx <= fill);
    if (cap() == 0) {
        ByteVec::insert(0, 2);
        pos(0) = 2;
        fill = 0;
    }

    auto ofill = fill;
    fill = pos(fill);
    ByteVec::insert(2 * idx, 2 * num);
    fill = ofill + num;

    for (uint32_t i = 0; i <= fill; ++i)
        pos(i) += 2 * num;
    for (uint32_t i = 0; i < num; ++i)
        pos(idx+i) = pos(idx+num);
}

void VaryVec::remove (uint32_t idx, uint32_t num) {
    assert(idx + num <= fill);
    auto diff = pos(idx+num) - pos(idx);

    auto ofill = fill;
    fill = pos(fill);
    ByteVec::remove(pos(idx), diff);
    ByteVec::remove(2 * idx, 2 * num);
    fill = ofill - num;

    for (uint32_t i = 0; i <= fill; ++i)
        pos(i) -= 2 * num;
    for (uint32_t i = idx; i <= fill; ++i)
        pos(i) -= diff;
}

auto Lookup::operator[] (char const* key) const -> Value {
    for (uint32_t i = 0; i < count; ++i)
        if (strcmp(key, items[i].k) == 0)
            return items[i].v;
    return {};
}

auto Lookup::getAt (Value k) const -> Value {
    assert(k.isStr());
    return (*this)[k];
}

void Lookup::marker () const {
    for (uint32_t i = 0; i < count; ++i)
        items[i].v.marker();
}

Tuple::Tuple (ArgVec const& args) : fill (args.num) {
    memcpy((Value*) data(), args.begin(), args.num * sizeof (Value));
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
    for (uint32_t i = 0; i < fill; ++i)
        data()[i].marker();
}

auto Tuple::create (ArgVec const& args, Type const*) -> Value {
    if (args.num == 0)
        return Empty; // there's one unique empty tuple
    return new (args.num * sizeof (Value)) Tuple (args);
}

List::List (ArgVec const& args) {
    insert(0, args.num);
    for (int i = 0; i < args.num; ++i)
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
    assert(r.by == 1);
    int olen = r.len();
    int nlen = v.len();
    if (nlen < olen)
        remove(r.from + nlen, olen - nlen);
    else if (nlen > olen)
        insert(r.from + olen, nlen - olen);
    for (int i = 0; i < nlen; ++i)
        (*this)[r.getAt(i)] = v.getAt(i);
    return {};
}

auto List::create (ArgVec const& args, Type const*) -> Value {
    return new List (args);
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
    p->adj(args.num);
    for (int i = 0; i < args.num; ++i)
        p->has(args[i]) = true; // TODO append, no need to lookup
    return p;
}

// dict invariant: items layout is: N keys, then N values, with N == d.size()
auto Dict::Proxy::operator= (Value v) -> Value {
    Value w;
    auto n = d.size();
    auto pos = d.find(k);
    if (v.isNil()) {
        if (pos < n) {
            d.fill = 2*n;     // don't wipe existing vals
            d.remove(n+pos);  // remove value
            d.remove(pos);    // remove key
            d.fill = --n;     // set length to new key count
        }
    } else {
        if (pos == n) { // move all values up and create new gaps
            d.fill = 2*n;     // don't wipe existing vals
            d.insert(2*n);    // create slot for new value
            d.insert(n);      // same for key, moves all vals one up
            d.fill = ++n;     // set length to new key count
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
            chain != nullptr ? chain->getAt(k) : Value {};
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
    for (uint32_t i = 0; i < 2 * fill; ++i) // note: twice the fill
        v[i].marker();
    mark(chain);
}

auto Dict::create (ArgVec const&, Type const*) -> Value {
    // TODO pre-alloc space to support fast add, needs vals midway cap iso len
    return new Dict;
}

auto DictView::getAt (Value k) const -> Value {
    assert(k.isInt());
    int n = k;
    if (vtype == 1)
        n += dict.fill;
    if (vtype <= 1)
        return dict[n];
    Vector avec;
    avec.insert(0, 2);
    avec[0] = dict[n];
    avec[1] = dict[n+dict.fill];
    return Tuple::create({avec, 2, 0});
}

auto Type::call (ArgVec const& args) const -> Value {
    return factory(args, this);
}

auto Type::noFactory (ArgVec const&, const Type*) -> Value {
    assert(false);
    return {};
}

auto Type::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1);
    Value v = args[0];
    switch (v.tag()) {
        case Value::Nil: break;
        case Value::Int: return "int";
        case Value::Str: return "str";
        case Value::Obj: return v.obj().type().name;
    }
    return {};
}

Class::Class (ArgVec const& args) : Type (args[1], Inst::create) {
    assert(2 <= args.num && args.num <= 3); // no support for multiple inheritance
    if (args.num > 2)
        chain = &args[2].asType<Class>();

    at(Q( 23,"__name__")) = args[1];
    at(Q(183,"__bases__")) = Tuple::create({args.vec, args.num-2, args.off+2});

    args[0].obj().call({args.vec, args.num - 2, args.off + 2});

    auto ctx = Interp::context;
    assert(ctx != nullptr);
    ctx->frame().locals = this;
}

auto Class::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num >= 2 && args[0].isObj() && args[1].isStr());
    return new Class (args);
}

Super::Super (ArgVec const& args) {
    assert(args.num == 2);
    sclass = args[0];
    sinst = args[1];
}

auto Super::create (ArgVec const& args, Type const*) -> Value {
    return new Super (args);
}

Inst::Inst (ArgVec const& args, Class const& cls) : Dict (&cls) {
    auto ctx = Interp::context;
    assert(ctx != nullptr); (void) ctx;

    Value self;
    Value init = attr(Q( 17,"__init__"), self);
    if (!init.isNil()) {
        // stuff "self" before the args passed in TODO is this always ok ???
        assert(ctx == &args.vec && args.off > 0);
        args[-1] = this;
        init.obj().call({args.vec, args.num + 1, args.off - 1});
    }
}

auto Inst::create (ArgVec const& args, Type const* t) -> Value {
    Value v = t;
    return new Inst (args, v.asType<Class>());
}
