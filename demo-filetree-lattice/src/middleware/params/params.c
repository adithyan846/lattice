#include <middleware/params/params.h>

#include <string.h>

#include <freertrtos/semphr.h>

#include <middleware/params/param_table.h>

static frt_mutex_t params_lock;
static bool params_dirty;

int params_init(void)
{
    params_lock = frt_mutex_create();
    params_dirty = false;
    return params_lock != NULL ? 0 : -1;
}

int params_load(void)
{
    /* TODO: read the persisted blob from hal_flash_read(HAL_FLASH_PARTITION_PARAMS)
     * and apply it over the defaults. The scaffold starts from defaults. */
    return 0;
}

int params_save(void)
{
    if (!params_dirty) {
        return 0;
    }

    /* TODO: serialise the table with a CRC and write it to the parameter
     * flash partition. Return -1 until that is implemented so callers do not
     * believe a value survived a reboot. */
    return -1;
}

param_entry_t *params_find(const char *name)
{
    if (name == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < param_table_count; i++) {
        if (strncmp(param_table[i].name, name, PARAM_NAME_MAX) == 0) {
            return &param_table[i];
        }
    }

    return NULL;
}

bool params_exists(const char *name)
{
    return params_find(name) != NULL;
}

float params_get_float(const char *name, float default_value)
{
    param_entry_t *entry = params_find(name);

    if (entry == NULL || entry->type != PARAM_TYPE_FLOAT) {
        return default_value;
    }

    return entry->value.f;
}

int32_t params_get_int(const char *name, int32_t default_value)
{
    param_entry_t *entry = params_find(name);

    if (entry == NULL || entry->type != PARAM_TYPE_INT32) {
        return default_value;
    }

    return entry->value.i;
}

int params_set_float(const char *name, float value)
{
    param_entry_t *entry = params_find(name);

    if (entry == NULL || entry->type != PARAM_TYPE_FLOAT) {
        return -1;
    }

    if (params_lock != NULL) {
        (void)frt_mutex_take(params_lock, 10);
    }

    entry->value.f = value;
    params_dirty = true;

    if (params_lock != NULL) {
        (void)frt_mutex_give(params_lock);
    }

    return 0;
}

int params_set_int(const char *name, int32_t value)
{
    param_entry_t *entry = params_find(name);

    if (entry == NULL || entry->type != PARAM_TYPE_INT32) {
        return -1;
    }

    if (params_lock != NULL) {
        (void)frt_mutex_take(params_lock, 10);
    }

    entry->value.i = value;
    params_dirty = true;

    if (params_lock != NULL) {
        (void)frt_mutex_give(params_lock);
    }

    return 0;
}

size_t params_count(void)
{
    return param_table_count;
}

const param_entry_t *params_at(size_t index)
{
    return index < param_table_count ? &param_table[index] : NULL;
}
