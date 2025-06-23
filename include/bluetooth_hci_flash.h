/*
 * SPDX-FileCopyrightText: 2023-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef HCI_ON_FLASH

    #ifndef __BLUETOOTH_HCI_FLASH_H
        #define __BLUETOOTH_HCI_FLASH_H


        #include "rtthread.h"
        #include "os_adaptor.h"
        #include "board.h"

        #ifndef BT_HCI_PARTION
            #define BT_HCI_PARTION "hci"
        #endif

        #ifndef BT_HCI_PATH
            #define BT_HCI_PATH "hci/"
        #endif

        #ifndef BT_HCI_DFT_SIZE
            #define BT_HCI_DFT_SIZE (512*1024)
        #endif // BT_HCI_DFT_SIZE

        #define BT_HCI_DEFAULT_FLUSH_SIZE (64 * 1024)
        #define BT_HCI_INVALID_FLUSH_SIZE (0xFFFFFFFF)


        int bt_hci_log_path_get(char *path);
        int bt_hci_log_onoff(int flag);
        int bt_hci_log_type_get(void);
        int bt_hci_log_clear(void);



        #if defined(RT_USING_DFS) || defined(USING_FILE_LOGGER)
            uint32_t bt_hci_init(uint32_t flush_size, uint32_t is_start);
            uint32_t bt_hci_write(uint8_t *buffer, uint32_t len);
            uint32_t bt_hci_flush(void);
            uint32_t bt_hci_close(void);
            uint32_t bt_hci_open(void);

        #endif // RT_USING_DFS

    #endif //__BLUETOOTH_HCI_FLASH_H

#endif // HCI_ON_FLASH


