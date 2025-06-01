#ifndef W25Q128_H 
#define W25Q128_H

#include "w25q128_conf_ll.h"

#define ERASE_BEFORE_PAGE_WRITE_AUTO 0
#define W25Q128_SECTOR_SIZE 4096
#define W25Q128_PAGE_SIZE   256
#define W25Q128_SECTOR_COUNT 4096

typedef enum {
    W25Q128_SUCCESS = 0,
    W25Q128_ERROR = 1,
    W25Q128_READY = 2,
    W25Q128_BUSY = 3,
    W25Q128_ERROR_TIMEOUT = 4,
} W25Q128_StatusTypeDef;

typedef enum {
    INST_WRITE_ENABLE = 0x06,
    INST_VOLATILE_SR_WRITE_ENABLE = 0x50,
    INST_WRITE_DISABLE = 0x04,

    INST_RELEASE_POWER_DOWN_ID = 0xAB,
    INST_MANUFACTURER_DEVICE_ID = 0x90,
    INST_JEDEC_ID = 0x9F,
    INST_READ_UNIQUE_ID = 0x4B,

    INST_READ_DATA = 0x03,
    INST_FAST_READ = 0x0B,
        
    INST_PAGE_PROGRAM = 0x02,

    INST_SECTOR_ERASE_4KB = 0x20,
    INST_BLOCK_ERASE_32KB = 0x52,
    INST_BLOCK_ERASE_64KB = 0xD8,
    INST_CHIP_ERASE = 0xC7, // 0x60

    INST_READ_STATUS_REG_1 = 0x05,
    INST_WRITE_STATUS_REG_1 = 0x01,
    INST_READ_STATUS_REG_2 = 0x35,
    INST_WRITE_STATUS_REG_2 = 0x31,
    INST_READ_STATUS_REG_3 = 0x15,
    INST_WRITE_STATUS_REG_3 = 0x11,

    INST_READ_SFDP_REG = 0x5A,
    INST_ERASE_SECURITY_REG = 0x44,
    INST_PROGRAM_SECURITY_REG = 0x42,
    INST_READ_SECURITY_REG = 0x48,

    INST_GLOBAL_BLOCK_LOCK = 0x7E,
    INST_GLOBAL_BLOCK_UNLOCK = 0x98,
    INST_READ_BLOCK_LOCK = 0x3D,
    INST_INDIVIDUAL_BLOCK_LOCK = 0x36,
    INST_INDIVIDUAL_BLOCK_UNLOCK = 0x39,

    INST_ERASE_PROGRAM_SUSPEND = 0x75,
    INST_ERASE_PROGRAM_RESUME = 0x7A,
    INST_POWER_DOWN = 0xB9,
        
    INST_ENABLE_RESET = 0x66,
    INST_RESET_DEVICE = 0x99,
} W25Q128_InstructionTypeDef;

typedef enum {
    ID_RELEASE_POWER_DOWN = 0,
    ID_MANUFACTURER_DEVICE = 1,
    ID_JEDEC = 2,
    ID_READ_UNIQUE = 3,
} W25Q128_ID_TypeDef;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
} W25Q128_TypeDef;

void W25Q128_ChipSelect(W25Q128_TypeDef *w25q128);

void W25Q128_ChipDeselect(W25Q128_TypeDef *w25q128);

void W25Q128_SPIWrite(W25Q128_TypeDef *w25q128, uint8_t *data, uint16_t len, 
                                                            uint32_t timeout);

void W25Q128_SPIRead(W25Q128_TypeDef *w25q128, uint8_t *data, uint16_t len, 
                                                            uint32_t timeout);

void W25Q128_DelayMs(uint32_t delay_ms);

void W25Q128_Reset(W25Q128_TypeDef *w25q128);

uint32_t W25Q128_ReadID(W25Q128_TypeDef *w25q128, W25Q128_ID_TypeDef id);

W25Q128_StatusTypeDef W25Q128_Read(W25Q128_TypeDef *w25,
                                    uint32_t start_page,
                                    uint8_t offset,
                                    uint32_t size,
                                    uint8_t *r_data);


W25Q128_StatusTypeDef W25Q128_FastRead(W25Q128_TypeDef *w25,
                                        uint32_t start_page,
                                        uint8_t offset,
                                        uint32_t size,
                                        uint8_t *r_data);

W25Q128_StatusTypeDef W25Q128_WriteEnable(W25Q128_TypeDef *w25);

W25Q128_StatusTypeDef W25Q128_WriteDisable(W25Q128_TypeDef *w25);

W25Q128_StatusTypeDef W25Q128_EraseSector(W25Q128_TypeDef *w25, 
                                                        uint16_t num_sector);

uint8_t W25Q128_ReadStatusRegister(W25Q128_TypeDef *w25);

W25Q128_StatusTypeDef W25Q128_CheckBUSY(W25Q128_TypeDef *w25);

W25Q128_StatusTypeDef W25Q128_WritePage(W25Q128_TypeDef *w25, uint32_t page, 
                                        uint16_t offset, uint32_t data_size, 
                                        uint8_t *data);

W25Q128_StatusTypeDef W25Q128_Write(W25Q128_TypeDef *w25, uint32_t page, 
                                    uint16_t offset, uint32_t size, 
                                    uint8_t *data);


#endif 