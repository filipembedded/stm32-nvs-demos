#ifndef NORFX_TEST_LOG_H
#define NORFX_TEST_LOG_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void norfx_test_log_init(void);
void norfx_test_log_write(const char *text);
void norfx_test_log_write_bytes(const uint8_t *data, size_t length);
void norfx_test_log_printf(const char *format, ...);

int norfx_test_log_transport_ready(void);
void norfx_test_log_transport_write(const uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif
