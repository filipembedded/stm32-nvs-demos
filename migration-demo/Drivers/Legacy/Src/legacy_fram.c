#include "legacy_fram.h"
#include "w25q128_ll.h"
#include "w25q128_lfs.h"
#include "main.h"

#include <stdio.h>

extern SPI_HandleTypeDef hspi1;


static lfs_t lfs;
static W25Q128_TypeDef w25;
static const struct lfs_config cfg = {
    .context = (void *)&w25,
    .read = w25q128_lfs_read,
    .prog = w25q128_lfs_prog,
    .erase = w25q128_lfs_erase,
    .sync = w25q128_lfs_sync,
    .read_size = 256,
    .prog_size = 256,
    .block_size = W25Q128_SECTOR_SIZE,
    .block_count = W25Q128_SECTOR_COUNT,
    .cache_size = 256,
    .lookahead_size = 16,
    .block_cycles = 500,
};

static void addr_to_filename(uint16_t address, char *name) {
    snprintf(name, 8, "%05u", (unsigned)address);
}

bool InitFramMemory(void) {
    w25.cs_port = GPIOA;
    w25.cs_pin  = GPIO_PIN_4;
    w25.hspi    = &hspi1;

    int err = lfs_mount(&lfs, &cfg);
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }
    return true;
}

bool ReadFramMemoryWithoutDebug(uint16_t address, uint8_t *buff, uint16_t len) {
    char name[8];
    lfs_file_t file;
    addr_to_filename(address, name);

    if (lfs_file_open(&lfs, &file, name, LFS_O_RDONLY) < 0)
        return false;

    lfs_ssize_t r = lfs_file_read(&lfs, &file, buff, len);
    lfs_file_close(&lfs, &file);
    return (r == (lfs_ssize_t)len);
}

bool WriteFramMemoryWithoutDebug(uint16_t address, uint8_t *buff, uint16_t len) {
    char name[8];
    lfs_file_t file;
    addr_to_filename(address, name);

    if (lfs_file_open(&lfs, &file, name,
                      LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) < 0)
        return false;

    lfs_ssize_t w = lfs_file_write(&lfs, &file, buff, len);
    lfs_file_close(&lfs, &file);
    return (w == (lfs_ssize_t)len);
}
