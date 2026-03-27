#ifndef LEGACY_FRAM_H
#define LEGACY_FRAM_H

#include <stdbool.h>
#include <stdint.h>

#include "lfs.h"

bool InitFramMemory(void);
bool ReadFramMemoryWithoutDebug(uint16_t address, uint8_t *buff, uint16_t len);
bool WriteFramMemoryWithoutDebug(uint16_t address, uint8_t *buff, uint16_t len);

#endif // LEGACY_FRAM_H