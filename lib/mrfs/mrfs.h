namespace mrfs {
    constexpr auto MAGIC = 0x3079746D; // 'mty0'

    struct Info {
        // only first fields are located at the start
        uint32_t magic;
        uint32_t size :24;
        uint32_t flags :8;
        // everything else lives in the tail part
        char name [15], zero;
        uint32_t time;
        uint32_t crc;

        auto valid () const -> bool { return magic == MAGIC; }
        auto data () -> void* { return name; }
        auto tail () -> Info* { return this + (size+31)/32; }
    };
    static_assert(sizeof (Info) == 32, "incorrect header size");

    extern Info* base; // start of flash
    extern int skip;   // items to skip, i.e. non-MRFS content
    extern Info* next; // next unused position
    extern Info* last; // past end of flash used for MRFS

    void init (void* ptr, size_t len, size_t keep =0);
    void wipe ();
    void dump ();

    auto add (char const* name, uint32_t time,
                void const* buf, uint32_t len) -> int;
    auto find (char const* name) -> int;
}
