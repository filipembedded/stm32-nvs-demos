#include "norfx_test_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

__attribute__((weak)) int norfx_test_log_transport_ready(void)
{
    return 0;
}

__attribute__((weak)) void norfx_test_log_transport_write(const uint8_t *data, size_t length)
{
    (void)data;
    (void)length;
}

void norfx_test_log_init(void)
{
}

void norfx_test_log_write_bytes(const uint8_t *data, size_t length)
{
    if (data == NULL || length == 0u)
    {
        return;
    }

    if (norfx_test_log_transport_ready() == 0)
    {
        return;
    }

    norfx_test_log_transport_write(data, length);
}

void norfx_test_log_write(const char *text)
{
    if (text == NULL)
    {
        return;
    }

    norfx_test_log_write_bytes((const uint8_t *)text, strlen(text));
}

void norfx_test_log_printf(const char *format, ...)
{
    char buffer[192];
    va_list args;

    if (format == NULL)
    {
        return;
    }

    va_start(args, format);
    int written = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (written <= 0)
    {
        return;
    }

    size_t length = (size_t)written;
    if (length >= sizeof(buffer))
    {
        length = sizeof(buffer) - 1u;
    }

    norfx_test_log_write_bytes((const uint8_t *)buffer, length);
}

int __io_putchar(int ch)
{
    uint8_t value = (uint8_t)ch;
    norfx_test_log_write_bytes(&value, 1u);
    return ch;
}
