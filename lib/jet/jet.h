namespace jet {
    using namespace monty;

    struct Gadget : Object {
        void marker () const override;
    };

    struct Flow : Object {
        static Type info;
        auto type () const -> Type const& override { return info; }

        Array _fanout;  // number of outgoing wires per outlet
        Array _wires;   // ieach one is stored as an EndPoint
        Array _index;   // offset into _state
        Vector _state;  // all gadget inlets, outlets, and other state

        struct Endpoint {
            uint16_t pos :4;
            uint16_t num :12;
        };

        struct GadGetInfo {
            uint32_t ins :4;    // number of inlets
            uint32_t outs :4;   // number of outlets
            uint32_t type :8;   // gadget type
            uint32_t live :15;  // bitmap of live inlets
            uint32_t ignore :1;
        };

        Flow ();

        auto offset (int i) const -> int {
            return ((uint16_t const*) _index.begin())[i];
        }

        auto ginfo (int i) const -> GadGetInfo {
            int v = _state[offset(i)];
            return (GadGetInfo&) v;
        }

        auto entry (int i) const -> ArgVec {
            auto off = offset (i);
            return {_state, off, offset(i+1)-off};
        }

        void marker () const override;
    };
}
