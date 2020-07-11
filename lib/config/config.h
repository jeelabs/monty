// Application wide configuration

#if STM32F1 || STM32F4
#include "config-stm32.h"
#endif

#if INCLUDE_NETWORK
extern const ModuleObj m_network;
#endif

#if INCLUDE_SDCARD
extern const ModuleObj m_sdcard;
#endif
