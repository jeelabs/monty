namespace mrfs {
    constexpr auto MAGIC = '0YTM';

    struct Info {
        uint32_t magic;
        uint32_t size :24;
        uint32_t flags :8;
        char name [16];
        uint32_t time;
        uint32_t crc;

        auto isValid () const -> bool { return magic == MAGIC; }
        auto payload () -> void* { return name; }
    };

    Info* start; // start of flash used for MRFS
    Info* next;  // next unused position
    Info* limit; // past end of flash used for MRFS

    void init ();
    void wipe ();
    void dump ();

    auto add(char const* name, uint32_t time,
                void const* buf, uint32_t len) -> int;
}
