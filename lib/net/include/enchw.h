#include <stdint.h>

#define ENC28J60_USE_PBUF 1

#define DEBUG(...)

typedef struct {} enchw_device_t;

void enchw_setup(enchw_device_t *dev);
void enchw_select(enchw_device_t *dev);
void enchw_unselect(enchw_device_t *dev);
uint8_t enchw_exchangebyte(enchw_device_t *dev, uint8_t byte);
