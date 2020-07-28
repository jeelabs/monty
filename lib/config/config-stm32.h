#define F103RC_NET      1    // see configs/genericSTM32F103RC.ini
#define F103ZE_NET      2    // see configs/genericSTM32F103ZE.ini

#if CONFIG == F103RC_NET
    #define INCLUDE_NETWORK 1
    #define PINS_NETWORK PinA<7>, PinA<6>, PinA<5>, PinC<4>
    #define INCLUDE_SDCARD 1
    #define PINS_SDCARD PinD<2>, PinC<8>, PinC<12>, PinC<11>
#elif CONFIG == F103ZE_NET
    #define INCLUDE_NETWORK 1
    #define PINS_NETWORK PinA<7>, PinA<6>, PinA<5>, PinA<4>
#endif

#if BOARD_discovery_f4 || STM32L412xx
    #define PINS_CONSOLE PinA<2>, PinA<3>
#elif STM32H743xx // nucleo-144
    #define PINS_CONSOLE PinD<8>, PinD<9>
#else
    #define PINS_CONSOLE PinA<9>, PinA<10>
#endif

// see monty/gc.cpp
#ifndef GC_MEM_BYTES
#define GC_MEM_BYTES (10*1024)  // 10 Kb total memory
#endif
#ifndef GC_MEM_ALIGN
#define GC_MEM_ALIGN 8          // 8-byte slot boundaries
#endif
