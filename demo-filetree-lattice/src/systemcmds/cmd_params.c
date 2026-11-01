#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <middleware/log/log.h>
#include <middleware/params/params.h>
#include <systemcmds/commands.h>

static int params_command(int argc, char **argv)
{
    if (argc < 2 || strcmp(argv[1], "list") == 0) {
        for (size_t index = 0; index < params_count(); index++) {
            const param_entry_t *entry = params_at(index);

            if (entry == NULL) {
                continue;
            }

            if (entry->type == PARAM_TYPE_FLOAT) {
                FRT_LOG_INFO("%-16s %10.4f", entry->name, (double)entry->value.f);

            } else {
                FRT_LOG_INFO("%-16s %10ld", entry->name, (long)entry->value.i);
            }
        }

        return 0;
    }

    if (argc < 3) {
        FRT_LOG_INFO("usage: params <list|get|set> [name] [value]");
        return -1;
    }

    if (strcmp(argv[1], "get") == 0) {
        param_entry_t *entry = params_find(argv[2]);

        if (entry == NULL) {
            FRT_LOG_WARN("params: '%s' not found", argv[2]);
            return -1;
        }

        if (entry->type == PARAM_TYPE_FLOAT) {
            FRT_LOG_INFO("%s = %.4f", entry->name, (double)entry->value.f);

        } else {
            FRT_LOG_INFO("%s = %ld", entry->name, (long)entry->value.i);
        }

        return 0;
    }

    if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            FRT_LOG_INFO("usage: params set <name> <value>");
            return -1;
        }

        param_entry_t *entry = params_find(argv[2]);

        if (entry == NULL) {
            FRT_LOG_WARN("params: '%s' not found", argv[2]);
            return -1;
        }

        if (entry->type == PARAM_TYPE_FLOAT) {
            const float value = strtof(argv[3], NULL);

            if (params_set_float(argv[2], value) != 0) {
                return -1;
            }

            FRT_LOG_INFO("%s = %.4f", argv[2], (double)value);

        } else {
            const long value = strtol(argv[3], NULL, 10);

            if (params_set_int(argv[2], (int32_t)value) != 0) {
                return -1;
            }

            FRT_LOG_INFO("%s = %ld", argv[2], value);
        }

        return 0;
    }

    FRT_LOG_WARN("params: unknown action '%s'", argv[1]);
    return -1;
}

const shell_command_t cmd_params = {
    .name = "params",
    .usage = "params <list|get|set> [name] [value]",
    .help = "List, read or write parameters.",
    .function = params_command,
};
