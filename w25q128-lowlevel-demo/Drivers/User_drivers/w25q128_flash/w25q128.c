#include "w25q128.h"

#define W25Q128_SPI_TIMEOUT_MS 100
#define W25Q128_RECOVERY_TIMEOUT_MS 500

/*************************** Static functions *********************************/
static uint32_t calculate_bytes_to_write(uint32_t size, uint16_t offset);
static uint32_t calculate_bytes_to_modify(uint32_t size, uint16_t offset);

void W25Q128_ChipSelect(W25Q128_TypeDef *w25q128)
{
    HAL_GPIO_WritePin(w25q128->cs_port, w25q128->cs_pin, GPIO_PIN_RESET);
}

void W25Q128_ChipDeselect(W25Q128_TypeDef *w25q128)
{
    HAL_GPIO_WritePin(w25q128->cs_port, w25q128->cs_pin, GPIO_PIN_SET);
}

void W25Q128_SPIWrite(W25Q128_TypeDef *w25q128, uint8_t *data, uint16_t len, 
                                                            uint32_t timeout)
{
    HAL_SPI_Transmit(w25q128->hspi, data, len, timeout);
}

void W25Q128_SPIRead(W25Q128_TypeDef *w25q128, uint8_t *data, uint16_t len, 
                                                            uint32_t timeout)
{
    HAL_SPI_Receive(w25q128->hspi, data, len, timeout);
}

void W25Q128_DelayMs(uint32_t delay_ms)
{
    HAL_Delay(delay_ms);
}

void W25Q128_Reset(W25Q128_TypeDef *w25q128)
{
    uint8_t t_data[2];

    t_data[0] = INST_ENABLE_RESET;
    t_data[1] = INST_RESET_DEVICE;

    W25Q128_ChipSelect(w25q128);

    W25Q128_SPIWrite(w25q128, t_data, 2, W25Q128_SPI_TIMEOUT_MS);

    W25Q128_ChipDeselect(w25q128);

    W25Q128_DelayMs(100);
}

uint32_t W25Q128_ReadID(W25Q128_TypeDef *w25q128, W25Q128_ID_TypeDef id)
{
    uint8_t t_data;
    uint8_t r_data[3];
    uint32_t result_id;

    switch (id)
    {
        case ID_JEDEC:
            t_data = INST_JEDEC_ID;
            W25Q128_ChipSelect(w25q128);
            W25Q128_SPIWrite(w25q128, &t_data, 1, W25Q128_SPI_TIMEOUT_MS);
            W25Q128_SPIRead(w25q128, r_data, 3, W25Q128_SPI_TIMEOUT_MS);
            W25Q128_ChipDeselect(w25q128);

            // MFN_ID : MEM_ID : CAPACITY_ID
            result_id = ((r_data[0] << 16) | (r_data[1] << 8) | (r_data[2])); 
            break;

        default:
            break;
    }
    return result_id;
}

W25Q128_StatusTypeDef W25Q128_Read(W25Q128_TypeDef *w25,
                                    uint32_t start_page,
                                    uint8_t offset,
                                    uint32_t size,
                                    uint8_t *r_data)
{
    uint8_t t_data[4];
    uint32_t mem_addr = (start_page * 256) + offset;

    t_data[0] = INST_READ_DATA;
    t_data[1] = (mem_addr >> 16) & 0xFF; // MSB of 24-bit memory address
    t_data[2] = (mem_addr >> 8) & 0xFF;
    t_data[3] = (mem_addr) & 0xFF;       // LSB of 24-bit memory address

    W25Q128_ChipSelect(w25);
    W25Q128_SPIWrite(w25, t_data, sizeof(t_data), W25Q128_SPI_TIMEOUT_MS);
    W25Q128_SPIRead(w25, r_data, size, W25Q128_SPI_TIMEOUT_MS);
    W25Q128_ChipDeselect(w25);

    return W25Q128_SUCCESS;
}

W25Q128_StatusTypeDef W25Q128_FastRead(W25Q128_TypeDef *w25,
                                        uint32_t start_page,
                                        uint8_t offset,
                                        uint32_t size,
                                        uint8_t *r_data)
{
    uint8_t t_data[5];
    uint32_t mem_addr = (start_page * 256) + offset;
    
    t_data[0] = INST_FAST_READ;
    t_data[1] = (mem_addr >> 16) & 0xFF; // MSB of 24-bit memory address
    t_data[2] = (mem_addr >> 8) & 0xFF;
    t_data[3] = (mem_addr) & 0xFF;       // LSB of 24-bit memory address
    t_data[4] = 0x00;
    
    W25Q128_ChipSelect(w25);
    W25Q128_SPIWrite(w25, t_data, sizeof(t_data), W25Q128_SPI_TIMEOUT_MS);
    W25Q128_SPIRead(w25, r_data, size, W25Q128_SPI_TIMEOUT_MS);
    W25Q128_ChipDeselect(w25);
    
    return W25Q128_SUCCESS;
}

W25Q128_StatusTypeDef W25Q128_WriteEnable(W25Q128_TypeDef *w25)
{
    uint8_t t_data = INST_WRITE_ENABLE;

    W25Q128_ChipSelect(w25);
    W25Q128_SPIWrite(w25, &t_data, sizeof(t_data), W25Q128_SPI_TIMEOUT_MS);
    W25Q128_ChipDeselect(w25);
    
    // TODO: Check if there is some better way to implement this delay...
    W25Q128_DelayMs(5);

    return W25Q128_SUCCESS;
}

W25Q128_StatusTypeDef W25Q128_WriteDisable(W25Q128_TypeDef *w25)
{
    uint8_t t_data = INST_WRITE_DISABLE;

    W25Q128_ChipSelect(w25);
    W25Q128_SPIWrite(w25, &t_data, sizeof(t_data), W25Q128_SPI_TIMEOUT_MS);
    W25Q128_ChipDeselect(w25);
    
    // TODO: Check if there is some better way to implement this delay...
    W25Q128_DelayMs(5);

    return W25Q128_SUCCESS;
}

W25Q128_StatusTypeDef W25Q128_EraseSector(W25Q128_TypeDef *w25, 
                                                        uint16_t num_sector)
{
    W25Q128_StatusTypeDef status;
    uint8_t t_data[4];

    // Sector contains 16 pages, page contains 256 bytes.
    uint32_t mem_addr = num_sector*16*256;

    status = W25Q128_WriteEnable(w25);
    if (status != W25Q128_SUCCESS)
        return W25Q128_ERROR;
    
    t_data[0] = INST_SECTOR_ERASE_4KB;
    t_data[1] = (mem_addr >> 16) & 0xFF; // MSB of a 24-bit memory address
    t_data[2] = (mem_addr >> 8) & 0xFF;
    t_data[3] = (mem_addr) & 0xFF;       // LSB of a 24-bit memory address

    W25Q128_ChipSelect(w25);
    W25Q128_SPIWrite(w25, t_data, sizeof(t_data), W25Q128_SPI_TIMEOUT_MS);
    W25Q128_ChipDeselect(w25);

    // TODO: Check here for BUSY bit in Status Register
    if (W25Q128_CheckBUSY(w25) == W25Q128_READY)
    {
        status = W25Q128_WriteDisable(w25);
        if (status != W25Q128_SUCCESS)
            return W25Q128_ERROR;
    } else {
        return W25Q128_ERROR;
    }

    return W25Q128_SUCCESS;
}

uint8_t W25Q128_ReadStatusRegister(W25Q128_TypeDef *w25)
{
    uint8_t t_data = INST_READ_STATUS_REG_1;
    uint8_t status_val = 0;

    W25Q128_ChipSelect(w25);
    W25Q128_SPIWrite(w25, &t_data, sizeof(uint8_t), W25Q128_SPI_TIMEOUT_MS);
    W25Q128_SPIRead(w25, &status_val, sizeof(uint8_t), W25Q128_SPI_TIMEOUT_MS);
    W25Q128_ChipDeselect(w25);

    return status_val;
}

W25Q128_StatusTypeDef W25Q128_CheckBUSY(W25Q128_TypeDef *w25)
{
    uint32_t current_time = HAL_GetTick();
    while(W25Q128_ReadStatusRegister(w25) & 0x01)
    {
        if ((HAL_GetTick() - current_time) > W25Q128_RECOVERY_TIMEOUT_MS)
        {
            return W25Q128_ERROR_TIMEOUT;
        }
        W25Q128_DelayMs(1);
    }
    return W25Q128_READY;
}

W25Q128_StatusTypeDef W25Q128_WritePage(W25Q128_TypeDef *w25, uint32_t page, 
                                        uint16_t offset, uint32_t data_size, 
                                        uint8_t *data)
{
    uint8_t t_data[266];
    uint32_t index = 0;

    // Position of the data - track data in the data pointer
    uint32_t data_position = 0;
    
    // Starting page number
    uint32_t start_page = page; 
    
    // Ending page number
    uint32_t end_page = start_page + ((data_size + offset - 1)/256); 

    // Number of pages to be written
    uint32_t num_pages = end_page - start_page + 1;

#if ERASE_BEFORE_PAGE_WRITE_AUTO
    uint16_t start_sector = start_page/16;
    uint16_t end_sector = end_page/16;
    uint16_t num_sectors = end_sector - start_sector + 1;
    for (int i = 0; i < num_sectors; i++)
    {
        W25Q128_EraseSector(w25, start_sector++);
    }
#endif

    // Writting the data
    for (uint32_t i = 0; i < num_pages; i++)
    {
        uint32_t mem_addr = (start_page * 256) + offset;
        uint16_t bytes_remaining = calculate_bytes_to_write(data_size, offset); 
        
        W25Q128_WriteEnable(w25);

        t_data[0] = INST_PAGE_PROGRAM;
        t_data[1] = (mem_addr >> 16) & 0xFF; 
        t_data[2] = (mem_addr >> 8) & 0xFF;
        t_data[3] = (mem_addr) & 0xFF;

        index = 4;

        uint16_t bytes_to_send = bytes_remaining + index;

        for (int i = 0; i < bytes_remaining; i++)
        {
            t_data[index++] = data[i + data_position];
        }

        W25Q128_ChipSelect(w25);

        W25Q128_SPIWrite(w25, t_data, bytes_to_send, W25Q128_SPI_TIMEOUT_MS);

        W25Q128_ChipDeselect(w25);
        
        start_page++;
        offset = 0;
        data_size = data_size - bytes_remaining;
        data_position = data_position + bytes_remaining;

        W25Q128_DelayMs(5);
        W25Q128_WriteDisable(w25);
    }

    return W25Q128_SUCCESS;
}

W25Q128_StatusTypeDef W25Q128_Write(W25Q128_TypeDef *w25, uint32_t page, 
                                    uint16_t offset, uint32_t size, 
                                    uint8_t *data)
{
    uint16_t start_sector = page/16;
    uint16_t end_sector = (page + ((size + offset - 1)/256))/16;
    uint16_t num_sectors = end_sector - start_sector + 1;

    uint8_t previous_data[W25Q128_SECTOR_SIZE];
    uint32_t sector_offset = ((page%16)*256) + offset;
    uint32_t data_index = 0;

    for (uint16_t i = 0; i < num_sectors; i++)
    {
        uint32_t start_page = start_sector*16;
        
        if (W25Q128_FastRead(w25, start_page, 0, 
                    W25Q128_SECTOR_SIZE, previous_data) != W25Q128_SUCCESS)
            return W25Q128_ERROR;
        
        uint16_t bytes_remaining = calculate_bytes_to_modify(size, offset);
        for (uint16_t i = 0; i < bytes_remaining; i++)
        {
            previous_data[i + sector_offset] = data[i + data_index];
        }
        
        W25Q128_StatusTypeDef status = W25Q128_WritePage(w25, page, 0, 
                                            W25Q128_SECTOR_SIZE, previous_data);
        if (status != W25Q128_SUCCESS)
            return W25Q128_ERROR;
        
        start_sector++;
        sector_offset = 0;
        data_index = data_index + bytes_remaining;
        size = size - bytes_remaining;
    }

    return W25Q128_SUCCESS;
}

/*************************** Static functions *********************************/
static uint32_t calculate_bytes_to_write(uint32_t size, uint16_t offset)
{
    if ((size + offset) < 256)
        return size; 
    else 
        return (256 - offset);
}

static uint32_t calculate_bytes_to_modify(uint32_t size, uint16_t offset)
{
    if ((size + offset) < 4096)
        return size;
    else 
        return (4096 - offset);
}
