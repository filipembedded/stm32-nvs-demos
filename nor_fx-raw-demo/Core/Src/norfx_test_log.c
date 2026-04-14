#include "norfx_test_log.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define NORFX_USB_ENUMERATION_TIMEOUT_MS  2000u
#define NORFX_USB_TX_TIMEOUT_MS           100u

extern USBD_HandleTypeDef hUsbDeviceFS;

int norfx_test_log_transport_ready(void)
{
    return (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) &&
           (hUsbDeviceFS.pClassData != NULL);
}

void norfx_test_log_transport_write(const uint8_t *data, size_t length)
{
    if (data == NULL || length == 0u)
    {
        return;
    }

    size_t offset = 0u;
    while (offset < length)
    {
        size_t chunk_length = length - offset;
        if (chunk_length > APP_TX_DATA_SIZE)
        {
            chunk_length = APP_TX_DATA_SIZE;
        }

        uint32_t start_tick = HAL_GetTick();
        while (CDC_Transmit_FS((uint8_t *)&data[offset], (uint16_t)chunk_length) == USBD_BUSY)
        {
            if ((HAL_GetTick() - start_tick) > NORFX_USB_TX_TIMEOUT_MS)
            {
                return;
            }
        }

        offset += chunk_length;
    }
}

void norfx_test_log_init(void)
{
    uint32_t start_tick = HAL_GetTick();

    while (norfx_test_log_transport_ready() == 0)
    {
        if ((HAL_GetTick() - start_tick) > NORFX_USB_ENUMERATION_TIMEOUT_MS)
        {
            return;
        }
    }
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
