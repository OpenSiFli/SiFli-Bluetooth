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
static ssize_t settings_read_callback(void *arg, void *buf, size_t len)
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

/**

@brief Load settings and invoke callbacks (settings framework interface)
Loads all keys from cache and passes them to the framework
*/
static int nvds_csi_load(struct settings_store *cs, const struct settings_load_arg *arg)
{
    int ret;
    char key_buf[MAX_KEY_NAME_LEN];
    if (!cs || !arg)
    {
        LOG_ERR("Invalid parameters (cs=%p, arg=%p, cb=%p)", cs, arg, arg->cb);
        return -EINVAL;
    }
    LOG_DBG("Loading BT settings and invoking callbacks");
// Load bundle from NVDS into cache
    ret = bt_bundle_load_from_nvds();
    if (ret != 0)
    {
        LOG_ERR("Failed to load bundle (ret=%d)", ret);
        return ret;
    }
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
    ret = invoke_load_callback(arg, BT_NAME_KEY,
                               nvds_backend.bt_cache.name,
                               strlen((const char *)nvds_backend.bt_cache.name));
    if (ret < 0)
    {
        LOG_WRN("Callback error for %s (ret=%d)", BT_NAME_KEY, ret);
    }
// 4. Load bt/id/<index> entries (Bluetooth addresses)
    for (size_t i = 0; i < BT_ID_MAX; i++)
    {
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

