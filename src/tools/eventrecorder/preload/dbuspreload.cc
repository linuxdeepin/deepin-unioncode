// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dbus/dbus.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <memory.h>

#include "shared_mem_dump.h"
#include "event_man.h"

/* build:
gcc -g -O0 -o dbuspreload.so dbus_preload.c -fPIC -shared -D_GNU_SOURCE -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/include/dbus-1.0/ -ldbus-1 -ldl

  note:
  -D_GNU_SOURCE flag is specified to satisfy #ifdef conditions that allow us to use the
  RTLD_NEXT enum.
  The RTLD_NEXT enum tells the dynamic loader API that we want to return the next instance
  of the function associated with the second argument (in this case puts) in the load
  order.

  usage:
  LD_PRELOAD=libdbuspreload.so dbus-send

/usr/bin/dbus-send --system --print-reply --dest=org.freedesktop.DBus \
 / org.freedesktop.DBus.GetId

/usr/bin/dbus-send --system --print-reply --dest=org.freedesktop.NetworkManager \
    /org/freedesktop/NetworkManager org.freedesktop.NetworkManager.GetAllDevices
*/

static bool g_debug = false;
static char g_msg_flag[DBUS_MESSAGE_TYPE_SIGNAL+1] = {0};

//NOTE: snprintf not call new syscall, or dead lock!

static int dump_iter(DBusMessageIter* iter, char* buf, int size)
{
    char* walk = buf;
    int remain = size;
    do
    {
        int type = dbus_message_iter_get_arg_type(iter);
        if (type == DBUS_TYPE_INVALID) break;
        switch (type)
        {
            case DBUS_TYPE_STRING:
            case DBUS_TYPE_SIGNATURE:
            case DBUS_TYPE_OBJECT_PATH:
            {
                char *val = NULL;
                dbus_message_iter_get_basic(iter, &val);
                if (val) {
                    int len = snprintf(walk, remain, "%s,", val);
                    walk += len;
                    remain -= len;
                }
                break;
            }

            case DBUS_TYPE_INT16:
            {
                dbus_int16_t val = 0;
                dbus_message_iter_get_basic (iter, &val);
                int len = snprintf(walk, remain, "%d,", val);
                walk += len;
                remain -= len;
                break;
            }

            case DBUS_TYPE_UINT16:
            {
                dbus_uint16_t val = 0;
                dbus_message_iter_get_basic (iter, &val);
                int len = snprintf(walk, remain, "%u,", val);
                walk += len;
                remain -= len;
                break;
            }

            case DBUS_TYPE_INT32:
            {
                dbus_int32_t val = 0;
                dbus_message_iter_get_basic (iter, &val);
                int len = snprintf(walk, remain, "%d,", val);
                walk += len;
                remain -= len;
                break;
            }

            case DBUS_TYPE_UINT32:
            {
                dbus_uint32_t val = 0;
                dbus_message_iter_get_basic (iter, &val);
                int len = snprintf(walk, remain, "%u,", val);
                walk += len;
                remain -= len;
                break;
            }

            case DBUS_TYPE_INT64:
            {
                dbus_int64_t val = 0;
                dbus_message_iter_get_basic (iter, &val);
                int len = snprintf(walk, remain, "%ld,", val);
                walk += len;
                remain -= len;
                break;
            }

            case DBUS_TYPE_UINT64:
            {
                dbus_uint64_t val = 0;
                dbus_message_iter_get_basic (iter, &val);
                int len = snprintf(walk, remain, "%lu,", val);
                walk += len;
                remain -= len;
                break;
            }

            case DBUS_TYPE_DOUBLE:
            {
                double val = 0;
                dbus_message_iter_get_basic (iter, &val);
                int len = snprintf(walk, remain, "%f,", val);
                walk += len;
                remain -= len;
                break;
            }

            case DBUS_TYPE_BYTE:
            {
                unsigned char val = 0;
                dbus_message_iter_get_basic (iter, &val);
                int len = snprintf(walk, remain, "%u,", val);
                walk += len;
                remain -= len;
                break;
            }

            case DBUS_TYPE_BOOLEAN:
            {
                dbus_bool_t val = 0;
                dbus_message_iter_get_basic (iter, &val);
                int len = snprintf(walk, remain, "%s,", val ? "true" : "false");
                walk += len;
                remain -= len;
                break;
            }

            case DBUS_TYPE_VARIANT:
            {
                int len = snprintf(walk, remain, "variant,");
                walk += len;
                remain -= len;
                break;
            }
            case DBUS_TYPE_ARRAY:
            {
                int len = snprintf(walk, remain, "array,");
                walk += len;
                remain -= len;
                break;
            }
            case DBUS_TYPE_DICT_ENTRY:
            {
                int len = snprintf(walk, remain, "entry,");
                walk += len;
                remain -= len;
                break;
            }
            case DBUS_TYPE_STRUCT:
            {
                int len = snprintf(walk, remain, "struct,");
                walk += len;
                remain -= len;
                break;
            }
            default:
            break;
        }
    } while (dbus_message_iter_next (iter));

    return (walk - buf);
}

static int get_msg_detail(DBusMessage* msg, int* pmsg_type, char* buf, int size)
{
    const char* sender;
    const char* destination;
    int len = 0;
    int msg_type = dbus_message_get_type(msg);

    if (pmsg_type) {
        *pmsg_type = msg_type;
    }
    else {
        // FIXME: msg_type = DBUS_MESSAGE_TYPE_METHOD_RETURN;
    }
    if (!g_msg_flag[msg_type]) return 0;

    sender = dbus_message_get_sender(msg);
    destination = dbus_message_get_destination(msg);

    switch (msg_type)
    {
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
    case DBUS_MESSAGE_TYPE_SIGNAL: {
        const char* path = dbus_message_get_path(msg);
        const char* interface = dbus_message_get_interface(msg);
        const char* member = dbus_message_get_member(msg);
        if (sender) {
            len = snprintf(buf, size, "%s->%s,p=%s,i=%s,m=%s;",
                sender, destination != nullptr ? destination:"null",
                path, interface, member);
        }
        else {
            len = snprintf(buf, size, "d=%s,p=%s,i=%s,m=%s;",
                destination != nullptr ? destination:"null",
                path, interface, member);
        }
    }
    break;

    case DBUS_MESSAGE_TYPE_METHOD_RETURN: {
        DBusMessageIter msgIter;
        dbus_message_iter_init(msg, &msgIter);
        len = dump_iter(&msgIter, buf, size);
    }
    break;

    case DBUS_MESSAGE_TYPE_ERROR: {
        const char* err = dbus_message_get_error_name(msg);
        len = snprintf(buf, size, "%s->%s,e=%s;",
                sender != nullptr ? sender:"null",
                destination != nullptr ? destination:"null",
                err);
    }
    break;

    default:
    break;
    }

    return (len > 0) ? len : 0;
}

static void dump(DBusMessage* msg, DBusMessage* reply)
{
    char detail[EVENT_EXTRA_INFO_SIZE];
    char* walk = &detail[0];
    char* walk_end = walk + EVENT_EXTRA_INFO_SIZE;
    int length = 0;
    int msg_type = -1;

    if (msg) {
        walk += get_msg_detail(msg, &msg_type, walk, walk_end - walk);
    }
    if (reply) {
        walk += get_msg_detail(reply, nullptr, walk, walk_end - walk);
        if (msg_type < 0) msg_type = DBUS_MESSAGE_TYPE_METHOD_RETURN;
    }

    length = walk - &detail[0];
    if (length > 0) {
        MemoryDumper* dump = get_memory_dumper();
        if (dump) {
            record_event_simple(dump, DUMP_REASON_dbus + msg_type, detail, length);
        }
        else {
            syscall(SYS_dump_dbus, msg_type, detail, length, 0, 0);
        }
        if (g_debug) {
            // if (strstr(detail, "/lfs/idc300test/listener"))
            printf("dbus preload %d:%s\n", msg_type, detail);
        }
    }
}

typedef dbus_bool_t (*DBUS_CONNECTION_SEND_WITH_REPLY)(DBusConnection *connection,
                                 DBusMessage *message,
                                 DBusPendingCall **pending_return,
                                 int timeout_milliseconds);
DBUS_CONNECTION_SEND_WITH_REPLY real_dbus_connection_send_with_reply = NULL;

dbus_bool_t dbus_connection_send_with_reply(DBusConnection *connection,
                                 DBusMessage *message,
                                 DBusPendingCall **pending_return,
                                 int timeout_milliseconds)
{
    dbus_bool_t ret = 0;
    if (NULL == real_dbus_connection_send_with_reply) {
        real_dbus_connection_send_with_reply = (DBUS_CONNECTION_SEND_WITH_REPLY)dlsym(RTLD_NEXT,
                "dbus_connection_send_with_reply");
    }

    if (NULL != real_dbus_connection_send_with_reply) {
        ret = real_dbus_connection_send_with_reply(connection,
            message, pending_return, timeout_milliseconds);
        dump(message, nullptr);
    }

    return ret;
}

typedef DBusMessage* (*DBUS_PENDING_CALL_STEAL_REPLY)(DBusPendingCall* pending);

DBUS_PENDING_CALL_STEAL_REPLY real_dbus_pending_call_steal_reply = NULL;

DBusMessage* dbus_pending_call_steal_reply(DBusPendingCall* pending)
{
    DBusMessage* msg = NULL;

    if (NULL == real_dbus_pending_call_steal_reply) {
        real_dbus_pending_call_steal_reply = (DBUS_PENDING_CALL_STEAL_REPLY)dlsym(RTLD_NEXT,
                "dbus_pending_call_steal_reply");
    }
    if (NULL != real_dbus_pending_call_steal_reply) {
        msg = real_dbus_pending_call_steal_reply(pending);
        if (msg) {
            dump(nullptr, msg);
        }
    }

    return msg;
}

typedef DBusMessage* (*DBUS_CONNECTION_POP_MESSAGE)(DBusConnection *connection);
DBUS_CONNECTION_POP_MESSAGE real_dbus_connection_pop_message = NULL;

DBusMessage* dbus_connection_pop_message (DBusConnection *connection)
{
    DBusMessage* msg = NULL;

    if (NULL == real_dbus_connection_pop_message) {
        real_dbus_connection_pop_message = (DBUS_CONNECTION_POP_MESSAGE)dlsym(RTLD_NEXT,
                "dbus_connection_pop_message");
    }
    if (NULL != real_dbus_connection_pop_message) {
        msg = real_dbus_connection_pop_message(connection);
        if (msg) {
            dump(nullptr, msg);
        }
    }

    return msg;
}

typedef dbus_bool_t (*DBUS_CONNECTION_SEND)(DBusConnection *connection,
                      DBusMessage    *message, dbus_uint32_t  *serial);
DBUS_CONNECTION_SEND real_dbus_connection_send = NULL;
dbus_bool_t dbus_connection_send(DBusConnection *connection,
                      DBusMessage *message, dbus_uint32_t *serial)
{
    dbus_bool_t ret = 0;

    if (NULL == real_dbus_connection_send) {
        real_dbus_connection_send = (DBUS_CONNECTION_SEND)dlsym(RTLD_NEXT,
                "dbus_connection_send");
    }
    if (NULL != real_dbus_connection_send) {
        ret = real_dbus_connection_send(connection, message, serial);
        if (ret) {
            dump(message, nullptr);
        }
    }

    return ret;
}

typedef DBusMessage* (*DBUS_CONNECTION_SEND_WITH_REPLY_AND_BLOCK)(
        DBusConnection     *connection,
        DBusMessage        *message,
        int                 timeout_milliseconds,
        DBusError          *error);


// NOTE: dbus_connection_send_with_reply_and_block will call dbus_connection_send_with_reply internally!
#if 0
DBUS_CONNECTION_SEND_WITH_REPLY_AND_BLOCK real_dbus_connection_send_with_reply_and_block = NULL;
DBusMessage* dbus_connection_send_with_reply_and_block(
        DBusConnection     *connection,
        DBusMessage        *message,
        int                 timeout_milliseconds,
        DBusError          *error)
{
    DBusMessage* reply = NULL;

    if (NULL == real_dbus_connection_send_with_reply_and_block) {
        real_dbus_connection_send_with_reply_and_block =
            (DBUS_CONNECTION_SEND_WITH_REPLY_AND_BLOCK)dlsym(RTLD_NEXT,
                "dbus_connection_send_with_reply_and_block");
    }
    if (NULL != real_dbus_connection_send_with_reply_and_block) {
        reply = real_dbus_connection_send_with_reply_and_block(
                connection, message, timeout_milliseconds, error);

        if (reply) {
            dump(message, reply);
        }
    }

    return reply;
}
#endif

static void __attribute__((constructor)) init_process(void) {
    const char* filter = getenv("ST2_DBUS_FILTER");
    if (filter != nullptr) {
        const char* walk = filter;
        char* stop = nullptr;
        while (*walk > 0) {
            int v = strtol(walk, &stop, 10);
            if (v >= DBUS_MESSAGE_TYPE_METHOD_CALL &&
                v <= DBUS_MESSAGE_TYPE_SIGNAL) {
                g_msg_flag[v] = 1;
            }
            if (0 == *stop) break;
            walk = stop + 1;
        }
    }
    else {
        memset(g_msg_flag, 1, sizeof(g_msg_flag));
    }
    if (getenv("ST2_DEBUG_DBUS") != nullptr) {
        g_debug = true;
        printf("dbus preload: ST2_DBUS_FILTER=%s\n", filter);
    }
}
