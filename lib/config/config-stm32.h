#define F103RC_NET      1    // see configs/genericSTM32F103RC.ini
#define F103ZE_NET      2    // see configs/genericSTM32F103ZE.ini

#if CONFIG == F103RC_NET || CONFIG == F103ZE_NET
#define INCLUDE_NETWORK 1
#endif

// see monty/gc.cpp
#ifndef GC_MEM_BYTES
#define GC_MEM_BYTES (10*1024)  // 10 Kb total memory
#endif
#ifndef GC_MEM_ALIGN
#define GC_MEM_ALIGN 8          // 8-byte slot boundaries
#endif
