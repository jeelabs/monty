#define F103RC_NET      1    // see configs/genericSTM32F103RC.ini
#define F103ZE_NET      2    // see configs/genericSTM32F103ZE.ini
#define GOLD_DRAGON     3    // see configs/genericSTM32F407VGT6.ini

#if CONFIG == F103RC_NET
    #define INCLUDE_NETWORK 1
    #define PINS_NETWORK PinA<7>, PinA<6>, PinA<5>, PinC<4>
    #define INCLUDE_SDCARD 1
    #define PINS_SDCARD PinD<2>, PinC<8>, PinC<12>, PinC<11>
#elif CONFIG == F103ZE_NET
    #define INCLUDE_NETWORK 1
    #define PINS_NETWORK PinA<7>, PinA<6>, PinA<5>, PinA<4>
#endif

#if BOARD_discovery_f4 || STM32L073xx || STM32L412xx
    #define PINS_CONSOLE PinA<2>, PinA<3>
#elif STM32H743xx // nucleo-144
    #define PINS_CONSOLE PinD<8>, PinD<9>
#elif CONFIG == GOLD_DRAGON
    #define PINS_CONSOLE PinC<10>, PinC<11>
#else
    #define PINS_CONSOLE PinA<9>, PinA<10>
#endif

#if BOARD_discovery_f4 || STM32H743xx || CONFIG == GOLD_DRAGON
    #define UART_BUSDIV 4
#else
    #define UART_BUSDIV 1
#endif

#if STM32F1 || INCLUDE_NETWORK
    #define MEM_BYTES (12*1024) // e.g. Blue Pill
#else
    #define MEM_BYTES (32*1024) // will fit in ≥ 48 KB ram
#endif
