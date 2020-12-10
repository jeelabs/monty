// type.cpp - collection types and type system

#include "monty.h"
//#include "ops.h"
#include <cassert>

using namespace monty;

Tuple::Tuple (ArgVec const& args) : fill (args.num) {
    memcpy((Value*) data(), args.begin(), args.num * sizeof (Value));
}

auto Tuple::getAt (Value k) const -> Value {
    if (!k.isInt())
        return sliceGetter(k);
    return data()[k];
}

#if 0 //XXX
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
#endif

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

#if 0
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
#endif

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

#if 0 //XXX
auto Set::binop (BinOp op, Value rhs) const -> Value {
    if (op == BinOp::Contains)
        return Value::asBool(find(rhs) < size());
    return Object::binop(op, rhs);
}
#endif

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
