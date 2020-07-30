// Application wide configuration

#if NATIVE
#include "config-native.h"
#endif

#if STM32F1 || STM32F4 || STM32H7 || STM32L0 || STM32L4
#include "config-stm32.h"
#endif

#if INCLUDE_NETWORK
extern const ModuleObj m_network;
#endif

#if INCLUDE_SDCARD
extern const ModuleObj m_sdcard;
#endif
