#ifndef MIDDLEWARE_PARAMS_PARAMS_H
#define MIDDLEWARE_PARAMS_PARAMS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PARAM_NAME_MAX 16

typedef enum {
    PARAM_TYPE_FLOAT = 0,
    PARAM_TYPE_INT32,
} param_type_t;

typedef union {
    float f;
    int32_t i;
} param_value_t;

typedef struct {
    const char *name;
    param_type_t type;
    param_value_t value;
} param_entry_t;

int params_init(void);

/** Load persisted values. Returns 0 if the defaults are valid. */
int params_load(void);

/** Persist dirty values. Returns 0 on success. */
int params_save(void);

bool params_exists(const char *name);

float params_get_float(const char *name, float default_value);
int32_t params_get_int(const char *name, int32_t default_value);

int params_set_float(const char *name, float value);
int params_set_int(const char *name, int32_t value);

/** Iterate the table for the MAVLink parameter protocol. */
size_t params_count(void);
const param_entry_t *params_at(size_t index);
param_entry_t *params_find(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* MIDDLEWARE_PARAMS_PARAMS_H */
