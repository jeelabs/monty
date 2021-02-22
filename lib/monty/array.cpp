// array.cpp - arrays, dicts, and other derived types

#include "monty.h"
#include <cassert>

using namespace monty;

Lookup const Array::attrs;

struct Accessor {
    virtual auto get (ByteVec&, uint32_t) const -> Value  = 0;
    virtual void set (ByteVec&, uint32_t, Value) const = 0;
    virtual void ins (ByteVec&, uint32_t, uint32_t) const = 0;
    virtual void del (ByteVec&, uint32_t, uint32_t) const = 0;
};

template< typename T >
static auto asVecOf (ByteVec& a) -> VecOf<T>& {
    return (VecOf<T>&) a;
}

template< typename T, int L =0 >
struct AccessAs : Accessor {
    auto get (ByteVec& vec, uint32_t pos) const -> Value override {
        if (L)
            return Int::make(asVecOf<T>(vec)[pos]);
        else
            return asVecOf<T>(vec)[pos];
    }
    void set (ByteVec& vec, uint32_t pos, Value val) const override {
        if (L)
            asVecOf<T>(vec)[pos] = val.asInt();
        else
            asVecOf<T>(vec)[pos] = val;
    }
    void ins (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        asVecOf<T>(vec).insert(pos, num);
    }
    void del (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        asVecOf<T>(vec).remove(pos, num);
    }
};

template< int L >                                   // 0 1 2
struct AccessAsBits : Accessor {                    // P T N
    constexpr static auto bits = 1 << L;            // 1 2 4
    constexpr static auto mask = (1 << bits) - 1;   // 1 3 15
    constexpr static auto shft = 3 - L;             // 3 2 1
    constexpr static auto rest = (1 << shft) - 1;   // 7 3 1

    auto get (ByteVec& vec, uint32_t pos) const -> Value override {
        return (vec[pos>>shft] >> bits * (pos & rest)) & mask;
    }
    void set (ByteVec& vec, uint32_t pos, Value val) const override {
        auto b = bits * (pos & rest);
        auto& e = vec[pos>>shft];
        e = (e & ~(mask << b)) | (((int) val & mask) << b);
    }
    void ins (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        assert((pos & rest) == 0 && (num & rest) == 0);
        vec._fill >>= shft;
        vec.insert(pos >> shft, num >> shft);
        vec._fill <<= shft;
    }
    void del (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        assert((pos & rest) == 0 && (num & rest) == 0);
        vec._fill >>= shft;
        vec.remove(pos >> shft, num >> shft);
        vec._fill <<= shft;
    }
};

struct AccessAsVaryBytes : Accessor {
    auto get (ByteVec& vec, uint32_t pos) const -> Value override {
        auto& v = (VaryVec&) vec;
        return new Bytes (v.atGet(pos), v.atLen(pos));
    }
    void set (ByteVec& vec, uint32_t pos, Value val) const override {
        auto& v = (VaryVec&) vec;
        if (val.isNil())
            v.remove(pos);
        else {
            auto& b = val.asType<Bytes>();
            v.atSet(pos, b.begin(), b.size());
        }
    }
    void ins (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        ((VaryVec&) vec).insert(pos, num);
    }
    void del (ByteVec& vec, uint32_t pos, uint32_t num) const override {
        ((VaryVec&) vec).remove(pos, num);
    }
};

struct AccessAsVaryStr : AccessAsVaryBytes {
    auto get (ByteVec& vec, uint32_t pos) const -> Value override {
        return new Str ((char const*) ((VaryVec&) vec).atGet(pos));
    }
    void set (ByteVec& vec, uint32_t pos, Value val) const override {
        auto& v = (VaryVec&) vec;
        if (val.isNil())
            v.remove(pos);
        else {
            char const* s = val.isStr() ? val : val.asType<Str>();
            v.atSet(pos, (char const*) s, strlen(s) + 1);
        }
    }
};

constexpr char arrayModes [] = "PTNbBhHiIlLqvV"
#if USE_FLOAT
                            "f"
#endif
#if USE_DOUBLE
                            "d"
#endif
;

// permanent per-type accessors, these are re-used for every Array instance
// the cost per get/set/ins/del is one table index step, just as with vtables

static AccessAsBits<0>      const accessor_P;
static AccessAsBits<1>      const accessor_T;
static AccessAsBits<2>      const accessor_N;
static AccessAs<int8_t>     const accessor_b;
static AccessAs<uint8_t>    const accessor_B;
static AccessAs<int16_t>    const accessor_h;
static AccessAs<uint16_t>   const accessor_H;
static AccessAs<int32_t,1>  const accessor_l;
static AccessAs<uint32_t,1> const accessor_L;
static AccessAs<int64_t,1>  const accessor_q;
static AccessAsVaryBytes    const accessor_v;
static AccessAsVaryStr      const accessor_V;
#if USE_FLOAT
static AccessAs<float>      const accessor_f;
#endif
#if USE_DOUBLE
static AccessAs<double>     const accessor_d;
#endif

// must be in same order as arrayModes
static Accessor const* accessors [] = {
    &accessor_P,
    &accessor_T,
    &accessor_N,
    &accessor_b,
    &accessor_B,
    &accessor_h,
    &accessor_H,
    &accessor_h, // i, same as h
    &accessor_H, // I, same as H
    &accessor_l,
    &accessor_L,
    &accessor_q,
    &accessor_v,
    &accessor_V,
#if USE_FLOAT
    &accessor_f,
#endif
#if USE_DOUBLE
    &accessor_d,
#endif
};

static_assert (sizeof arrayModes - 1 <= 1 << (32 - Array::LEN_BITS),
                "not enough bits for accessor modes");
static_assert (sizeof arrayModes - 1 == sizeof accessors / sizeof *accessors,
                "incorrect number of accessors");

Array::Array (char type, uint32_t len) {
    auto p = strchr(arrayModes, type);
    auto s = p != nullptr ? p - arrayModes : 0; // use Value if unknown type
    accessors[s]->ins(*this, 0, len);
    _fill |= s << LEN_BITS;
}

auto Array::mode () const -> char {
    return arrayModes[_fill >> LEN_BITS];
}

auto Array::len () const -> uint32_t {
    return size() & ((1 << LEN_BITS) - 1);
}

auto Array::getAt (Value k) const -> Value {
    if (!k.isInt())
        return sliceGetter(k);
    auto n = k; // TODO relPos(k);
    return accessors[sel()]->get(const_cast<Array&>(*this), n);
}

auto Array::setAt (Value k, Value v) -> Value {
    if (!k.isInt())
        return sliceSetter(k, v);
    auto n = k; // TODO relPos(k);
    auto s = sel();
    _fill &= 0x07FFFFFF;
    accessors[s]->set(*this, n, v);
    _fill |= s << LEN_BITS;
    return {};
}

void Array::insert (uint32_t idx, uint32_t num) {
    auto s = sel();
    _fill &= 0x07FFFFFF;
    accessors[s]->ins(*this, idx, num);
    _fill = _fill + (s << LEN_BITS);
}

void Array::remove (uint32_t idx, uint32_t num) {
    auto s = sel();
    _fill &= 0x07FFFFFF;
    accessors[s]->del(*this, idx, num);
    _fill += s << LEN_BITS;
}

auto Array::copy (Range const& r) const -> Value {
    auto n = r.len();
    auto v = new Array (mode(), n);
    for (uint32_t i = 0; i < n; ++i)
        v->setAt(i, getAt(r.getAt(i)));
    return v;
}

auto Array::store (Range const& r, Object const& v) -> Value {
    assert(r._by == 1);
    int olen = r.len();
    int nlen = v.len();
    if (nlen < olen)
        remove(r._from + nlen, olen - nlen);
    else if (nlen > olen)
        insert(r._from + olen, nlen - olen);
    for (int i = 0; i < nlen; ++i)
        setAt(r.getAt(i), v.getAt(i));
    return {};
}

auto Array::create (ArgVec const& args, Type const*) -> Value {
    assert(args._num >= 1 && args[0].isStr());
    char type = *((char const*) args[0]);
    uint32_t len = 0;
    if (args._num == 2) {
        assert(args[1].isInt());
        len = args[1];
    }
    return new Array (type, len);
}

Value Array::repr (Buffer& buf) const {
    auto m = mode();
    auto n = len();
    buf.print("%d%c", n, m);
    switch (m) {
        case 'q':                               n <<= 1; // fall through
        case 'l': case 'L':                     n <<= 1; // fall through
        case 'h': case 'H': case 'i': case 'I': n <<= 1; // fall through
        case 'b': case 'B':                     break;
        case 'P':                               n >>= 1; // fall through
        case 'T':                               n >>= 1; // fall through
        case 'N':                               n >>= 1; break;
        case 'v': case 'V': n = ((uint16_t const*) begin())[len()]; break;
    }
    auto p = (uint8_t const*) begin();
    for (uint32_t i = 0; i < n; ++i)
        buf.print("%02x", p[i]);
    return {};
}
