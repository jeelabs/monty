namespace monty {

    //CG< type array
    struct Array : Bytes {
        static auto create (ArgVec const&,Type const* =nullptr) -> Value;
        static Lookup const attrs;
        static Type info;
        auto type () const -> Type const& override { return info; }
        auto repr (Buffer&) const -> Value override;
    //CG>
        constexpr static auto LEN_BITS = 27;

        //constexpr Array () =default; // default is array of Value items
        Array (char type, uint32_t num =0);

        auto mode () const -> char;

        void insert (uint32_t idx, uint32_t num =1);
        void remove (uint32_t idx, uint32_t num =1);

        auto len () const -> uint32_t override;
        auto getAt (Value k) const -> Value override;
        auto setAt (Value k, Value v) -> Value override;
        auto copy (Range const&) const -> Value override;
        auto store (Range const&, Object const&) -> Value override;

    private:
        auto sel () const -> uint8_t { return _fill >> LEN_BITS; }
    };
}
