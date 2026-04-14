#ifndef NORFX_SELFTEST_H
#define NORFX_SELFTEST_H

#include <stdint.h>

#include "nor_fx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t total;
    uint32_t passed;
    uint32_t failed;
} norfx_selftest_summary_t;

norfx_selftest_summary_t norfx_selftest_run(struct norfx_device *dev);

#ifdef __cplusplus
}
#endif

#endif
