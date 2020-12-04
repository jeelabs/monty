#if STM32L4
constexpr auto flashSegSize = 2048;
#endif

struct SegmentHdr {
    uint32_t magic;
    void (*regFun)();
    void (*deregFun)();
};

// this code is in a header file and will be compiled with different globals
static auto nextSegment () -> SegmentHdr const& {
    extern uint8_t g_pfnVectors[], _sidata [];
    auto romSize = _sidata - g_pfnVectors;
    auto romAlign = romSize + (-romSize & (flashSegSize-1));
    auto romNext = g_pfnVectors + romAlign;
    return *(SegmentHdr const*) romNext;
}
