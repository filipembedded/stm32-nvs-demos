#include "w25q128_lfs.h"
#include "w25q128_ll.h"

int w25q128_lfs_read(const struct lfs_config *c, lfs_block_t block,
                                lfs_off_t off, void *buffer, lfs_size_t size)
{
    W25Q128_TypeDef *w25 = (W25Q128_TypeDef *)c->context;

    uint32_t page = block * (c->block_size / W25Q128_PAGE_SIZE) + 
                                                    (off / W25Q128_PAGE_SIZE);

    uint32_t offset = off % W25Q128_PAGE_SIZE;

    W25Q128_StatusTypeDef status = W25Q128_FastRead(w25, page, offset, size,
                                                                        buffer);
    if (status != W25Q128_SUCCESS)
        return LFS_ERR_IO;
    
    return 0;
}

int w25q128_lfs_prog(const struct lfs_config *c, lfs_block_t block,
                            lfs_off_t off, const void *buffer, lfs_size_t size)
{
    W25Q128_TypeDef *w25 = (W25Q128_TypeDef *)c->context;

    uint32_t page = block * (c->block_size / W25Q128_PAGE_SIZE) + 
                                                    (off / W25Q128_PAGE_SIZE);

    uint32_t offset = off % W25Q128_PAGE_SIZE;

    W25Q128_StatusTypeDef status = W25Q128_Write(w25, page, offset, size, 
                                                                        buffer);
    if (status != W25Q128_SUCCESS)
        return LFS_ERR_IO;
    
    return 0;                                                           
}

int w25q128_lfs_erase(const struct lfs_config *c, lfs_block_t block)
{
    W25Q128_TypeDef *w25 = (W25Q128_TypeDef *)c->context;

    W25Q128_StatusTypeDef status = W25Q128_EraseSector(w25, block);

    if (status != W25Q128_SUCCESS)
        return LFS_ERR_IO;
    
    return 0;
}

int w25q128_lfs_sync(const struct lfs_config *c)
{
    // For now, nothing to sync
    return 0;
}