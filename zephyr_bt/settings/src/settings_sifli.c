// NVDS Backend for Bluetooth Settings
// Key formats (using colon-free Bluetooth address hex):
// - bt/keys/AABBCCDDEEFF (device-specific keys)
// - bt/sc/AABBCCDDEEFF (security configurations)
// - bt/name (device name string)
// - bt/id/<index> (Bluetooth addresses, 6 bytes each)
// - bt/irk/<index> (identity resolving keys, 16 bytes each)

#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <stdio.h>
#include <zephyr/sys/util.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <../../host/keys.h>

// NVDS Adapter Header (update path for your project)
#include "bf0_sibles_nvds.h"

// FlashDB header for KV iteration
#ifdef PKG_USING_FLASHDB
    #include "flashdb.h"
#endif

#ifdef CONFIG_BT_MESH
#include "subnet.h"
#include "app_keys.h"
#include "cdb.h"
#include "settings.h"
#include "access.h"
#include "net.h"
#include "../mesh/keys.h"

#define F_NODE_CONFIGURED 0x01

/* NetKey storage information */
struct net_key_val
{
    uint8_t kr_flag: 1,
            kr_phase: 7;
    struct bt_mesh_key val[2];
} __packed;

/* Node information for persistent storage (from cdb.c) */
struct node_val
{
    uint16_t net_idx;
    uint8_t num_elem;
    uint8_t flags;
    uint8_t uuid[16];
    struct bt_mesh_key dev_key;
} __packed;

/* CDB Network information for persistent storage (from cdb.c) */
struct cdb_net_val
{
    struct __packed
    {
        uint32_t index;
        bool     update;
    } iv;
    uint16_t lowest_avail_addr;
} __packed;

/* AppKey information for persistent storage. */
struct app_key_val
{
    uint16_t net_idx;
    bool updated;
    struct bt_mesh_key val[2];
} __packed;

struct net_val
{
    uint16_t primary_addr;
    struct bt_mesh_key dev_key;
} __packed;

/* Sequence number information for persistent storage. */
struct seq_val
{
    uint8_t val[3];
} __packed;

/* IV Index & IV Update information for persistent storage. */
struct iv_val
{
    uint32_t iv_index;
    uint8_t  iv_update: 1,
             iv_duration: 7;
} __packed;
#endif

LOG_MODULE_REGISTER(settings_sifli, LOG_LEVEL_INF);

// Log Configuration
#if 0
    #undef LOG_DBG
    #define LOG_DBG(fmt,...) rt_kprintf("%s "fmt"\n",__FUNCTION__,##__VA_ARGS__)
    #undef LOG_INF
    #define LOG_INF(fmt,...) rt_kprintf("I:%s "fmt"\n",__FUNCTION__,##__VA_ARGS__)
    #undef LOG_WRN
    #define LOG_WRN(fmt,...) rt_kprintf("W:%s "fmt"\n",__FUNCTION__,##__VA_ARGS__)
    #undef LOG_ERR
    #define LOG_ERR(fmt,...) rt_kprintf("E:%s "fmt"\n",__FUNCTION__,##__VA_ARGS__)
#endif


// --------------------------
// Configuration Constants
// --------------------------
#ifndef BT_SC_STORAGE_LEN
    #define BT_SC_STORAGE_LEN 4 // Length of security config per device (bytes)
#endif

#ifndef BT_DEVICE_NAME_MAX
    #define BT_DEVICE_NAME_MAX 32
#endif


// Key Name Constants
#define BT_ALL_KEY "bt/all" // Base path for device keys
#define BT_KEYS_BASE_KEY "bt/keys" // Base path for device keys
#define BT_SC_BASE_KEY "bt/sc" // Base path for security configs
#define BT_NAME_KEY "bt/name" // Device name key
#define BT_ID_KEY "bt/id" // Base path for identity addresses (bt/id/<index>)
#define BT_IRK_KEY "bt/irk" // Base path for identity resolving keys
#define BT_BUNDLE_ROOT_KEY "bt_bundle"// Root key for NVDS bundle storage

// Bluetooth Address Constants
#define BT_ADDR_LEN sizeof(bt_addr_le_t) // 6 bytes (fixed for bt_addr_le_t)
#define BT_ADDR_HEX_LEN 13 // Length of colon-free hex address string (6 bytes → 12 chars + 1 char for type)
#define MAX_KEY_NAME_LEN 64 // Maximum length for any settings key

// --------------------------
// Data Structures
// --------------------------

/**

@brief Stores all data associated with a paired Bluetooth device
Used for both bt/keys and bt/sc entries for the same device
*/
struct bt_paired_device
{
    bt_addr_le_t addr; // 7-byte BLE address (unique identifier)
    uint8_t key[BT_KEYS_STORAGE_LEN];// Key data (for bt/keys/<addr>)
    uint8_t sc[BT_SC_STORAGE_LEN]; // Security configuration (for bt/sc/<addr>)
    bool in_use; // Flag indicating entry is occupied
};

/**

@brief In-memory cache for all Bluetooth settings
Serialized as a single blob for NVDS storage
NOTE: bt/id/<index> now stores Bluetooth addresses (6 bytes each)
*/
struct bt_settings_bundle
{
    struct bt_paired_device paired_devs[BT_MAX_PAIRED]; // Address-based entries
    uint8_t name[BT_DEVICE_NAME_MAX + 1]; // Device name (null-terminated)
    bt_addr_le_t id[BT_ID_MAX]; // bt/id/<index>: BLE addresses (7 bytes each)
    uint8_t irk[BT_ID_MAX][16]; // bt/irk/<index>: IRKs (16 bytes each)
    size_t id_available; // Count of active/available bt/id entries
};

/**

@brief NVDS settings backend instance
Manages cache state and integration with Zephyr's settings framework
*/
struct settings_nvds_store
{
    struct settings_store base_store; // Base settings store structure
    bool is_initialized; // Initialization state flag
    struct bt_settings_bundle bt_cache;// In-memory settings cache
    bool bundle_dirty; // Flag indicating unsaved changes
    struct k_work_delayable flush_work; // work to flush data
};

// Static singleton instance of the NVDS backend
static struct settings_nvds_store nvds_backend;

// --------------------------
// Bluetooth Address Helpers
// --------------------------

/**

@brief Convert bt_addr_le_t to colon-free hex string (e.g., AABBCCDDEEFF)
@param addr Pointer to Bluetooth address structure
@param hex_buf Buffer to store hex string (must be ≥ BT_ADDR_HEX_LEN + 1)
@param buf_len Size of hex_buf
@return 0 on success, -EINVAL on invalid parameters
*/
static int bt_addr_to_hex(const bt_addr_le_t *addr, char *hex_buf, size_t buf_len)
{
    if (!addr || !hex_buf || buf_len < BT_ADDR_LE_SIZE)
    {
        LOG_ERR("Invalid parameters (addr=%p, buf=%p, len=%d)", addr, hex_buf, buf_len);
        return -EINVAL;
    }
// Format 6-byte address as 12-character hex string (no colons, uppercase)
    snprintf(hex_buf, buf_len, "%02x%02x%02x%02x%02x%02x%u",
             addr->a.val[5], addr->a.val[4], addr->a.val[3],
             addr->a.val[2], addr->a.val[1], addr->a.val[0], addr->type);
    return 0;
}

/**

@brief Parse colon-free hex string to bt_addr_le_t
@param hex_str Input string (e.g., "AABBCCDDEEFF")
@param addr Pointer to bt_addr_le_t to store result
@return 0 on success, -EINVAL on invalid string format
*/
static int bt_hex_to_addr(const char *hex_str, bt_addr_le_t *addr)
{
    if (!hex_str || !addr || strlen(hex_str) != BT_ADDR_HEX_LEN)
    {
        LOG_ERR("Invalid address string: %s (length=%d, expected %d)",
                hex_str, strlen(hex_str), BT_ADDR_HEX_LEN);
        return -EINVAL;
    }
// Parse 2-character hex segments (6 total segments for 6-byte address)
    for (int i = 0; i < sizeof(bt_addr_le_t); i++)
    {
        const char *seg = &hex_str[i * 2]; // Get 2-character segment
        char seg_buf[3] = {seg[0], seg[1], '\0'};
        char *end_ptr;
// Convert hex segment to byte value
        long val = strtol(seg_buf, &end_ptr, 16);
        if (*end_ptr != '\0' || val < 0 || val > 0xFF)
        {
            LOG_ERR("Invalid hex segment: %s in %s", seg_buf, hex_str);
            return -EINVAL;
        }
// Store in Bluetooth address format (val[5] = most significant byte)
        if (i <= 5)
            addr->a.val[5 - i] = (uint8_t)val;
        else
            addr->type = (uint8_t)val;
    }
    return 0;
}

/**

@brief Find index of paired device by Bluetooth address
@param addr Pointer to Bluetooth address to search for
@return Index (0-based) if found, -ENOENT if not found
*/
static int find_paired_dev_by_addr(const bt_addr_le_t *addr)
{
    for (size_t i = 0; i < BT_MAX_PAIRED; i++)
    {
        if (nvds_backend.bt_cache.paired_devs[i].in_use &&
                memcmp(&nvds_backend.bt_cache.paired_devs[i].addr, addr, sizeof(bt_addr_le_t)) == 0)
        {
            return (int)i;
        }
    }
    return -ENOENT;
}

/**

@brief Find first empty slot in paired devices array
@return Index (0-based) of empty slot, -ENOMEM if all slots are used
*/
static int find_empty_paired_dev_slot(void)
{
    for (size_t i = 0; i < BT_MAX_PAIRED; i++)
    {
        if (!nvds_backend.bt_cache.paired_devs[i].in_use)
        {
            return (int)i;
        }
    }
    LOG_ERR("No empty slots (max paired devices: %d)", BT_MAX_PAIRED);
    return -ENOMEM;
}

// --------------------------
// Key Generation/Validation Helpers
// --------------------------

/**

@brief Generate address-based key name (e.g., bt/keys/AABBCCDDEEFF)
@param key_buf Buffer to store key name
@param buf_len Size of key_buf
@param base_key Base key path (e.g., "bt/keys")
@param addr Pointer to Bluetooth address
@return 0 on success, -EINVAL on buffer overflow
*/
static int generate_addr_based_key(char *key_buf, size_t buf_len,
                                   const char *base_key, const bt_addr_le_t *addr)
{
    char addr_hex[BT_ADDR_HEX_LEN + 1];
    int ret = bt_addr_to_hex(addr, addr_hex, sizeof(addr_hex));
    if (ret != 0)
    {
        return ret;
    }
// Combine base key and address hex (e.g., "bt/keys" + "AABBCCDDEEFF")
    ret = snprintf(key_buf, buf_len, "%s/%s", base_key, addr_hex);
    if (ret < 0 || (size_t)ret >= buf_len)
    {
        LOG_ERR("Key buffer too small (required: %d, available: %d)", ret + 1, buf_len);
        return -EINVAL;
    }
    return 0;
}

/**

@brief Parse Bluetooth address from address-based key name
@param key Key name (e.g., "bt/keys/AABBCCDDEEFF")
@param base_key Base key path (e.g., "bt/keys")
@param addr Pointer to bt_addr_let to store result
@return 0 on success, -EINVAL on invalid key format
*/
static int parse_bt_addr_from_key(const char *key, const char *base_key, bt_addr_le_t *addr)
{
// Verify key starts with base_key + "/" (e.g., "bt/keys/")
    const char *base_prefix = strstr(key, base_key);
    if (!base_prefix || strlen(base_prefix) <= strlen(base_key))
    {
        LOG_ERR("Key %s is not a valid %s key (missing '/<addr>')", key, base_key);
        return -EINVAL;
    }
// Extract address hex string (skip "base_key/")
    const char *addr_hex = base_prefix + strlen(base_key) + 1;
    return bt_hex_to_addr(addr_hex, addr);
}

/**

@brief Generate indexed key name (e.g., bt/id/0, bt/irk/2)
@param key_buf Buffer to store key name
@param buf_len Size of key_buf
@param base_key Base key path (e.g., "bt/id")
@param index Numeric index (0 ≤ index < BT_ID_MAX)
@return 0 on success, -EINVAL on buffer overflow
*/
static int generate_indexed_key(char *key_buf, size_t buf_len, const char *base_key, size_t index)
{
    if (index >= BT_ID_MAX)
    {
        LOG_ERR("Index %d exceeds max bt/id/irk entries (%d)", index, BT_ID_MAX - 1);
        return -EINVAL;
    }
    int ret = snprintf(key_buf, buf_len, "%s/%d", base_key, index);
    if (ret < 0 || (size_t)ret >= buf_len)
    {
        LOG_ERR("Key buffer too small (required: %d, available: %d)", ret + 1, buf_len);
        return -EINVAL;
    }
    return 0;
}

/**

@brief Parse numeric index from indexed key name (e.g., bt/id/0 → index 0)
@param key Key name (e.g., "bt/id/0")
@param base_key Base key path (e.g., "bt/id")
@param index_out Pointer to store parsed index
@return 0 on success, -EINVAL on invalid key format
*/
static int parse_key_index(const char *key, const char *base_key, size_t *index_out)
{
// Verify key starts with base_key + "/" (e.g., "bt/id/")
    const char *base_prefix = strstr(key, base_key);
    if (!base_prefix || strlen(base_prefix) <= strlen(base_key))
    {
        LOG_WRN("Key %s is not a valid %s key (missing '/<index>')", key, base_key);
        *index_out = 0;
        return 0;
    }
// Extract index string (skip "base_key/")
    const char *index_str = base_prefix + strlen(base_key) + 1;
    if (*index_str == '\0')
    {
        LOG_ERR("Key %s has empty index (expected %s/<number>)", key, base_key);
        return -EINVAL;
    }
// Parse numeric index (must be non-negative and < BT_ID_MAX)
    char *end_ptr;
    long index = strtol(index_str, &end_ptr, 10);
    if (*end_ptr != '\0' || index < 0 || (size_t)index >= BT_ID_MAX)
    {
        LOG_ERR("Invalid index in key %s (must be 0-%d)", key, BT_ID_MAX - 1);
        return -EINVAL;
    }
    *index_out = (size_t)index;
    return 0;
}

/**

@brief Custom key matching function (filters keys by subtree)
Used to check if a key belongs to the requested subtree (e.g., "bt/id" → matches "bt/id/0")
*/
static bool key_match(const char *subtree, const char *key)
{
    size_t subtree_len = strlen(subtree);
// Match if: key == subtree (exact) OR key starts with subtree + "/" (child entry)
    return (strncmp(key, subtree, subtree_len) == 0) &&
           (key[subtree_len] == '\0' || key[subtree_len] == '/');
}


/**
@brief Recalculate id_available by counting non-zero id entries
Ensures id_available stays in sync with actual used entries
*/
static void recalculate_id_available(void)
{
    size_t count = 0;
    for (size_t i = 0; i < BT_ID_MAX; i++)
    {
        // Check if address is non-zero (active entry)
        bool is_non_zero = false;
        for (size_t j = 0; j < sizeof(bt_addr_le_t); j++)
        {
            if (((uint8_t *)&nvds_backend.bt_cache.id[i])[j] != 0)
            {
                is_non_zero = true;
                break;
            }
        }
        if (is_non_zero)
        {
            count++;
        }
    }
    nvds_backend.bt_cache.id_available = count;
}

// --------------------------
// Settings Framework Callbacks
// --------------------------

/**

@brief Read callback for Zephyr settings framework
Provides cached data to the framework when requested (required for direct loading)
@param arg Pointer to callback data (contains cached value and length)
@param buf Buffer to copy data into (provided by framework)
@param len Length of data to read (requested by framework)
@return len on success, -EINVAL on invalid parameters
*/
static ssize_t settings_read_callback(void *arg, void *buf, unsigned int len)
{
// Unpack callback data (cached value + length)
    struct
    {
        const void *data; // Pointer to cached value (e.g., bt/id address, IRK)
        size_t data_len; // Length of cached value (fixed for each key type)
    } *cb_data = arg;
    if (!cb_data || !buf)
    {
        LOG_ERR("Invalid read callback params (arg=%p, buf=%p, len=%d > max %d)",
                cb_data, buf, len, cb_data->data_len);
        return -EINVAL;
    }
    if (len > cb_data->data_len)
        len = cb_data->data_len;
// Copy cached data to framework buffer
    LOG_INF("%p, %p, %p", cb_data->data, &nvds_backend.bt_cache, &nvds_backend.bt_cache.name[0]);
    memcpy(buf, cb_data->data, len);
    return len;
}

/**

@brief Invoke framework callback for a single key-value pair
Filters keys by subtree and passes data to the framework via settings_read_callback
@param arg Framework load argument (contains subtree, callback, etc.)
@param key Settings key name (e.g., "bt/id/0")
@param value Pointer to cached value (e.g., bt_addr_le_t for bt/id)
@param val_len Length of cached value (6 for bt/id, 16 for bt/irk, etc.)
@return Framework callback return code, 0 if skipped
*/
static int invoke_load_callback(const struct settings_load_arg *arg,
                                const char *key, const void *value, size_t val_len)
{
// Skip if key doesn't match requested subtree
    if (arg->subtree && !key_match(arg->subtree, key))
    {
        return 0;
    }
// Prepare callback data structure
    struct
    {
        const void *data;
        size_t data_len;
    } cb_data =
    {
        .data = value,
        .data_len = val_len
    };
// Invoke framework callback with correct signature
    if (arg->cb)
    {
        LOG_INF("%s:%p", key, arg->cb);
        int cb_ret = arg->cb(key, val_len, settings_read_callback, &cb_data, arg->param);
        if (cb_ret != 0 && cb_ret != -ENOENT)
        {
            LOG_WRN("Load callback failed for key '%s' (ret=%d)", key, cb_ret);
        }
        return cb_ret;
    }
    else
    {
        struct settings_handler_static *ch;
        int rc;
        const char *name_key = key;

        ch = settings_parse_and_lookup(key, &name_key);
        if (!ch)
        {
            return 0;
        }

        rc = ch->h_set(name_key, val_len, settings_read_callback, &cb_data);

        if (rc != 0)
        {
            LOG_ERR("set-value failure. key: %s error(%d)",
                    key, rc);
            /* Ignoring the error */
            rc = 0;
        }
        else
        {
            LOG_DBG("set-value OK. key: %s, %p", key, ch->h_set);
        }
        return rc;
    }
}

// --------------------------
// Public API - Settings Access
// --------------------------

/**

@brief Set key data for a paired device (bt/keys/<addr>)
@param addr Pointer to device's Bluetooth address
@param key_data Pointer to BT_KEYS_STORAGE_LEN bytes of key data
@return 0 on success, negative error code on failure
*/
int settings_bt_keys_set(const bt_addr_le_t *addr, const void *key_data)
{
    if (!nvds_backend.is_initialized || !addr || !key_data)
    {
        LOG_ERR("Invalid parameters (initialized=%d, addr=%p, data=%p)",
                nvds_backend.is_initialized, addr, key_data);
        return -EINVAL;
    }
// Find existing entry or empty slot
    int idx = find_paired_dev_by_addr(addr);
    if (idx < 0)
    {
        idx = find_empty_paired_dev_slot();
        if (idx < 0)
        {
            return idx; // -ENOMEM (no slots available)
        }
    }
// Update device entry
    struct bt_paired_device *dev = &nvds_backend.bt_cache.paired_devs[idx];
    memcpy(&dev->addr, addr, BT_ADDR_LEN);
    memcpy(dev->key, key_data, BT_KEYS_STORAGE_LEN);
    dev->in_use = true;
    nvds_backend.bundle_dirty = true;
    char addr_hex[BT_ADDR_HEX_LEN + 1];
    bt_addr_to_hex(addr, addr_hex, sizeof(addr_hex));
    LOG_DBG("Set %s/%s", BT_KEYS_BASE_KEY, addr_hex);
    return 0;
}

/**

@brief Get key data for a paired device (bt/keys/<addr>)
@param addr Pointer to device's Bluetooth address
@param key_buf Buffer to store BT_KEYS_STORAGE_LEN bytes of key data
@return 0 on success, -ENOENT if not found, negative error code on failure
*/
int settings_bt_keys_get(const bt_addr_le_t *addr, void *key_buf)
{
    if (!nvds_backend.is_initialized || !addr || !key_buf)
    {
        LOG_ERR("Invalid parameters (initialized=%d, addr=%p, buf=%p)",
                nvds_backend.is_initialized, addr, key_buf);
        return -EINVAL;
    }
// Find device entry
    int idx = find_paired_dev_by_addr(addr);
    if (idx < 0)
    {
        return -ENOENT; // Device not found
    }
// Retrieve key data
    memcpy(key_buf, nvds_backend.bt_cache.paired_devs[idx].key, BT_KEYS_STORAGE_LEN);
    return 0;
}

/**

@brief Set security configuration for a paired device (bt/sc/<addr>)
@param addr Pointer to device's Bluetooth address
@param sc_data Pointer to BT_SC_STORAGE_LEN bytes of config data
@return 0 on success, negative error code on failure
*/
int settings_bt_sc_set(const bt_addr_le_t *addr, const void *sc_data)
{
    if (!nvds_backend.is_initialized || !addr || !sc_data)
    {
        LOG_ERR("Invalid parameters (initialized=%d, addr=%p, data=%p)",
                nvds_backend.is_initialized, addr, sc_data);
        return -EINVAL;
    }
// Find existing entry or empty slot
    int idx = find_paired_dev_by_addr(addr);
    if (idx < 0)
    {
        idx = find_empty_paired_dev_slot();
        if (idx < 0)
        {
            return idx; // -ENOMEM (no slots available)
        }
    }
// Update device entry
    struct bt_paired_device *dev = &nvds_backend.bt_cache.paired_devs[idx];
    memcpy(&dev->addr, addr, BT_ADDR_LEN);
    memcpy(dev->sc, sc_data, BT_SC_STORAGE_LEN);
    dev->in_use = true;
    nvds_backend.bundle_dirty = true;
    char addr_hex[BT_ADDR_HEX_LEN + 1];
    bt_addr_to_hex(addr, addr_hex, sizeof(addr_hex));
    LOG_DBG("Set %s/%s", BT_SC_BASE_KEY, addr_hex);
    return 0;
}

/**

@brief Get security configuration for a paired device (bt/sc/<addr>)
@param addr Pointer to device's Bluetooth address
@param sc_buf Buffer to store BT_SC_STORAGE_LEN bytes of config data
@return 0 on success, -ENOENT if not found, negative error code on failure
*/
int settings_bt_sc_get(const bt_addr_le_t *addr, void *sc_buf)
{
    if (!nvds_backend.is_initialized || !addr || !sc_buf)
    {
        LOG_ERR("Invalid parameters (initialized=%d, addr=%p, buf=%p)",
                nvds_backend.is_initialized, addr, sc_buf);
        return -EINVAL;
    }
// Find device entry
    int idx = find_paired_dev_by_addr(addr);
    if (idx < 0)
    {
        return -ENOENT; // Device not found
    }
// Retrieve security configuration
    memcpy(sc_buf, nvds_backend.bt_cache.paired_devs[idx].sc, BT_SC_STORAGE_LEN);
    return 0;
}

/**

@brief Delete all data for a paired device (keys and security config)
@param addr Pointer to device's Bluetooth address
@return 0 on success, -ENOENT if not found, negative error code on failure
*/
int settings_bt_device_delete(const bt_addr_le_t *addr)
{
    if (!nvds_backend.is_initialized || !addr)
    {
        LOG_ERR("Invalid parameters (initialized=%d, addr=%p)",
                nvds_backend.is_initialized, addr);
        return -EINVAL;
    }
// Find device entry
    int idx = find_paired_dev_by_addr(addr);
    if (idx < 0)
    {
        return -ENOENT; // Device not found
    }
// Clear device entry
    memset(&nvds_backend.bt_cache.paired_devs[idx], 0, sizeof(struct bt_paired_device));
    nvds_backend.bundle_dirty = true;
    return 0;
}

/**

@brief Set Bluetooth device name (bt/name)
@param name Null-terminated string (max length BT_DEVICE_NAME_MAX)
@return 0 on success, negative error code on failure
*/
int settings_bt_name_set(const char *name)
{
    if (!nvds_backend.is_initialized || !name)
    {
        LOG_ERR("Invalid parameters (initialized=%d, name=%p)",
                nvds_backend.is_initialized, name);
        return -EINVAL;
    }
    size_t name_len = strlen(name);
    if (name_len > BT_DEVICE_NAME_MAX)
    {
        LOG_ERR("Name too long (%d > %d)", name_len, BT_DEVICE_NAME_MAX);
        return -EINVAL;
    }
// Update device name
    memset(nvds_backend.bt_cache.name, 0, BT_DEVICE_NAME_MAX + 1);
    memcpy(nvds_backend.bt_cache.name, name, name_len);
    nvds_backend.bundle_dirty = true;
    LOG_DBG("Set %s to '%s'", BT_NAME_KEY, name);
    return 0;
}

/**

@brief Get Bluetooth device name (bt/name)
@param name_buf Buffer to store null-terminated name
@param buf_len Size of name_buf (must be ≥ BT_DEVICE_NAME_MAX + 1)
@return 0 on success, negative error code on failure
*/
int settings_bt_name_get(char *name_buf, size_t buf_len)
{
    if (!nvds_backend.is_initialized || !name_buf || buf_len < BT_DEVICE_NAME_MAX + 1)
    {
        LOG_ERR("Invalid parameters (initialized=%d, buf=%p, len=%d)",
                nvds_backend.is_initialized, name_buf, buf_len);
        return -EINVAL;
    }
// Retrieve device name with proper null termination
    strncpy(name_buf, (const char *)nvds_backend.bt_cache.name, buf_len - 1);
    name_buf[buf_len - 1] = '\0';
    return 0;
}

/**

@brief Set identity address (bt/id/<index>)
@param index Entry index (0 ≤ index < BT_ID_MAX)
@param addr Bluetooth address to store
@return 0 on success, negative error code on failure
*/
int settings_bt_id_set(size_t index, const bt_addr_le_t *addr)
{
    if (!nvds_backend.is_initialized || index >= BT_ID_MAX || !addr)
    {
        LOG_ERR("Invalid parameters (initialized=%d, index=%d, addr=%p)",
                nvds_backend.is_initialized, index, addr);
        return -EINVAL;
    }
// Update identity address
    memcpy(&(nvds_backend.bt_cache.id[index]), addr, BT_ADDR_LEN);
    nvds_backend.bundle_dirty = true;
    char addr_hex[BT_ADDR_HEX_LEN + 1];
    bt_addr_to_hex(addr, addr_hex, sizeof(addr_hex));
    LOG_DBG("Set %s/%d to %s", BT_ID_KEY, index, addr_hex);
    return 0;
}

/**

@brief Get identity address (bt/id/<index>)
@param index Entry index (0 ≤ index < BT_ID_MAX)
@param addr_out Pointer to store Bluetooth address
@return 0 on success, negative error code on failure
*/
int settings_bt_id_get(size_t index, bt_addr_le_t *addr_out)
{
    if (!nvds_backend.is_initialized || index >= BT_ID_MAX || !addr_out)
    {
        LOG_ERR("Invalid parameters (initialized=%d, index=%d, out=%p)",
                nvds_backend.is_initialized, index, addr_out);
        return -EINVAL;
    }
// Retrieve identity address
    memcpy(addr_out, &(nvds_backend.bt_cache.id[index]), BT_ADDR_LEN);
    return 0;
}

/**

@brief Set Identity Resolving Key (bt/irk/<index>)
@param index Entry index (0 ≤ index < BT_ID_MAX)
@param irk_data Pointer to 16-byte IRK data
@return 0 on success, negative error code on failure
*/
int settings_bt_irk_set(size_t index, const void *irk_data)
{
    if (!nvds_backend.is_initialized || index >= BT_ID_MAX || !irk_data)
    {
        LOG_ERR("Invalid parameters (initialized=%d, index=%d, data=%p)",
                nvds_backend.is_initialized, index, irk_data);
        return -EINVAL;
    }
// Update IRK
    memcpy(nvds_backend.bt_cache.irk[index], irk_data, 16);
    nvds_backend.bundle_dirty = true;
    LOG_DBG("Set %s/%d", BT_IRK_KEY, index);
    return 0;
}

/**

@brief Get Identity Resolving Key (bt/irk/<index>)
@param index Entry index (0 ≤ index < BT_ID_MAX)
@param irk_buf Buffer to store 16-byte IRK data
@return 0 on success, negative error code on failure
*/
int settings_bt_irk_get(size_t index, void *irk_buf)
{
    if (!nvds_backend.is_initialized || index >= BT_ID_MAX || !irk_buf)
    {
        LOG_ERR("Invalid parameters (initialized=%d, index=%d, buf=%p)",
                nvds_backend.is_initialized, index, irk_buf);
        return -EINVAL;
    }
// Retrieve IRK
    memcpy(irk_buf, nvds_backend.bt_cache.irk[index], 16);
    return 0;
}

// --------------------------
// Bundle Serialization/Storage
// --------------------------

/**

@brief Get size of Bluetooth settings bundle
@return Size in bytes
*/
static size_t bt_bundle_get_size(void)
{
    return sizeof(struct bt_settings_bundle);
}

/**

@brief Serialize in-memory cache to buffer
@param buf Buffer to store serialized data
@param buf_len Size of buffer
@return Size of serialized data on success, -EINVAL on failure
*/
static ssize_t bt_bundle_serialize(void *buf, size_t buf_len)
{
    if (!buf || buf_len < bt_bundle_get_size())
    {
        LOG_ERR("Invalid buffer (size=%d < required=%d)", buf_len, bt_bundle_get_size());
        return -EINVAL;
    }
    memcpy(buf, &nvds_backend.bt_cache, bt_bundle_get_size());
    return bt_bundle_get_size();
}

/**

@brief Deserialize buffer to in-memory cache
@param buf Buffer containing serialized data
@param buf_len Size of buffer
@return 0 on success, -EINVAL on failure
*/
static int bt_bundle_deserialize(const void *buf, size_t buf_len)
{
    if (!buf || buf_len < bt_bundle_get_size())
    {
        LOG_ERR("Invalid buffer (size=%d < required=%d)", buf_len, bt_bundle_get_size());
        return -EINVAL;
    }
    memcpy(&nvds_backend.bt_cache, buf, bt_bundle_get_size());
    return 0;
}

/**

@brief Save settings bundle to NVDS
@return 0 on success, negative error code on failure
*/
static int bt_bundle_save(void)
{
    int r = 0;

    if (!nvds_backend.is_initialized)
    {
        LOG_ERR("Backend not initialized");
        r = -ENODEV;
        goto end;
    }
// No need to save if no changes
    if (!nvds_backend.bundle_dirty)
    {
        LOG_DBG("No changes to save");
        goto end;
    }
// Allocate buffer for serialized bundle
    uint8_t *bundle_buf = k_malloc(bt_bundle_get_size());
    if (!bundle_buf)
    {
        LOG_ERR("Memory allocation failed");
        r = -ENOMEM;
        goto end;
    }
// Serialize cache to buffer
    ssize_t serialized_len = bt_bundle_serialize(bundle_buf, bt_bundle_get_size());
    if (serialized_len < 0)
    {
        k_free(bundle_buf);
        r = (int)serialized_len;
        goto end;
    }

// Mark cache as clean
    nvds_backend.bundle_dirty = false;

// Write new bundle to NVDS
    uint8_t nvds_ret = sifli_nvds_flash_adaptor_write(BT_BUNDLE_ROOT_KEY, bundle_buf, bt_bundle_get_size());
    k_free(bundle_buf);
    if (nvds_ret != 0)
    {
        LOG_ERR("Failed to write bundle (NVDS ret=0x%02X)", nvds_ret);
        r = -EIO;
        goto end;
    }

end:
    LOG_INF("Saved BT settings bundle (%d bytes)", bt_bundle_get_size());
    return r;
}

/**

@brief Load settings bundle from NVDS
@return 0 on success, negative error code on failure
*/
static int bt_bundle_load_from_nvds(void)
{
    if (!nvds_backend.is_initialized)
    {
        LOG_ERR("Backend not initialized");
        return -ENODEV;
    }
// Allocate buffer for reading bundle
    uint8_t *bundle_buf = k_malloc(bt_bundle_get_size());
    if (!bundle_buf)
    {
        LOG_ERR("Memory allocation failed");
        return -ENOMEM;
    }
// Read bundle from NVDS
    size_t read_len = sifli_nvds_flash_adaptor_read(BT_BUNDLE_ROOT_KEY, bundle_buf, bt_bundle_get_size());
    if (read_len == 0)
    {
        LOG_WRN("No existing bundle found - initializing empty");
        memset(&nvds_backend.bt_cache, 0, sizeof(struct bt_settings_bundle));
        nvds_backend.bt_cache.id_available = 0; // Initialize new bundle with 0 available IDs
        k_free(bundle_buf);
        return 0;
    }
// Validate bundle size
    if (read_len != bt_bundle_get_size())
    {
        LOG_ERR("Corrupted bundle (read %d bytes, expected %d)", read_len, bt_bundle_get_size());
        k_free(bundle_buf);
        return -EIO;
    }
// Deserialize into cache
    int ret = bt_bundle_deserialize(bundle_buf, read_len);
    if (ret == 0)
    {
        // Recalculate to fix potential inconsistencies in loaded data
        recalculate_id_available();
        LOG_DBG("Recalculated id_available to %d after load", nvds_backend.bt_cache.id_available);
    }
    k_free(bundle_buf);
    return ret;
}

// --------------------------
// Zephyr Settings Framework Implementation
// --------------------------

#ifdef CONFIG_BT_MESH
/**
@brief Iterate and load all bt/mesh/* keys from FlashDB
@param arg Settings load argument with callback (may be NULL during probe/cleanup)
@return 0 on success, negative error code on failure
*/
static int load_mesh_keys_from_flashdb(const struct settings_load_arg *arg)
{
    struct fdb_kv_iterator itr_obj;
    fdb_kv_iterator_t itr = &itr_obj;
    int loaded_count = 0;
    fdb_kvdb_t kvdb;

    LOG_INF("load_mesh_keys_from_flashdb called (arg=%p)", arg);

    // Get FlashDB instance through NVDS adapter
    kvdb = sifli_nvds_get_ble_kvdb();
    if (!kvdb)
    {
        LOG_WRN("FlashDB not available, skipping Mesh key loading");
        return 0;  // Don't fail, just skip
    }

    LOG_INF("FlashDB instance obtained, starting iteration...");

    // Initialize iterator
    memset(itr, 0, sizeof(struct fdb_kv_iterator));
    itr->iterated_cnt = 0;

    // Debug: Print all keys in FlashDB
    int total_keys = 0;
    int mesh_keys = 0;
    struct fdb_kv_iterator debug_itr_obj;
    fdb_kv_iterator_t debug_itr = &debug_itr_obj;
    memset(debug_itr, 0, sizeof(struct fdb_kv_iterator));

    LOG_INF("=== FlashDB Contents ===");
    while (fdb_kv_iterate(kvdb, debug_itr))
    {
        total_keys++;
        const char *key_name = debug_itr->curr_kv.name;
        if (strncmp(key_name, "bt/mesh", 7) == 0)
        {
            mesh_keys++;
            LOG_INF("  [MESH] %s", key_name);
        }
        else if (strncmp(key_name, "bt/", 3) == 0)
        {
            LOG_INF("  [BT]   %s", key_name);
        }
    }
    LOG_INF("Total keys: %d, Mesh keys: %d", total_keys, mesh_keys);
    LOG_INF("========================");

    // Reset iterator for actual loading
    memset(itr, 0, sizeof(struct fdb_kv_iterator));
    itr->iterated_cnt = 0;

    LOG_INF("Starting to iterate bt/mesh/* keys from FlashDB");

    // Iterate through all KV pairs in FlashDB
    while (fdb_kv_iterate(kvdb, itr))
    {
        const char *key_name = itr->curr_kv.name;

        // Check if this is a bt/mesh/* key
        if (strncmp(key_name, "bt/mesh", 7) == 0)
        {
            // Read the value using blob API
            uint8_t value_buf[256];  // Max value size for Mesh keys
            struct fdb_blob blob;

            // Create blob structure pointing to our buffer
            fdb_blob_make(&blob, value_buf, sizeof(value_buf));

            // Read the value into the blob
            size_t read_len = fdb_kv_get_blob(kvdb, key_name, &blob);

            if (read_len > 0 && read_len <= sizeof(value_buf))
            {
                // Try to restore Mesh data directly using internal APIs
                bool restored = false;

                // Handle bt/mesh/NetKey/<net_idx>
                if (strncmp(key_name, "bt/mesh/NetKey/", 15) == 0)
                {
                    uint16_t net_idx = strtoul(key_name + 15, NULL, 16);
                    struct net_key_val *net_key = (struct net_key_val *)value_buf;

                    LOG_INF("Restoring NetKey 0x%03x (kr_phase=%d)", net_idx, net_key->kr_phase);

                    // Check if new key exists (all zeros means no new key)
                    bool has_new_key = false;
                    for (int i = 0; i < 16; i++)
                    {
                        if (net_key->val[1].key[i] != 0)
                        {
                            has_new_key = true;
                            break;
                        }
                    }

                    int ret = bt_mesh_subnet_set(net_idx, net_key->kr_phase,
                                                 &net_key->val[0],
                                                 has_new_key ? &net_key->val[1] : NULL);
                    if (ret == 0)
                    {
                        LOG_INF("Successfully restored NetKey 0x%03x", net_idx);
                        restored = true;
                    }
                    else
                    {
                        LOG_WRN("Failed to restore NetKey 0x%03x (ret=%d)", net_idx, ret);
                    }
                }
                // Handle bt/mesh/AppKey/<app_idx>
                else if (strncmp(key_name, "bt/mesh/AppKey/", 15) == 0)
                {
                    uint16_t app_idx = strtoul(key_name + 15, NULL, 16);
                    struct app_key_val *app_key = (struct app_key_val *)value_buf;

                    LOG_INF("Restoring AppKey 0x%03x (net_idx=%d, updated=%d)",
                            app_idx, app_key->net_idx, app_key->updated);

                    // Check if new key exists
                    bool has_new_key = false;
                    for (int i = 0; i < 16; i++)
                    {
                        if (app_key->val[1].key[i] != 0)
                        {
                            has_new_key = true;
                            break;
                        }
                    }

                    int ret = bt_mesh_app_key_set(app_idx, app_key->net_idx,
                                                  &app_key->val[0],
                                                  has_new_key ? &app_key->val[1] : NULL);
                    if (ret == 0)
                    {
                        LOG_INF("Successfully restored AppKey 0x%03x (net_idx=%d)", app_idx, app_key->net_idx);
                        restored = true;
                    }
                    else
                    {
                        LOG_WRN("Failed to restore AppKey 0x%03x (ret=%d)", app_idx, ret);
                    }
                }
                // Handle CDB keys - manually parse and restore since callback is not available
                else if (strncmp(key_name, "bt/mesh/cdb/", 12) == 0)
                {
                    LOG_INF("Restoring CDB key '%s' (%d bytes)", key_name, read_len);

                    // Parse the sub-key type
                    const char *sub_key = key_name + 12;  // Skip "bt/mesh/cdb/"

                    if (strcmp(sub_key, "Net") == 0)
                    {
                        // Restore CDB network state (IV index, etc.)
                        struct cdb_net_val *net_data = (struct cdb_net_val *)value_buf;
                        if (read_len >= sizeof(struct cdb_net_val))
                        {
                            bt_mesh_cdb.iv_index = net_data->iv.index;
                            bt_mesh_cdb.lowest_avail_addr = net_data->lowest_avail_addr;

                            if (net_data->iv.update)
                            {
                                atomic_set_bit(bt_mesh_cdb.flags, BT_MESH_CDB_IVU_IN_PROGRESS);
                            }

                            atomic_set_bit(bt_mesh_cdb.flags, BT_MESH_CDB_VALID);
                            LOG_INF("Successfully restored CDB Net state (IV=0x%08x)", bt_mesh_cdb.iv_index);
                            restored = true;
                        }
                    }
                    else if (strncmp(sub_key, "Node/", 5) == 0)
                    {
                        // Restore CDB node
                        uint16_t addr = strtol(sub_key + 5, NULL, 16);
                        struct node_val *node_data = (struct node_val *)value_buf;

                        if (read_len >= sizeof(struct node_val))
                        {
                            struct bt_mesh_cdb_node *node = bt_mesh_cdb_node_alloc(node_data->uuid, addr,
                                                            node_data->num_elem,
                                                            node_data->net_idx);
                            if (node)
                            {
                                if (node_data->flags & F_NODE_CONFIGURED)
                                {
                                    atomic_set_bit(node->flags, BT_MESH_CDB_NODE_CONFIGURED);
                                }

                                /* Restore DevKey - critical for Config Client operations */
                                memcpy(&node->dev_key, &node_data->dev_key, sizeof(struct bt_mesh_key));

                                LOG_INF("Successfully restored CDB Node 0x%04x (dev_key.id=%u)",
                                        addr, node_data->dev_key.key);
                                restored = true;
                            }
                            else
                            {
                                LOG_WRN("Failed to allocate CDB Node 0x%04x", addr);
                            }
                        }
                    }
                    else if (strncmp(sub_key, "Subnet/", 7) == 0)
                    {
                        // Restore CDB subnet - uses net_key_val structure (same as NetKey)
                        uint16_t net_idx = strtol(sub_key + 7, NULL, 16);
                        struct net_key_val *subnet_data = (struct net_key_val *)value_buf;

                        if (read_len >= sizeof(struct net_key_val))
                        {
                            struct bt_mesh_cdb_subnet *sub = bt_mesh_cdb_subnet_alloc(net_idx);
                            if (sub)
                            {
                                memcpy(sub->keys[0].net_key.key, subnet_data->val[0].key, 16);
                                sub->kr_phase = subnet_data->kr_phase;

                                LOG_INF("Successfully restored CDB Subnet 0x%03x", net_idx);
                                restored = true;
                            }
                            else
                            {
                                LOG_WRN("Failed to allocate CDB Subnet 0x%03x", net_idx);
                            }
                        }
                    }
                    else if (strncmp(sub_key, "AppKey/", 7) == 0)
                    {
                        // Restore CDB app key
                        uint16_t app_idx = strtol(sub_key + 7, NULL, 16);
                        struct app_key_val *appkey_data = (struct app_key_val *)value_buf;

                        if (read_len >= sizeof(struct app_key_val))
                        {
                            struct bt_mesh_cdb_app_key *app = bt_mesh_cdb_app_key_alloc(appkey_data->net_idx, app_idx);
                            if (app)
                            {
                                memcpy(app->keys[0].app_key.key, appkey_data->val[0].key, 16);
                                if (appkey_data->updated)
                                {
                                    memcpy(app->keys[1].app_key.key, appkey_data->val[1].key, 16);
                                }

                                LOG_INF("Successfully restored CDB AppKey 0x%03x", app_idx);
                                restored = true;
                            }
                            else
                            {
                                LOG_WRN("Failed to allocate CDB AppKey 0x%03x", app_idx);
                            }
                        }
                    }
                    else
                    {
                        LOG_WRN("Unknown CDB key: %s", key_name);
                    }
                }
                // Handle model binding and subscription keys (bt/mesh/s/<mod_key>/bind or sub)
                else if (strncmp(key_name, "bt/mesh/s/", 10) == 0)
                {
                    // Parse: bt/mesh/s/<mod_key>/<type> where type is "bind" or "sub"
                    const char *after_s = key_name + 10;  // Skip "bt/mesh/s/"
                    const char *slash = strchr(after_s, '/');

                    if (!slash)
                    {
                        LOG_WRN("Invalid s key format (no slash): %s", key_name);
                        continue;
                    }

                    // Extract mod_key (hex string before the slash)
                    char mod_key_str[8] = {0};
                    size_t key_len = slash - after_s;
                    if (key_len >= sizeof(mod_key_str))
                    {
                        LOG_WRN("Mod key too long: %s", key_name);
                        continue;
                    }
                    strncpy(mod_key_str, after_s, key_len);

                    uint16_t mod_key = strtol(mod_key_str, NULL, 16);
                    uint8_t elem_idx = mod_key >> 8;
                    uint8_t mod_idx = mod_key & 0xFF;

                    // Get the model
                    struct bt_mesh_model *mod = (struct bt_mesh_model *)bt_mesh_model_get(false, elem_idx, mod_idx);
                    if (!mod)
                    {
                        LOG_WRN("Model s/%d/%d (mod_key=0x%04x) not found", elem_idx, mod_idx, mod_key);
                        continue;
                    }

                    // Determine if this is a "sub" or "bind" key
                    if (strcmp(slash + 1, "sub") == 0)
                    {
                        // ===== Handle SUBSCRIPTION restoration =====
                        LOG_INF("Restoring subscription key '%s' (%d bytes)", key_name, read_len);

                        // Clear existing subscriptions first
                        memset(mod->groups, 0, mod->groups_cnt * sizeof(mod->groups[0]));

                        // Restore subscriptions from stored data
                        size_t max_groups = mod->groups_cnt * sizeof(mod->groups[0]);
                        if (read_len > 0 && read_len <= max_groups)
                        {
                            memcpy(mod->groups, value_buf, read_len);

                            // Count restored subscriptions
                            int sub_count = 0;
                            for (int i = 0; i < mod->groups_cnt; i++)
                            {
                                if (mod->groups[i] != 0)
                                {
                                    sub_count++;
                                    LOG_INF("  Restored sub[%d]: 0x%04x", i, mod->groups[i]);
                                }
                            }

                            LOG_INF("Successfully restored %d subscriptions for model s/%d/%d (mod_key=0x%04x)",
                                    sub_count, elem_idx, mod_idx, mod_key);
                            restored = true;
                        }
                        else
                        {
                            LOG_WRN("Invalid sub data length for s/%d/%d (read_len=%d, max=%zu)",
                                    elem_idx, mod_idx, read_len, max_groups);
                        }
                    }
                    else if (strcmp(slash + 1, "bind") == 0)
                    {
                        // ===== Handle BINDING restoration =====
                        LOG_INF("Restoring model binding key '%s' (%d bytes)", key_name, read_len);
                        LOG_INF("Parsing bind key: mod_key=0x%04x, elem=%d, mod=%d", mod_key, elem_idx, mod_idx);

                        LOG_INF("Found model s/%d/%d (keys_cnt=%d), mod_ptr=%p", elem_idx, mod_idx, mod->keys_cnt, (void *)mod);

                        // Calculate actual size of keys array
                        size_t keys_size = mod->keys_cnt * sizeof(mod->keys[0]);

                        // Dump keys before clearing
                        LOG_INF("Keys BEFORE clear:");
                        for (int dbg_i = 0; dbg_i < mod->keys_cnt; dbg_i++)
                        {
                            LOG_INF("  keys[%d] = 0x%04x", dbg_i, mod->keys[dbg_i]);
                        }

                        // Clear existing bindings
                        for (int i = 0; i < mod->keys_cnt; i++)
                        {
                            mod->keys[i] = BT_MESH_KEY_UNUSED;
                        }

                        // Dump keys after clearing
                        LOG_INF("Keys AFTER clear:");
                        for (int dbg_i = 0; dbg_i < mod->keys_cnt; dbg_i++)
                        {
                            LOG_INF("  keys[%d] = 0x%04x", dbg_i, mod->keys[dbg_i]);
                        }

                        // Read and restore bindings from stored data
                        if (read_len > 0 && read_len <= keys_size)
                        {
                            memcpy(mod->keys, value_buf, read_len);

                            // Count restored bindings
                            int bind_count = 0;
                            for (int i = 0; i < mod->keys_cnt; i++)
                            {
                                if (mod->keys[i] != BT_MESH_KEY_UNUSED)
                                {
                                    bind_count++;
                                    LOG_INF("  Restored bind[%d]: 0x%04x", i, mod->keys[i]);
                                }
                            }

                            // Dump keys after restore
                            LOG_INF("Keys AFTER restore:");
                            for (int dbg_i = 0; dbg_i < mod->keys_cnt; dbg_i++)
                            {
                                LOG_INF("  keys[%d] = 0x%04x", dbg_i, mod->keys[dbg_i]);
                            }

                            LOG_INF("Successfully restored %d bindings for model s/%d/%d (mod_key=0x%04x)",
                                    bind_count, elem_idx, mod_idx, mod_key);
                            restored = true;
                        }
                        else
                        {
                            LOG_WRN("Invalid bind data length for s/%d/%d (read_len=%d, max=%zu)",
                                    elem_idx, mod_idx, read_len, keys_size);
                        }
                    }
                    else
                    {
                        // Unknown key type - skip
                        LOG_DBG("Unknown key type '%s' in %s", slash + 1, key_name);
                    }
                }
                // Handle bt/mesh/Net key - restore provisioned state and device key
                else if (strcmp(key_name, "bt/mesh/Net") == 0)
                {
                    LOG_INF("Restoring Net key '%s' (%d bytes)", key_name, read_len);

                    if (read_len >= sizeof(struct net_val))
                    {
                        struct net_val *net = (struct net_val *)value_buf;

                        // Restore primary address and device key
                        extern struct bt_mesh_net bt_mesh_net;
                        extern void bt_mesh_comp_provision(uint16_t addr);
                        extern void bt_mesh_key_assign(struct bt_mesh_key * key, const struct bt_mesh_key * src);

                        bt_mesh_comp_provision(net->primary_addr);
                        bt_mesh_key_assign(&bt_mesh.dev_key, &net->dev_key);

                        LOG_INF("Successfully restored Net state (addr=0x%04x)", net->primary_addr);
                        restored = true;
                    }
                    else
                    {
                        LOG_WRN("Invalid Net data length: %d bytes (expected %zu)", read_len, sizeof(struct net_val));
                    }
                }
                // Handle bt/mesh/IV key - restore IV Index and update state
                else if (strcmp(key_name, "bt/mesh/IV") == 0)
                {
                    LOG_INF("Restoring IV key '%s' (%d bytes)", key_name, read_len);

                    if (read_len >= sizeof(struct iv_val))
                    {
                        struct iv_val *iv = (struct iv_val *)value_buf;

                        extern struct bt_mesh_net bt_mesh_net;
                        extern atomic_t bt_mesh_flags;

                        bt_mesh.iv_index = iv->iv_index;
                        atomic_set_bit_to(bt_mesh.flags, BT_MESH_IVU_IN_PROGRESS, iv->iv_update);
                        bt_mesh.ivu_duration = iv->iv_duration;

                        LOG_INF("Successfully restored IV state (index=0x%08x, update=%d, duration=%d)",
                                iv->iv_index, iv->iv_update, iv->iv_duration);
                        restored = true;
                    }
                    else
                    {
                        LOG_WRN("Invalid IV data length: %d bytes (expected %zu)", read_len, sizeof(struct iv_val));
                    }
                }
                else if (strcmp(key_name, "bt/mesh/Seq") == 0)
                {
                    // Manually restore sequence number since callback is not available
                    LOG_INF("Restoring sequence number key '%s' (%d bytes)", key_name, read_len);

                    if (read_len >= 3)
                    {
                        // seq_val is 3 bytes (24-bit sequence number)
                        uint32_t seq = value_buf[0] | (value_buf[1] << 8) | (value_buf[2] << 16);

                        // Apply the same adjustment as in seq_set handler
                        if (CONFIG_BT_MESH_SEQ_STORE_RATE > 0)
                        {
                            seq += (CONFIG_BT_MESH_SEQ_STORE_RATE -
                                    (seq % CONFIG_BT_MESH_SEQ_STORE_RATE));
                            seq--;
                        }

                        // Restore to bt_mesh.seq
                        extern struct bt_mesh_net bt_mesh_net;
                        bt_mesh.seq = seq;

                        LOG_INF("Successfully restored sequence number: 0x%06x", bt_mesh.seq);
                        restored = true;
                    }
                    else
                    {
                        LOG_WRN("Invalid Seq data length: %d bytes (expected 3)", read_len);
                    }
                }
                // Handle other Mesh keys that are not explicitly handled above
                else
                {
                    LOG_DBG("Unhandled Mesh key '%s', skipping", key_name);
                }

                if (restored)
                {
                    loaded_count++;
                }
                else
                {
                    // Fallback: try to invoke settings framework callback
                    if (arg && arg->cb)
                    {
                        LOG_INF("Invoking callback for Mesh key '%s' (%d bytes)", key_name, read_len);
                        int ret = invoke_load_callback(arg, key_name, value_buf, read_len);
                        if (ret < 0)
                        {
                            LOG_WRN("Failed to load Mesh key '%s' via callback (ret=%d)", key_name, ret);
                        }
                        else
                        {
                            LOG_DBG("Loaded Mesh key '%s' via callback (%d bytes)", key_name, read_len);
                            loaded_count++;
                        }
                    }
                }
            }
            else
            {
                LOG_WRN("Failed to read value for key '%s' (len=%d)", key_name, read_len);
            }
        }
    }

    LOG_INF("Completed loading %d bt/mesh/* keys from FlashDB", loaded_count);
    return 0;
}
#endif

/**

@brief Load BT settings from NVDS (settings framework interface)
*/
static int nvds_csi_load(struct settings_store *cs, const struct settings_load_arg *arg)
{
    int ret;
    char key_buf[MAX_KEY_NAME_LEN];

    LOG_INF("nvds_csi_load called (cs=%p, arg=%p)", cs, arg);

    if (!cs)
    {
        LOG_ERR("Invalid cs parameter");
        return -EINVAL;
    }

    // Note: arg may be NULL during probe phase, but we still need to load data
    // The callback checks inside each loading function will handle NULL arg gracefully

    LOG_INF("Loading BT settings bundle...");

    // Load bundle from NVDS into cache
    ret = bt_bundle_load_from_nvds();
    if (ret != 0)
    {
        LOG_ERR("Failed to load bundle (ret=%d)", ret);
        // Don't return here, continue to load Mesh keys
    }

    // If arg is NULL or has no callback, skip the detailed loading
    // but still try to load Mesh keys via iteration
    if (!arg || !arg->cb)
    {
        LOG_INF("Probe phase detected (arg=%p), skipping detailed BT settings load", arg);
        // Continue to Mesh key loading below
    }
    else
    {
        LOG_INF("Loading detailed BT settings and invoking callbacks");

        // 1. Load bt/keys/<addr> entries (address-based)
        for (size_t i = 0; i < BT_MAX_PAIRED; i++)
        {
            struct bt_paired_device *dev = &nvds_backend.bt_cache.paired_devs[i];
            if (!dev->in_use)
            {
                continue; // Skip unused entries
            }
// Generate key name (e.g., bt/keys/AABBCCDDEEFF)
            ret = generate_addr_based_key(key_buf, sizeof(key_buf), BT_KEYS_BASE_KEY, &dev->addr);
            if (ret != 0)
            {
                continue;
            }
// Invoke framework callback with key-value pair
            ret = invoke_load_callback(arg, key_buf, dev->key, BT_KEYS_STORAGE_LEN);
            if (ret < 0)
            {
                LOG_WRN("Callback error for %s (ret=%d)", key_buf, ret);
// Don't stop loading on individual errors
            }
        }
// 2. Load bt/sc/<addr> entries (address-based)
        for (size_t i = 0; i < BT_MAX_PAIRED; i++)
        {
            struct bt_paired_device *dev = &nvds_backend.bt_cache.paired_devs[i];
            if (!dev->in_use)
            {
                continue; // Skip unused entries
            }
// Generate key name (e.g., bt/sc/AABBCCDDEEFF)
            ret = generate_addr_based_key(key_buf, sizeof(key_buf), BT_SC_BASE_KEY, &dev->addr);
            if (ret != 0)
            {
                continue;
            }
// Invoke framework callback with key-value pair
            ret = invoke_load_callback(arg, key_buf, dev->sc, BT_SC_STORAGE_LEN);
            if (ret < 0)
            {
                LOG_WRN("Callback error for %s (ret=%d)", key_buf, ret);
            }
        }
// 3. Load bt/name (string entry)
        // Skip if name is empty (first boot or cleared)
        if (nvds_backend.bt_cache.name[0] != '\0')
        {
            ret = invoke_load_callback(arg, BT_NAME_KEY,
                                       nvds_backend.bt_cache.name,
                                       strlen((const char *)nvds_backend.bt_cache.name));
            if (ret < 0)
            {
                LOG_WRN("Callback error for %s (ret=%d)", BT_NAME_KEY, ret);
            }
        }
        else
        {
            LOG_DBG("Skipping empty bt/name");
        }
// 4. Load bt/id/<index> entries (Bluetooth addresses)
        for (size_t i = 0; i < BT_ID_MAX; i++)
        {
            // Skip if address is all zeros (unused entry)
            bool is_zero = true;
            for (size_t j = 0; j < sizeof(bt_addr_le_t); j++)
            {
                if (((uint8_t *)&nvds_backend.bt_cache.id[i])[j] != 0)
                {
                    is_zero = false;
                    break;
                }
            }

            if (is_zero)
            {
                LOG_DBG("Skipping empty bt/id/%d", i);
                continue;
            }

            ret = generate_indexed_key(key_buf, sizeof(key_buf), BT_ID_KEY, i);
            if (ret != 0)
            {
                continue;
            }
// Each bt/id/<index> entry is a 6-byte Bluetooth address
            ret = invoke_load_callback(arg, key_buf, &nvds_backend.bt_cache.id[i], BT_ADDR_LEN);
            if (ret < 0)
            {
                LOG_WRN("Callback error for %s (ret=%d)", key_buf, ret);
            }
        }
// 5. Load bt/irk/<index> entries
        for (size_t i = 0; i < BT_ID_MAX; i++)
        {
            // Skip if IRK is all zeros (unused entry)
            bool is_zero = true;
            for (size_t j = 0; j < 16; j++)
            {
                if (nvds_backend.bt_cache.irk[i][j] != 0)
                {
                    is_zero = false;
                    break;
                }
            }

            if (is_zero)
            {
                LOG_DBG("Skipping empty bt/irk/%d", i);
                continue;
            }

            ret = generate_indexed_key(key_buf, sizeof(key_buf), BT_IRK_KEY, i);
            if (ret != 0)
            {
                continue;
            }
            ret = invoke_load_callback(arg, key_buf, nvds_backend.bt_cache.irk[i], 16);
            if (ret < 0)
            {
                LOG_WRN("Callback error for %s (ret=%d)", key_buf, ret);
            }
        }
        LOG_DBG("Completed BT settings load");
    }

    // Load all bt/mesh/* keys from FlashDB (Mesh CDB, AppKeys, etc.)
    LOG_INF("Loading Mesh keys from FlashDB...");
#ifdef CONFIG_BT_MESH
    ret = load_mesh_keys_from_flashdb(arg);
#endif
    if (ret != 0)
    {
        LOG_ERR("Failed to load Mesh keys (ret=%d)", ret);
        // Don't fail the entire load operation, just log the error
    }
    else
    {
        LOG_INF("Mesh keys loading completed");
    }

    return 0;
}

/**

@brief Start a save transaction (settings framework interface)
@param cs Settings store instance
@return 0 on success, negative error code on failure
*/
static int nvds_csi_save_start(struct settings_store *cs)
{
    if (!cs)
    {
        LOG_ERR("Invalid settings store pointer");
        return -EINVAL;
    }
    LOG_DBG("Starting save transaction");
    return 0;
}


/**

@brief Save a key-value pair (settings framework interface)
Handles all supported key types and updates in-memory cache
*/
static int nvds_csi_save(struct settings_store *cs, const char *name,
                         const char *value, size_t val_len)
{
    int r;
    if (!cs || !name)
    {
        LOG_ERR("Invalid parameters (cs=%p, name=%p, value=%p)", cs, name, value);
        r = -EINVAL;
        return r;
    }

    // Handle clear operation (value is NULL and val_len is 0)
    bool clear_operation = (value == NULL && val_len == 0);

    // Handle bt/all - clear all Bluetooth configurations
    if (strcmp(name, BT_ALL_KEY) == 0)
    {
        if (!clear_operation)
        {
            LOG_ERR("Only clear operation supported for %s", name);
            return -EINVAL;
        }
        LOG_DBG("Clearing all Bluetooth configurations...");

        // Clear all Bluetooth settings
        memset(&nvds_backend.bt_cache, 0, sizeof(nvds_backend.bt_cache));
        nvds_backend.bundle_dirty = true;
        LOG_DBG("Cleared all Bluetooth configurations");
        r = 0;
    }
// Handle bt/keys/<addr> entries
    else if (strstr(name, BT_KEYS_BASE_KEY) == name)
    {
        bt_addr_le_t addr;
        r = parse_bt_addr_from_key(name, BT_KEYS_BASE_KEY, &addr);
        if (r == 0)
        {
            if (clear_operation)
            {
                // Clear operation: zero out the key data for this device
                int idx = find_paired_dev_by_addr(&addr);
                if (idx >= 0)
                {
                    memset(nvds_backend.bt_cache.paired_devs[idx].key, 0, BT_KEYS_STORAGE_LEN);
                    nvds_backend.bundle_dirty = true;
                    r = 0;
                }
                else
                {
                    LOG_WRN("No device found to clear keys for: %s", name);
                    r = -ENOENT;
                }
            }
            else if (value == NULL)
            {
                LOG_ERR("Invalid NULL value for non-clear operation on %s", name);
                r = -EINVAL;
            }
            else if (val_len != BT_KEYS_STORAGE_LEN)
            {
                LOG_ERR("Invalid key length for %s (got %d, expected %d)",
                        name, val_len, BT_KEYS_STORAGE_LEN);
                r = -EINVAL;
            }
            else
            {
                r = settings_bt_keys_set(&addr, (const void *)value);
            }
        }
    }
// Handle bt/sc/<addr> entries
    else if (strstr(name, BT_SC_BASE_KEY) == name)
    {
        bt_addr_le_t addr;
        r = parse_bt_addr_from_key(name, BT_SC_BASE_KEY, &addr);
        if (r == 0)
        {
            if (clear_operation)
            {
                // Clear operation: zero out the security config for this device
                int idx = find_paired_dev_by_addr(&addr);
                if (idx >= 0)
                {
                    memset(nvds_backend.bt_cache.paired_devs[idx].sc, 0, BT_SC_STORAGE_LEN);
                    nvds_backend.bundle_dirty = true;
                    r = 0;
                }
                else
                {
                    LOG_WRN("No device found to clear security config for: %s", name);
                    r = -ENOENT;
                }
            }
            else if (value == NULL)
            {
                LOG_ERR("Invalid NULL value for non-clear operation on %s", name);
                r = -EINVAL;
            }
            else if (val_len != BT_SC_STORAGE_LEN)
            {
                LOG_ERR("Invalid SC length for %s (got %d, expected %d)",
                        name, val_len, BT_SC_STORAGE_LEN);
                r = -EINVAL;
            }
            else
            {
                r = settings_bt_sc_set(&addr, (const void *)value);
            }
        }
    }
// Handle bt/name entry
    else if (strcmp(name, BT_NAME_KEY) == 0)
    {
        if (clear_operation)
        {
            // Clear operation: zero out the device name
            memset(nvds_backend.bt_cache.name, 0, BT_DEVICE_NAME_MAX + 1);
            nvds_backend.bundle_dirty = true;
            r = 0;
        }
        else if (value == NULL)
        {
            LOG_ERR("Invalid NULL value for non-clear operation on %s", name);
            r = -EINVAL;
        }
        else if (val_len > BT_DEVICE_NAME_MAX)
        {
            LOG_ERR("Name too long (got %d, max %d)", val_len, BT_DEVICE_NAME_MAX);
            r = -EINVAL;
        }
        else
        {
            r = settings_bt_name_set(value);
        }
    }
// Handle bt/id/<index> entries (Bluetooth addresses)
    else if (strstr(name, BT_ID_KEY) == name)
    {
        size_t index;
        r = parse_key_index(name, BT_ID_KEY, &index);
        if (r == 0)
        {
            // Check current state of the ID entry before modification
            bool was_active = false;
            for (size_t j = 0; j < sizeof(bt_addr_le_t); j++)
            {
                if (((uint8_t *)&nvds_backend.bt_cache.id[index])[j] != 0)
                {
                    was_active = true;
                    break;
                }
            }

            if (clear_operation)
            {
                // Clear operation: zero out the identity address
                memset(&(nvds_backend.bt_cache.id[index]), 0, BT_ADDR_LEN);
                nvds_backend.bundle_dirty = true;

                // Decrement available count if entry was active
                if (was_active && nvds_backend.bt_cache.id_available > 0)
                {
                    nvds_backend.bt_cache.id_available--;
                    LOG_DBG("Deleted bt/id/%d, id_available=%d", index, nvds_backend.bt_cache.id_available);
                }
                r = 0;
            }
            else if (value == NULL)
            {
                LOG_ERR("Invalid NULL value for non-clear operation on %s", name);
                r = -EINVAL;
            }
            else if (val_len != BT_ADDR_LEN)
            {
                LOG_ERR("Invalid ID address length for %s (got %d, expected %d)",
                        name, val_len, BT_ADDR_LEN);
                r = -EINVAL;
            }
            else
            {
                bt_addr_le_t addr;
                memcpy(&addr, value, BT_ADDR_LEN);

                // Check if new address is non-zero (active)
                bool is_active = false;
                for (size_t j = 0; j < sizeof(bt_addr_le_t); j++)
                {
                    if (((uint8_t *)&addr)[j] != 0)
                    {
                        is_active = true;
                        break;
                    }
                }

                // Set the new address
                memcpy(&(nvds_backend.bt_cache.id[index]), &addr, BT_ADDR_LEN);
                nvds_backend.bundle_dirty = true;
                r = 0;

                // Increment available count if transitioning from inactive to active
                if (is_active && !was_active && nvds_backend.bt_cache.id_available < BT_ID_MAX)
                {
                    nvds_backend.bt_cache.id_available++;
                    LOG_DBG("Added bt/id/%d, id_available=%d", index, nvds_backend.bt_cache.id_available);
                }
            }
        }
    }
// Handle bt/irk/<index> entries
    else if (strstr(name, BT_IRK_KEY) == name)
    {
        size_t index;
        r = parse_key_index(name, BT_IRK_KEY, &index);
        if (r == 0)
        {
            if (clear_operation)
            {
                // Clear operation: zero out the IRK
                memset(nvds_backend.bt_cache.irk[index], 0, 16);
                nvds_backend.bundle_dirty = true;
                r = 0;
            }
            else if (value == NULL)
            {
                LOG_ERR("Invalid NULL value for non-clear operation on %s", name);
                r = -EINVAL;
            }
            else if (val_len != 16)
            {
                LOG_ERR("Invalid IRK length for %s (got %d, expected 16)",
                        name, val_len, 16);
                r = -EINVAL;
            }
            else
            {
                r = settings_bt_irk_set(index, (const void *)value);
            }
        }
    }
#ifdef CONFIG_BT_MESH
// Handle bt/mesh/* entries (store directly to FlashDB)
    else if (strstr(name, "bt/mesh") == name)
    {
        // Mesh data should be stored as individual keys in FlashDB
        if (clear_operation)
        {
            // Clear operation: delete the key from FlashDB
            LOG_DBG("Clearing Mesh key '%s'", name);
            r = 0; // FlashDB will handle deletion internally
        }
        else if (value == NULL)
        {
            LOG_ERR("Invalid NULL value for non-clear operation on %s", name);
            r = -EINVAL;
        }
        else
        {
            // Store Mesh data directly to FlashDB
            uint8_t nvds_ret = sifli_nvds_flash_adaptor_write(name, value, val_len);
            if (nvds_ret != 0)
            {
                LOG_ERR("Failed to store Mesh key '%s' (NVDS ret=0x%02X)", name, nvds_ret);
                r = -EIO;
            }
            else
            {
                LOG_DBG("Stored Mesh key '%s' (%d bytes)", name, val_len);
                r = 0;
            }
        }
    }
#endif
    else
    {
        LOG_ERR("Unknown BT key: %s", name);
        r = -ENOTSUP;
    }

    if (r == 0)
    {
        k_work_reschedule(&(nvds_backend.flush_work), K_MSEC(500));
    }
    return r;
}


/**

@brief End a save transaction (settings framework interface)
Commits changes to NVDS storage
@param cs Settings store instance
@return 0 on success, negative error code on failure
*/
static int nvds_csi_save_end(struct settings_store *cs)
{
    if (!cs)
    {
        LOG_ERR("Invalid settings store pointer");
        return -EINVAL;
    }
// Save entire bundle to NVDS
    return bt_bundle_save();
}

/**

@brief Get storage context (settings framework interface)
@param cs Settings store instance
@return Pointer to storage context
*/
static void *nvds_csi_storage_get(struct settings_store *cs)
{
    return (cs) ? &nvds_backend : NULL;
}

// Settings store interface definition - binds framework to our implementation
static const struct settings_store_itf nvds_store_itf =
{
    .csi_load = nvds_csi_load,
    .csi_save_start = nvds_csi_save_start,
    .csi_save = nvds_csi_save,
    .csi_save_end = nvds_csi_save_end,
    .csi_storage_get = nvds_csi_storage_get,
};

void flush_handler(struct k_work *work)
{
    nvds_store_itf.csi_save_end(&(nvds_backend.base_store));
}

// --------------------------
// Initialization
// --------------------------

/**

@brief Initialize NVDS Bluetooth settings backend
@return 0 on success, negative error code on failure
*/
int settings_backend_init(void)
{
    if (nvds_backend.is_initialized)
    {
        LOG_DBG("Backend already initialized");
        return 0;
    }
    LOG_INF("Initializing NVDS Bluetooth settings backend");
// Initialize NVDS adapter
    uint8_t ret = sifli_nvds_flash_adaptor_init();
    if (ret != 0)
    {
        LOG_ERR("Failed to initialize NVDS adapter (ret=0x%02X)", ret);
        return -EIO;
    }
// Initialize cache and backend structure
    memset(&nvds_backend.bt_cache, 0, sizeof(struct bt_settings_bundle));
    nvds_backend.base_store.cs_itf = &nvds_store_itf;
    nvds_backend.is_initialized = true;
// Register with Zephyr settings framework as both source and destination
    settings_src_register(&nvds_backend.base_store);
    settings_dst_register(&nvds_backend.base_store);
    k_work_init_delayable(&nvds_backend.flush_work, flush_handler);
    LOG_INF("NVDS Bluetooth settings backend initialized successfully");
    return 0;
}

