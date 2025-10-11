#include "rtconfig.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "sf32lb52_lcpu.h"
#include "sf32lb52_lcpu_patch.h"
#include "bf0_hal.h"
#include "mem_map.h"

/* Install Bluetooth core (LCPU) image*/
void lcpu_img_install()
{
    uint8_t rev_id = __HAL_SYSCFG_GET_REVID();

    if (rev_id < HAL_CHIP_REV_ID_A4)
        memcpy((void *)(HCPU_LCPU_CODE_START_ADDR), g_lcpu_bin, sizeof(g_lcpu_bin));
}

/* Install Bluetooth core (LCPU) image patch*/
void lcpu_patch_install()
{
    memcpy((void *)(LCPU_PATCH_RECORD_ADDR), g_lcpu_patch_list, sizeof(g_lcpu_patch_list));
    HAL_PATCH_install();
    memset((void *)(LCPU_PATCH_START_ADDR_S), 0, LCPU_PATCH_TOTAL_SIZE);
    memcpy((void *)(LCPU_PATCH_START_ADDR_S), g_lcpu_patch_bin, sizeof(g_lcpu_patch_bin));
};

/* Install Bluetooth core (LCPU) image patch for rev B chipsets*/
void lcpu_patch_install_rev_b()
{
    uint32_t entry[3] = {0x48434150, 0xA, LCPU_PATCH_CODE_START_ADDR + 1};

    memcpy((void *)LCPU_PATCH_BUF_START_ADDR, (void *)&entry, 12);
    memset((void *)(LCPU_PATCH_CODE_START_ADDR_S), 0, LCPU_PATCH_CODE_SIZE);
    memcpy((void *)(LCPU_PATCH_CODE_START_ADDR_S), g_lcpu_patch_bin_b, sizeof(g_lcpu_patch_bin_b));
    HAL_PATCH_install();
}

/* Get patch entry address*/
uint32_t *HAL_PATCH_GetEntryAddr(void)
{
    uint32_t *entry_addr;
    uint8_t rev_id = __HAL_SYSCFG_GET_REVID();

    if (rev_id >= HAL_CHIP_REV_ID_A4)
        entry_addr = (uint32_t *)g_lcpu_patch_list_b;
    else
        entry_addr = (uint32_t *)LCPU_PATCH_RECORD_ADDR;
    return entry_addr;
}
