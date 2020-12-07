#if STM32L4
constexpr auto flashSegSize = 2048;
#endif

struct SegmentHdr {
    uint32_t magic;
    void (*regFun)();
    void (*deregFun)();
    uint8_t const* romEnd;
    uint8_t const* ramEnd;

    auto isValid () const -> bool {
        return magic == 0x12345678;
    }

    static auto next () -> SegmentHdr const& {
        extern uint8_t g_pfnVectors[], _sidata [], _sdata [], _edata [];
        auto romSize = _sidata + (_edata - _sdata) - g_pfnVectors;
        auto romAlign = romSize + (-romSize & (flashSegSize-1));
        auto romNext = g_pfnVectors + romAlign;
        return *(SegmentHdr const*) romNext;
    }
};
