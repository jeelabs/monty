#include <stdint.h>

#define ENC28J60_USE_PBUF 1

extern int debugf (const char*, ...);
#define DEBUG debugf

typedef struct {} enchw_device_t;

void enchw_setup(enchw_device_t *dev);
void enchw_select(enchw_device_t *dev);
void enchw_unselect(enchw_device_t *dev);
uint8_t enchw_exchangebyte(enchw_device_t *dev, uint8_t byte);
