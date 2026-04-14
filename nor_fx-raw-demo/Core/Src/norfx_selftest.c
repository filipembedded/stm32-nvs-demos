#include "norfx_selftest.h"

#include "stm32f4xx_hal.h"
#include "norfx_test_log.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define NORFX_TEST_WEL_MASK          0x02u
#define NORFX_TEST_BASE_SECTOR       0u
#define NORFX_TEST_SECOND_SECTOR     1u
#define NORFX_TEST_BASE_PAGE         (NORFX_TEST_BASE_SECTOR * 16u)
#define NORFX_TEST_SECOND_BASE_PAGE  (NORFX_TEST_SECOND_SECTOR * 16u)
#define NORFX_TEST_LAST_SECTOR       (NORFX_BLOCK_COUNT - 1u)

typedef bool (*norfx_test_function_t)(struct norfx_device *dev);

typedef struct {
    const char *name;
    norfx_test_function_t function;
} norfx_test_case_t;

typedef struct {
    const char *name;
    uint32_t operations;
    uint32_t total_us;
    uint32_t max_us;
    uint32_t total_bytes;
} norfx_timing_stats_t;

static uint8_t norfx_test_sector_scratch[NORFX_SECTOR_SIZE];
static uint8_t norfx_test_readback[NORFX_SECTOR_SIZE];
static norfx_timing_stats_t norfx_read_stats;
static norfx_timing_stats_t norfx_write_stats;
static norfx_timing_stats_t norfx_erase_stats;
static bool norfx_cycle_counter_ready;

static bool norfx_expect_status(const char *label,
                                enum norfx_status actual,
                                enum norfx_status expected);
static bool norfx_expect_true(const char *label, bool condition);
static bool norfx_expect_buffer_value(const char *label,
                                      const uint8_t *buffer,
                                      size_t length,
                                      uint8_t expected_value);
static bool norfx_expect_buffer_equal(const char *label,
                                      const uint8_t *expected,
                                      const uint8_t *actual,
                                      size_t length);
static void norfx_fill_pattern(uint8_t *buffer, size_t length, uint8_t seed);
static bool norfx_test_reset_and_status(struct norfx_device *dev);
static bool norfx_test_ids(struct norfx_device *dev);
static bool norfx_test_write_latch(struct norfx_device *dev);
static bool norfx_test_erase_and_blank_reads(struct norfx_device *dev);
static bool norfx_test_page_program_and_readback(struct norfx_device *dev);
static bool norfx_test_read_across_page_boundary(struct norfx_device *dev);
static bool norfx_test_nor_programming_behavior(struct norfx_device *dev);
static bool norfx_test_write_preserves_neighbors(struct norfx_device *dev);
static bool norfx_test_write_across_sector_boundary(struct norfx_device *dev);
static bool norfx_test_full_sector_sweep(struct norfx_device *dev);
static void norfx_timing_init(void);
static uint32_t norfx_timing_capture_start(void);
static uint32_t norfx_timing_elapsed_us(uint32_t start_mark);
static void norfx_timing_record(norfx_timing_stats_t *stats, uint32_t elapsed_us, uint32_t bytes);
static void norfx_timing_print_summary(const norfx_timing_stats_t *stats);
static enum norfx_status norfx_timed_read(struct norfx_device *dev,
                                          uint32_t start_page,
                                          uint8_t offset,
                                          uint32_t size,
                                          uint8_t *rx_buf);
static enum norfx_status norfx_timed_fast_read(struct norfx_device *dev,
                                               uint32_t start_page,
                                               uint8_t offset,
                                               uint32_t size,
                                               uint8_t *rx_buf);
static enum norfx_status norfx_timed_erase_sector(struct norfx_device *dev, uint16_t sector);
static enum norfx_status norfx_timed_page_program(struct norfx_device *dev,
                                                  uint32_t page,
                                                  uint16_t offset,
                                                  uint32_t size,
                                                  uint8_t *data);
static enum norfx_status norfx_timed_write(struct norfx_device *dev,
                                           uint32_t page,
                                           uint16_t offset,
                                           uint32_t size,
                                           uint8_t *data,
                                           uint8_t *scratch_buf);

norfx_selftest_summary_t norfx_selftest_run(struct norfx_device *dev)
{
    static const norfx_test_case_t test_cases[] = {
        {"reset_and_status", norfx_test_reset_and_status},
        {"read_ids", norfx_test_ids},
        {"write_latch", norfx_test_write_latch},
        {"erase_and_blank_reads", norfx_test_erase_and_blank_reads},
        {"page_program_and_readback", norfx_test_page_program_and_readback},
        {"read_across_page_boundary", norfx_test_read_across_page_boundary},
        {"nor_programming_behavior", norfx_test_nor_programming_behavior},
        {"write_preserves_neighbors", norfx_test_write_preserves_neighbors},
        {"write_across_sector_boundary", norfx_test_write_across_sector_boundary},
        {"full_sector_sweep", norfx_test_full_sector_sweep},
    };

    norfx_selftest_summary_t summary = {0};

    norfx_timing_init();

    norfx_test_log_printf("\r\n[norfx] self-test start\r\n");
    norfx_test_log_printf("[norfx] FULL FLASH TEST enabled\r\n");
    norfx_test_log_printf("[norfx] destructive range: sectors 0..%lu\r\n",
                          (unsigned long)NORFX_TEST_LAST_SECTOR);
    norfx_test_log_printf("[norfx] representative boundary sectors: %lu and %lu\r\n",
                          (unsigned long)NORFX_TEST_BASE_SECTOR,
                          (unsigned long)NORFX_TEST_SECOND_SECTOR);

    for (size_t index = 0; index < (sizeof(test_cases) / sizeof(test_cases[0])); index++)
    {
        summary.total++;
        norfx_test_log_printf("[RUN ] %s\r\n", test_cases[index].name);

        if (test_cases[index].function(dev))
        {
            summary.passed++;
            norfx_test_log_printf("[PASS] %s\r\n", test_cases[index].name);
        }
        else
        {
            summary.failed++;
            norfx_test_log_printf("[FAIL] %s\r\n", test_cases[index].name);
        }
    }

    norfx_test_log_printf("[done] total=%lu passed=%lu failed=%lu\r\n",
                          (unsigned long)summary.total,
                          (unsigned long)summary.passed,
                          (unsigned long)summary.failed);
    norfx_timing_print_summary(&norfx_read_stats);
    norfx_timing_print_summary(&norfx_write_stats);
    norfx_timing_print_summary(&norfx_erase_stats);

    return summary;
}

static void norfx_timing_init(void)
{
    norfx_read_stats = (norfx_timing_stats_t){ .name = "read" };
    norfx_write_stats = (norfx_timing_stats_t){ .name = "write" };
    norfx_erase_stats = (norfx_timing_stats_t){ .name = "erase" };

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0u;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    norfx_cycle_counter_ready = ((DWT->CTRL & DWT_CTRL_NOCYCCNT_Msk) == 0u);
}

static uint32_t norfx_timing_capture_start(void)
{
    if (norfx_cycle_counter_ready)
    {
        return DWT->CYCCNT;
    }

    return HAL_GetTick();
}

static uint32_t norfx_timing_elapsed_us(uint32_t start_mark)
{
    if (norfx_cycle_counter_ready)
    {
        uint32_t elapsed_cycles = DWT->CYCCNT - start_mark;
        if (SystemCoreClock == 0u)
        {
            return 0u;
        }

        return (uint32_t)(((uint64_t)elapsed_cycles * 1000000ull) / (uint64_t)SystemCoreClock);
    }

    return (HAL_GetTick() - start_mark) * 1000u;
}

static void norfx_timing_record(norfx_timing_stats_t *stats, uint32_t elapsed_us, uint32_t bytes)
{
    if (stats == NULL)
    {
        return;
    }

    stats->operations++;
    if ((UINT32_MAX - stats->total_us) < elapsed_us)
    {
        stats->total_us = UINT32_MAX;
    }
    else
    {
        stats->total_us += elapsed_us;
    }

    if ((UINT32_MAX - stats->total_bytes) < bytes)
    {
        stats->total_bytes = UINT32_MAX;
    }
    else
    {
        stats->total_bytes += bytes;
    }

    if (elapsed_us > stats->max_us)
    {
        stats->max_us = elapsed_us;
    }
}

static void norfx_timing_print_summary(const norfx_timing_stats_t *stats)
{
    uint32_t average_us = 0u;
    uint32_t kib_per_s = 0u;

    if (stats == NULL || stats->operations == 0u)
    {
        return;
    }

    average_us = (uint32_t)(stats->total_us / stats->operations);
    if (stats->total_us != 0u)
    {
        kib_per_s = (uint32_t)(((uint64_t)stats->total_bytes * 1000000ull) /
                               ((uint64_t)stats->total_us * 1024ull));
    }

    norfx_test_log_printf("[perf] %s ops=%lu bytes=%lu total_us=%lu avg_us=%lu max_us=%lu rate=%lu KiB/s\r\n",
                          stats->name,
                          (unsigned long)stats->operations,
                          (unsigned long)stats->total_bytes,
                          (unsigned long)stats->total_us,
                          (unsigned long)average_us,
                          (unsigned long)stats->max_us,
                          (unsigned long)kib_per_s);
}

static enum norfx_status norfx_timed_read(struct norfx_device *dev,
                                          uint32_t start_page,
                                          uint8_t offset,
                                          uint32_t size,
                                          uint8_t *rx_buf)
{
    uint32_t start_mark = norfx_timing_capture_start();
    enum norfx_status status = norfx_read(dev, start_page, offset, size, rx_buf);
    if (status == NORFX_SUCCESS)
    {
        norfx_timing_record(&norfx_read_stats, norfx_timing_elapsed_us(start_mark), size);
    }

    return status;
}

static enum norfx_status norfx_timed_fast_read(struct norfx_device *dev,
                                               uint32_t start_page,
                                               uint8_t offset,
                                               uint32_t size,
                                               uint8_t *rx_buf)
{
    uint32_t start_mark = norfx_timing_capture_start();
    enum norfx_status status = norfx_fast_read(dev, start_page, offset, size, rx_buf);
    if (status == NORFX_SUCCESS)
    {
        norfx_timing_record(&norfx_read_stats, norfx_timing_elapsed_us(start_mark), size);
    }

    return status;
}

static enum norfx_status norfx_timed_erase_sector(struct norfx_device *dev, uint16_t sector)
{
    uint32_t start_mark = norfx_timing_capture_start();
    enum norfx_status status = norfx_erase_sector(dev, sector);
    if (status == NORFX_SUCCESS)
    {
        norfx_timing_record(&norfx_erase_stats, norfx_timing_elapsed_us(start_mark), NORFX_SECTOR_SIZE);
    }

    return status;
}

static enum norfx_status norfx_timed_page_program(struct norfx_device *dev,
                                                  uint32_t page,
                                                  uint16_t offset,
                                                  uint32_t size,
                                                  uint8_t *data)
{
    uint32_t start_mark = norfx_timing_capture_start();
    enum norfx_status status = norfx_page_program(dev, page, offset, size, data);
    if (status == NORFX_SUCCESS)
    {
        norfx_timing_record(&norfx_write_stats, norfx_timing_elapsed_us(start_mark), size);
    }

    return status;
}

static enum norfx_status norfx_timed_write(struct norfx_device *dev,
                                           uint32_t page,
                                           uint16_t offset,
                                           uint32_t size,
                                           uint8_t *data,
                                           uint8_t *scratch_buf)
{
    uint32_t start_mark = norfx_timing_capture_start();
    enum norfx_status status = norfx_write(dev, page, offset, size, data, scratch_buf);
    if (status == NORFX_SUCCESS)
    {
        norfx_timing_record(&norfx_write_stats, norfx_timing_elapsed_us(start_mark), size);
    }

    return status;
}

static bool norfx_expect_status(const char *label,
                                enum norfx_status actual,
                                enum norfx_status expected)
{
    if (actual == expected)
    {
        return true;
    }

    norfx_test_log_printf("[step] %s status=%u expected=%u\r\n",
                          label,
                          (unsigned int)actual,
                          (unsigned int)expected);
    return false;
}

static bool norfx_expect_true(const char *label, bool condition)
{
    if (condition)
    {
        return true;
    }

    norfx_test_log_printf("[step] %s condition failed\r\n", label);
    return false;
}

static bool norfx_expect_buffer_value(const char *label,
                                      const uint8_t *buffer,
                                      size_t length,
                                      uint8_t expected_value)
{
    if (buffer == NULL)
    {
        norfx_test_log_printf("[step] %s null buffer\r\n", label);
        return false;
    }

    for (size_t index = 0; index < length; index++)
    {
        if (buffer[index] != expected_value)
        {
            norfx_test_log_printf("[step] %s mismatch index=%lu value=0x%02X expected=0x%02X\r\n",
                                  label,
                                  (unsigned long)index,
                                  buffer[index],
                                  expected_value);
            return false;
        }
    }

    return true;
}

static bool norfx_expect_buffer_equal(const char *label,
                                      const uint8_t *expected,
                                      const uint8_t *actual,
                                      size_t length)
{
    if (expected == NULL || actual == NULL)
    {
        norfx_test_log_printf("[step] %s null compare buffer\r\n", label);
        return false;
    }

    for (size_t index = 0; index < length; index++)
    {
        if (expected[index] != actual[index])
        {
            norfx_test_log_printf("[step] %s mismatch index=%lu actual=0x%02X expected=0x%02X\r\n",
                                  label,
                                  (unsigned long)index,
                                  actual[index],
                                  expected[index]);
            return false;
        }
    }

    return true;
}

static void norfx_fill_pattern(uint8_t *buffer, size_t length, uint8_t seed)
{
    if (buffer == NULL)
    {
        return;
    }

    for (size_t index = 0; index < length; index++)
    {
        buffer[index] = (uint8_t)(seed + (uint8_t)(index * 13u));
    }
}

static bool norfx_test_reset_and_status(struct norfx_device *dev)
{
    uint8_t status_register = 0u;

    if (!norfx_expect_status("reset", norfx_reset(dev), NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_status_reg", norfx_read_status_reg(dev, &status_register), NORFX_SUCCESS))
    {
        return false;
    }

    return norfx_expect_true("status_wip_clear", (status_register & NORFX_WIP_MASK) == 0u);
}

static bool norfx_test_ids(struct norfx_device *dev)
{
    uint32_t jedec_id = 0u;
    uint32_t manufacturer_device_id = 0u;
    uint32_t release_power_down_id = 0u;
    uint32_t unique_id_lsb = 0u;

    if (!norfx_expect_status("read_jedec_id", norfx_read_id(dev, ID_JEDEC, &jedec_id), NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_manufacturer_device_id",
                             norfx_read_id(dev, ID_MANUFACTURER_DEVICE, &manufacturer_device_id),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_release_power_down_id",
                             norfx_read_id(dev, ID_RELEASE_POWER_DOWN, &release_power_down_id),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_unique_id",
                             norfx_read_id(dev, ID_READ_UNIQUE, &unique_id_lsb),
                             NORFX_SUCCESS))
    {
        return false;
    }

    norfx_test_log_printf("[info] jedec=0x%06lX mfr_dev=0x%04lX release=0x%02lX uid_lsb=0x%08lX\r\n",
                          (unsigned long)jedec_id,
                          (unsigned long)manufacturer_device_id,
                          (unsigned long)release_power_down_id,
                          (unsigned long)unique_id_lsb);

    if (!norfx_expect_true("jedec_manufacturer", (jedec_id & 0xFF0000u) == 0xEF0000u))
    {
        return false;
    }

    if (!norfx_expect_true("jedec_capacity", (jedec_id & 0x0000FFu) == 0x18u))
    {
        return false;
    }

    if (!norfx_expect_true("manufacturer_device_prefix", (manufacturer_device_id & 0xFF00u) == 0xEF00u))
    {
        return false;
    }

    if (!norfx_expect_true("release_power_down_nonzero",
                           (release_power_down_id != 0u) && (release_power_down_id != 0xFFu)))
    {
        return false;
    }

    return norfx_expect_true("unique_id_nontrivial",
                             (unique_id_lsb != 0u) && (unique_id_lsb != 0xFFFFFFFFu));
}

static bool norfx_test_write_latch(struct norfx_device *dev)
{
    uint8_t status_register = 0u;

    if (!norfx_expect_status("write_enable", norfx_write_enable(dev), NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("status_after_write_enable",
                             norfx_read_status_reg(dev, &status_register),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_true("wel_set", (status_register & NORFX_TEST_WEL_MASK) != 0u))
    {
        return false;
    }

    if (!norfx_expect_status("write_disable", norfx_write_disable(dev), NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("status_after_write_disable",
                             norfx_read_status_reg(dev, &status_register),
                             NORFX_SUCCESS))
    {
        return false;
    }

    return norfx_expect_true("wel_clear", (status_register & NORFX_TEST_WEL_MASK) == 0u);
}

static bool norfx_test_erase_and_blank_reads(struct norfx_device *dev)
{
    memset(norfx_test_readback, 0x00, 128u);

    if (!norfx_expect_status("erase_blank_sector", norfx_timed_erase_sector(dev, NORFX_TEST_BASE_SECTOR), NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_blank_sector",
                             norfx_timed_read(dev, NORFX_TEST_BASE_PAGE, 0u, 128u, norfx_test_readback),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_buffer_value("blank_read_is_ff", norfx_test_readback, 128u, 0xFFu))
    {
        return false;
    }

    memset(norfx_test_readback, 0x00, 128u);

    if (!norfx_expect_status("fast_read_blank_sector",
                             norfx_timed_fast_read(dev, NORFX_TEST_BASE_PAGE, 0u, 128u, norfx_test_readback),
                             NORFX_SUCCESS))
    {
        return false;
    }

    return norfx_expect_buffer_value("blank_fast_read_is_ff", norfx_test_readback, 128u, 0xFFu);
}

static bool norfx_test_page_program_and_readback(struct norfx_device *dev)
{
    uint8_t expected[48];

    norfx_fill_pattern(expected, sizeof(expected), 0x21u);
    memset(norfx_test_readback, 0x00, sizeof(expected));

    if (!norfx_expect_status("erase_for_page_program", norfx_timed_erase_sector(dev, NORFX_TEST_BASE_SECTOR), NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("page_program",
                             norfx_timed_page_program(dev, NORFX_TEST_BASE_PAGE, 32u, sizeof(expected), expected),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_programmed_page",
                             norfx_timed_read(dev, NORFX_TEST_BASE_PAGE, 32u, sizeof(expected), norfx_test_readback),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_buffer_equal("page_program_readback", expected, norfx_test_readback, sizeof(expected)))
    {
        return false;
    }

    memset(norfx_test_readback, 0x00, sizeof(expected));

    if (!norfx_expect_status("fast_read_programmed_page",
                             norfx_timed_fast_read(dev, NORFX_TEST_BASE_PAGE, 32u, sizeof(expected), norfx_test_readback),
                             NORFX_SUCCESS))
    {
        return false;
    }

    return norfx_expect_buffer_equal("page_program_fast_readback", expected, norfx_test_readback, sizeof(expected));
}

static bool norfx_test_read_across_page_boundary(struct norfx_device *dev)
{
    uint8_t tail_pattern[8];
    uint8_t head_pattern[8];
    uint8_t expected[16];

    norfx_fill_pattern(tail_pattern, sizeof(tail_pattern), 0x30u);
    norfx_fill_pattern(head_pattern, sizeof(head_pattern), 0x70u);
    memcpy(expected, tail_pattern, sizeof(tail_pattern));
    memcpy(expected + sizeof(tail_pattern), head_pattern, sizeof(head_pattern));
    memset(norfx_test_readback, 0x00, sizeof(expected));

    if (!norfx_expect_status("erase_for_boundary_read", norfx_timed_erase_sector(dev, NORFX_TEST_BASE_SECTOR), NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("page_program_tail",
                             norfx_timed_page_program(dev, NORFX_TEST_BASE_PAGE, 248u, sizeof(tail_pattern), tail_pattern),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("page_program_head",
                             norfx_timed_page_program(dev, NORFX_TEST_BASE_PAGE + 1u, 0u, sizeof(head_pattern), head_pattern),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_across_boundary",
                             norfx_timed_read(dev, NORFX_TEST_BASE_PAGE, 248u, sizeof(expected), norfx_test_readback),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_buffer_equal("boundary_read_compare", expected, norfx_test_readback, sizeof(expected)))
    {
        return false;
    }

    memset(norfx_test_readback, 0x00, sizeof(expected));

    if (!norfx_expect_status("fast_read_across_boundary",
                             norfx_timed_fast_read(dev, NORFX_TEST_BASE_PAGE, 248u, sizeof(expected), norfx_test_readback),
                             NORFX_SUCCESS))
    {
        return false;
    }

    return norfx_expect_buffer_equal("boundary_fast_read_compare", expected, norfx_test_readback, sizeof(expected));
}

static bool norfx_test_nor_programming_behavior(struct norfx_device *dev)
{
    static const uint8_t first_pass[4] = {0xF0u, 0xF0u, 0xF0u, 0xF0u};
    static const uint8_t second_pass[4] = {0x0Fu, 0x0Fu, 0x0Fu, 0x0Fu};
    static const uint8_t expected[4] = {0x00u, 0x00u, 0x00u, 0x00u};

    memset(norfx_test_readback, 0x00, sizeof(expected));

    if (!norfx_expect_status("erase_for_nor_behavior", norfx_timed_erase_sector(dev, NORFX_TEST_BASE_SECTOR), NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("page_program_first_pass",
                             norfx_timed_page_program(dev, NORFX_TEST_BASE_PAGE, 0u, sizeof(first_pass), (uint8_t *)first_pass),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("page_program_second_pass",
                             norfx_timed_page_program(dev, NORFX_TEST_BASE_PAGE, 0u, sizeof(second_pass), (uint8_t *)second_pass),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_nor_behavior",
                             norfx_timed_read(dev, NORFX_TEST_BASE_PAGE, 0u, sizeof(expected), norfx_test_readback),
                             NORFX_SUCCESS))
    {
        return false;
    }

    return norfx_expect_buffer_equal("nor_behavior_compare", expected, norfx_test_readback, sizeof(expected));
}

static bool norfx_test_write_preserves_neighbors(struct norfx_device *dev)
{
    uint8_t page_fill[NORFX_PAGE_SIZE];
    uint8_t patch[6] = {0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66u};
    uint8_t before_byte = 0u;
    uint8_t after_byte = 0u;

    memset(page_fill, 0xAB, sizeof(page_fill));
    memset(norfx_test_readback, 0x00, sizeof(patch));

    if (!norfx_expect_status("erase_for_write_preserve", norfx_timed_erase_sector(dev, NORFX_TEST_BASE_SECTOR), NORFX_SUCCESS))
    {
        return false;
    }

    for (uint32_t page_index = 0u; page_index < 16u; page_index++)
    {
        if (!norfx_expect_status("preload_page",
                                 norfx_timed_page_program(dev,
                                                          NORFX_TEST_BASE_PAGE + page_index,
                                                          0u,
                                                          sizeof(page_fill),
                                                          page_fill),
                                 NORFX_SUCCESS))
        {
            return false;
        }
    }

    if (!norfx_expect_status("write_patch",
                             norfx_timed_write(dev,
                                               NORFX_TEST_BASE_PAGE,
                                               10u,
                                               sizeof(patch),
                                               patch,
                                               norfx_test_sector_scratch),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_patch",
                             norfx_timed_read(dev, NORFX_TEST_BASE_PAGE, 10u, sizeof(patch), norfx_test_readback),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_buffer_equal("patch_compare", patch, norfx_test_readback, sizeof(patch)))
    {
        return false;
    }

    if (!norfx_expect_status("read_before_patch",
                             norfx_timed_read(dev, NORFX_TEST_BASE_PAGE, 9u, 1u, &before_byte),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_after_patch",
                             norfx_timed_read(dev, NORFX_TEST_BASE_PAGE, 16u, 1u, &after_byte),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_true("before_patch_preserved", before_byte == 0xABu))
    {
        return false;
    }

    return norfx_expect_true("after_patch_preserved", after_byte == 0xABu);
}

static bool norfx_test_write_across_sector_boundary(struct norfx_device *dev)
{
    uint8_t payload[8];
    uint8_t before_boundary = 0u;
    uint8_t after_boundary = 0u;

    norfx_fill_pattern(payload, sizeof(payload), 0x90u);
    memset(norfx_test_readback, 0x00, sizeof(payload));

    if (!norfx_expect_status("erase_first_cross_sector", norfx_timed_erase_sector(dev, NORFX_TEST_BASE_SECTOR), NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("erase_second_cross_sector", norfx_timed_erase_sector(dev, NORFX_TEST_SECOND_SECTOR), NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("write_cross_sector",
                             norfx_timed_write(dev,
                                               NORFX_TEST_BASE_PAGE + 15u,
                                               252u,
                                               sizeof(payload),
                                               payload,
                                               norfx_test_sector_scratch),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_cross_sector",
                             norfx_timed_read(dev, NORFX_TEST_BASE_PAGE + 15u, 252u, sizeof(payload), norfx_test_readback),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_buffer_equal("cross_sector_compare", payload, norfx_test_readback, sizeof(payload)))
    {
        return false;
    }

    if (!norfx_expect_status("read_before_cross_sector",
                             norfx_timed_read(dev, NORFX_TEST_BASE_PAGE + 15u, 251u, 1u, &before_boundary),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_status("read_after_cross_sector",
                             norfx_timed_read(dev, NORFX_TEST_SECOND_BASE_PAGE, 4u, 1u, &after_boundary),
                             NORFX_SUCCESS))
    {
        return false;
    }

    if (!norfx_expect_true("before_cross_sector_ff", before_boundary == 0xFFu))
    {
        return false;
    }

    return norfx_expect_true("after_cross_sector_ff", after_boundary == 0xFFu);
}

static bool norfx_test_full_sector_sweep(struct norfx_device *dev)
{
    for (uint32_t sector = 0u; sector < NORFX_BLOCK_COUNT; sector++)
    {
        uint32_t base_page = sector * 16u;
        uint32_t sector_erase_us = 0u;
        uint32_t sector_blank_read_us = 0u;
        uint32_t sector_program_us = 0u;
        uint32_t sector_verify_read_us = 0u;
        uint32_t sector_total_us = 0u;
        uint32_t start_mark = 0u;
        uint8_t seed = (uint8_t)(0xA5u ^
                                 (uint8_t)(sector & 0xFFu) ^
                                 (uint8_t)((sector >> 8) & 0xFFu) ^
                                 (uint8_t)((sector >> 16) & 0xFFu));

        start_mark = norfx_timing_capture_start();
        if (norfx_erase_sector(dev, (uint16_t)sector) != NORFX_SUCCESS)
        {
            norfx_test_log_printf("[full] erase failed sector=%lu\r\n", (unsigned long)sector);
            return false;
        }
        sector_erase_us = norfx_timing_elapsed_us(start_mark);
        norfx_timing_record(&norfx_erase_stats, sector_erase_us, NORFX_SECTOR_SIZE);

        memset(norfx_test_readback, 0x00, sizeof(norfx_test_readback));
        start_mark = norfx_timing_capture_start();
        if (norfx_fast_read(dev, base_page, 0u, NORFX_SECTOR_SIZE, norfx_test_readback) != NORFX_SUCCESS)
        {
            norfx_test_log_printf("[full] blank fast-read failed sector=%lu\r\n", (unsigned long)sector);
            return false;
        }
        sector_blank_read_us = norfx_timing_elapsed_us(start_mark);
        norfx_timing_record(&norfx_read_stats, sector_blank_read_us, NORFX_SECTOR_SIZE);

        if (!norfx_expect_buffer_value("full_blank_sector_ff",
                                       norfx_test_readback,
                                       NORFX_SECTOR_SIZE,
                                       0xFFu))
        {
            norfx_test_log_printf("[full] blank verify failed sector=%lu\r\n", (unsigned long)sector);
            return false;
        }

        norfx_fill_pattern(norfx_test_sector_scratch, NORFX_SECTOR_SIZE, seed);
        for (uint32_t page_index = 0u; page_index < 16u; page_index++)
        {
            start_mark = norfx_timing_capture_start();
            if (norfx_page_program(dev,
                                   base_page + page_index,
                                   0u,
                                   NORFX_PAGE_SIZE,
                                   &norfx_test_sector_scratch[page_index * NORFX_PAGE_SIZE]) != NORFX_SUCCESS)
            {
                norfx_test_log_printf("[full] page program failed sector=%lu page=%lu\r\n",
                                      (unsigned long)sector,
                                      (unsigned long)page_index);
                return false;
            }
            uint32_t page_program_us = norfx_timing_elapsed_us(start_mark);
            sector_program_us += page_program_us;
            norfx_timing_record(&norfx_write_stats, page_program_us, NORFX_PAGE_SIZE);
        }

        memset(norfx_test_readback, 0x00, sizeof(norfx_test_readback));
        start_mark = norfx_timing_capture_start();
        if (norfx_fast_read(dev, base_page, 0u, NORFX_SECTOR_SIZE, norfx_test_readback) != NORFX_SUCCESS)
        {
            norfx_test_log_printf("[full] programmed fast-read failed sector=%lu\r\n", (unsigned long)sector);
            return false;
        }
        sector_verify_read_us = norfx_timing_elapsed_us(start_mark);
        norfx_timing_record(&norfx_read_stats, sector_verify_read_us, NORFX_SECTOR_SIZE);

        if (!norfx_expect_buffer_equal("full_sector_compare",
                                       norfx_test_sector_scratch,
                                       norfx_test_readback,
                                       NORFX_SECTOR_SIZE))
        {
            norfx_test_log_printf("[full] compare failed sector=%lu\r\n", (unsigned long)sector);
            return false;
        }

        sector_total_us = sector_erase_us + sector_blank_read_us + sector_program_us + sector_verify_read_us;
        norfx_test_log_printf("[sector] %lu/%lu erase_us=%lu blank_read_us=%lu write_us=%lu verify_read_us=%lu total_us=%lu\r\n",
                              (unsigned long)sector,
                              (unsigned long)NORFX_TEST_LAST_SECTOR,
                              (unsigned long)sector_erase_us,
                              (unsigned long)sector_blank_read_us,
                              (unsigned long)sector_program_us,
                              (unsigned long)sector_verify_read_us,
                              (unsigned long)sector_total_us);
    }

    return true;
}
