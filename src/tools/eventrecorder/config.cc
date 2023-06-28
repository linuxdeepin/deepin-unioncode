// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"
#include <string.h>

/*third party lib*/
#include <json-c/json.h>


static char *alloc_json_string(struct json_object *o)
{
    const char *v;

    if (!json_object_is_type(o, json_type_string))
        return NULL;

    v = json_object_get_string(o);
    if (!v)
        return NULL;

    return strdup(v);
}

static int get_json_int(struct json_object *o, int *i, bool nonneg)
{
    if (!json_object_is_type(o, json_type_int))
        return -1;

    *i = json_object_get_int(o);

    if (*i == INT32_MIN)
        return -1;

    if (*i == INT32_MAX)
        return -1;

    if (nonneg && *i < 0)
        return -1;

    return 0;
}

static int get_json_boolean(struct json_object *o, bool *b)
{
    if (!json_object_is_type(o, json_type_boolean))
        return -1;

    *b = json_object_get_boolean(o);

    return 0;
}

static int read_map_config(struct json_object *root, DumpConfig& cfg)
{
    if (!json_object_is_type(root, json_type_array))
        return -1;

    int len = json_object_array_length(root);
    if (len < 1)
        return -1;

    for (int i = 0; i < len; i++) {
        struct json_object *v = json_object_array_get_idx(root, i);
        if (!v)
            return -1;

        cfg.modules.push_back(alloc_json_string(v));
    }

    return 0;
}

static int read_variable_item(struct json_object *root, DumpConfig& cfg)
{
    struct json_object_iterator it;
    struct json_object_iterator it_end;
    Variable tmp;

    tmp.max_size = 1024;
    tmp.is_pointer = false;

    for (it = json_object_iter_begin(root),
         it_end = json_object_iter_end(root);
         !json_object_iter_equal(&it, &it_end);
         json_object_iter_next(&it)) {

        struct json_object *v;
        const char* n = json_object_iter_peek_name(&it);
        if (!n)
            goto out_err;

        v = json_object_iter_peek_value(&it);
        if (!v)
            goto out_err;

        if (strcmp(n, "name") == 0) {
            tmp.sym_name = alloc_json_string(v);
            if (tmp.sym_name.empty())
                goto out_err;
        }
        else if (strcmp(n, "is_pointer") == 0) {
            get_json_boolean(v, &tmp.is_pointer);
        }
        else if (strcmp(n, "max_size") == 0) {
            int number = 0;
            get_json_int(v, &number, true);
            if (number > 0) {
                tmp.max_size = (number + 3) & (~3);
            }
        } else {
            printf("WARNING: ignoring unknown config item: %s\n", n);
        }
    }

    cfg.vars.push_back(tmp);

    return 0;

out_err:
    return -1;
}


static int read_global_var_config(struct json_object *root, DumpConfig& cfg)
{
    if (!json_object_is_type(root, json_type_array))
        return -1;

    int len = json_object_array_length(root);
    if (len < 1)
        return -1;

    for (int i = 0; i < len; i++) {
        struct json_object *v;

        v = json_object_array_get_idx(root, i);
        if (!v)
            return -1;

        if (read_variable_item(v, cfg) != 0)
            return -1;
    }

    return 0;
}

int load_config(DumpConfig& cfg)
{
    const char* user_file = nullptr;
    char cfg_file[256];
    struct json_object_iterator it;
    struct json_object_iterator it_end;

    cfg.max_heap_size = 0;
    cfg.dump_pthread_list = false;
    cfg.dump_robust_mutex_list = false;
    cfg.current_thread_only = false;
    cfg.max_stack_size = 32*1024; /*set zero to disable dump stack*/
    cfg.max_param_size = 256;
    cfg.compress_level = 3;
    cfg.shared_buffer_size = 4*1024*1024;
    cfg.max_dump_bytes = 4*1024*1024*1024LL;

    cfg.log_to_stdout = false;
    cfg.log_to_file = true;
    cfg.log_file_max_size = 20*1024*1024;
    cfg.log_flush_threshold = 0;
    cfg.log_debug = false;

    user_file = getenv("ST2_CONFIG_FILE");
    if (nullptr == user_file) {
        snprintf(cfg_file, sizeof(cfg_file), "%s/.config/emd.json", getenv("HOME"));
    }
    else {
        snprintf(cfg_file, sizeof(cfg_file), "%s", user_file);
    }
    struct json_object* root = json_object_from_file(cfg_file);
    if (!root) {
        fprintf(stderr, "failed to open or parse :%s, error:%s\n",
                cfg_file, strerror(errno));
        goto end;
    }

    for (it = json_object_iter_begin(root),
         it_end = json_object_iter_end(root);
         !json_object_iter_equal(&it, &it_end);
         json_object_iter_next(&it)) {

        struct json_object *v = nullptr;
        const char* n = json_object_iter_peek_name(&it);
        if (!n)
            break;

        v = json_object_iter_peek_value(&it);
        if (!v)
            break;

        if (strcmp(n, "dump_dir") == 0) {
            cfg.dump_dir = alloc_json_string(v);
        }
        else if (strcmp(n, "modules") == 0) {
            read_map_config(v, cfg);
        }
        else if (strcmp(n, "variables") == 0) {
            read_global_var_config(v, cfg);
        }
        else if (strcmp(n, "compress_level") == 0) {
            get_json_int(v, &cfg.compress_level, true);
        }
        else if (strcmp(n, "dump_pthread_list") == 0) {
            get_json_boolean(v, &cfg.dump_pthread_list);
        }
        else if (strcmp(n, "dump_robust_mutex_list") == 0) {
            get_json_boolean(v, &cfg.dump_robust_mutex_list);
        }
        else if (strcmp(n, "log_debug") == 0) {
            get_json_boolean(v, &cfg.log_debug);
        }
        else if (strcmp(n, "log_to_stdout") == 0) {
            get_json_boolean(v, &cfg.log_to_stdout);
        }
        else if (strcmp(n, "log_to_file") == 0) {
            get_json_boolean(v, &cfg.log_to_file);
        }
        else if (strcmp(n, "shared_buffer_size") == 0) {
            int number = 0;
            get_json_int(v, &number, true);
            if (number > 0) {
                cfg.shared_buffer_size = number*1024;
            }
        }
        else if (strcmp(n, "max_dump_bytes") == 0) {
            int number = 0;
            get_json_int(v, &number, true);
            if (number > 0) {
                cfg.max_dump_bytes = number*1024LL;
            }
        }
        else if (strcmp(n, "log_file_max_size") == 0) {
            int number = 0;
            get_json_int(v, &number, true);
            if (number > 0) {
                cfg.log_file_max_size = number*1024;
            }
        }
        else if (strcmp(n, "log_flush_threshold") == 0) {
            int number = 0;
            get_json_int(v, &number, true);
            if (number >= 0) {
                cfg.log_flush_threshold = number*1024;
            }
        }
        else if (strcmp(n, "mode") == 0) {
            get_json_int(v, (int*)&cfg.mode, true);
        }
        else {
            printf("WARNING: ignoring unknown config item: %s\n", n);
        }
    }

    json_object_put(root);

end:
    if (cfg.dump_dir.empty()) {
        cfg.dump_dir = getenv("HOME");
        cfg.dump_dir += "/.local/share/emd/";
    }
    else if (cfg.dump_dir[0] == '~') {
        cfg.dump_dir.replace(0, 1, getenv("HOME"));
    }

    return 0;
}


