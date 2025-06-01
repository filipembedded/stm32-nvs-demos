#ifndef NOR_FLASH_H
#define NOR_FLASH_H

#include "lfs.h"


int w25q128_lfs_read(const struct lfs_config *c, lfs_block_t block,
                                lfs_off_t off, void *buffer, lfs_size_t size);

int w25q128_lfs_prog(const struct lfs_config *c, lfs_block_t block,
                            lfs_off_t off, const void *buffer, lfs_size_t size);

int w25q128_lfs_erase(const struct lfs_config *c, lfs_block_t block);

int w25q128_lfs_sync(const struct lfs_config *c);

#endif