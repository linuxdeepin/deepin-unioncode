/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuchanghui<liuchanghui@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
doc:
https://www.x.org/docs/Xext/recordlib.pdf

build:
gcc -g -O0 -shared -fPIC -D_GNU_SOURCE -o x11preload.so x11_preload.c -W -Wall -L/usr/X11R6/lib -lX11 -lXtst -ldl -lpthread
 * */

#include <X11/Xlibint.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include "x11preload.h"
#ifdef SIMPLE_X11_HOOK
#include <xcb/xcb.h>
#else
#include <X11/keysym.h>
#include <X11/extensions/record.h>
#include <X11/extensions/XTest.h>
#endif

#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <pthread.h>

#include "shared_mem_dump.h"
#include "event_man.h"

static bool g_debug = false;
static char g_x11_event_flag[MapNotify] = {0}; /*form Error to MapNotify*/

inline long my_dump(int type, const char* msg, int len)
{
    MemoryDumper* dump = get_memory_dumper();
    if (dump) {
        record_event_simple(dump, DUMP_REASON_x11 + type, msg, len);
        return 0;
    }
    else {
        return syscall(SYS_dump_x11, type, msg, len, 0, 1);
    }
}

#ifdef SIMPLE_X11_HOOK

// https://github.com/freedesktop-unofficial-mirror/xorg__app__xinput
// https://www.clearchain.com/blog/posts/xinput-1-xinput-2-conversion-guide
// XQueryExtension(display, "XInputExtension", &extension, &event, &error)) {
//
static void print_deviceevent(XIDeviceEvent* event)
{
    double *val;
    int i;

    printf("    device: %d (%d)\n", event->deviceid, event->sourceid);
    printf("    detail: %d\n", event->detail);
    switch(event->evtype) {
        case XI_KeyPress:
        case XI_KeyRelease:
            printf("    flags: %s\n", (event->flags & XIKeyRepeat) ?  "repeat" : "");
            break;
#if HAVE_XI21
        case XI_ButtonPress:
        case XI_ButtonRelease:
        case XI_Motion:
            printf("    flags: %s\n", (event->flags & XIPointerEmulated) ?  "emulated" : "");
            break;
#endif
    }

    printf("    root: %.2f/%.2f\n", event->root_x, event->root_y);
    printf("    event: %.2f/%.2f\n", event->event_x, event->event_y);

    printf("    buttons:");
    for (i = 0; i < event->buttons.mask_len * 8; i++)
        if (XIMaskIsSet(event->buttons.mask, i))
            printf(" %d", i);
    printf("\n");

    printf("    modifiers: locked %#x latched %#x base %#x effective: %#x\n",
            event->mods.locked, event->mods.latched,
            event->mods.base, event->mods.effective);
    printf("    group: locked %#x latched %#x base %#x effective: %#x\n",
            event->group.locked, event->group.latched,
            event->group.base, event->group.effective);
    printf("    valuators:\n");

    val = event->valuators.values;
    for (i = 0; i < event->valuators.mask_len * 8; i++)
        if (XIMaskIsSet(event->valuators.mask, i))
            printf("        %i: %.2f\n", i, *val++);

    printf("    windows: root 0x%lx event 0x%lx child 0x%lx\n",
            event->root, event->event, event->child);
}

static XGenericEvent g_prev_ge;

Bool (* Real_XGetEventData)(Display *display, XGenericEventCookie *cookie) = nullptr;

Bool XGetEventData(Display *display, XGenericEventCookie *cookie)
{
    char info[EVENT_EXTRA_INFO_SIZE];
    int len = 0;
    BOOL ret = 0;
    if (NULL == Real_XGetEventData){
        *(void**)&Real_XGetEventData = dlsym(RTLD_NEXT, "XGetEventData");
    }
    if (NULL != Real_XGetEventData) {
        ret = Real_XGetEventData(display, cookie);
    }
    if (!ret) {
        return 0;
    }

    if (cookie->serial != g_prev_ge.serial ||
        cookie->evtype != g_prev_ge.evtype ||
        cookie->extension != g_prev_ge.extension) {
        return ret;
    }

    XIDeviceEvent* dev = (XIDeviceEvent*)cookie->data;
    switch (cookie->evtype) {
    case KeyPress:
    case KeyRelease:
        {
            len = snprintf(info, sizeof(info),
                    "win=%lx,key=%d", dev->event, dev->detail);
        }
        break;
    case ButtonPress:
    case ButtonRelease:
        {
            len = snprintf(info, sizeof(info), "win=%lx,x=%.2f,y=%.2f",
                    dev->event, dev->event_x, dev->event_y);
        }
        break;
    default:
        break;
    }
    if (len > 0 && g_x11_event_flag[cookie->evtype]) {
        long res = my_dump(cookie->evtype, info, len + 1);
        if (g_debug) {
            printf("X11 XNextEvent GenericEvent %d,%s->%ld\n",
                    cookie->evtype, info, res);
        }
        len = 0; //reset for avoiding dump twice
    }

    return ret;
}

int (*Real_XNextEvent)(Display *display, XEvent *event_return) = NULL;
int XNextEvent(Display *display, XEvent *event_return)
{
    char info[EVENT_EXTRA_INFO_SIZE];
    int len = 0;
    int ret = 0;
    if (NULL == Real_XNextEvent){
        *(void**)&Real_XNextEvent = dlsym(RTLD_NEXT, "XNextEvent");
    }
    if (NULL != Real_XNextEvent) {
        event_return->type = 0;
        ret = Real_XNextEvent(display, event_return);
    }

    switch (event_return->type) {
    case GenericEvent:
        {
            XGenericEvent* ge = (XGenericEvent*)event_return;
            // store till the user call XGetEventData(ge->display, xcookie);
            // so we can get detail info in XEvent.xcookie.data
            memcpy(&g_prev_ge, ge, sizeof(*ge));
        }
        break;
    case KeyPress:
    case KeyRelease:
        {
            XKeyEvent* key = (XKeyEvent*)event_return;
            len = snprintf(info, sizeof(info),
                    "win=%lx,key=%d", key->window, key->keycode);
        }
        break;
    case ButtonPress:
    case ButtonRelease:
        {
            XButtonEvent* btn = (XButtonEvent*)event_return;
            len = snprintf(info, sizeof(info),
                    "win=%lx,x=%d,y=%d", btn->window, btn->x, btn->y);
        }
        break;
#if 0
    case MotionNotify:
        {
            XMotionEvent* motion = (XMotionEvent*)event_return;
            x = motion->x;
            y = motion->y;
            w = motion->window;
        }
        break;
#endif
    case FocusIn:
    case FocusOut:
        {
            XFocusChangeEvent* focus = (XFocusChangeEvent*)event_return;
            len = snprintf(info, sizeof(info), "win=%lx", focus->window);
        }
        break;
    case CreateNotify:
        {
            XCreateWindowEvent* cr = (XCreateWindowEvent*)event_return;
            len = snprintf(info, sizeof(info),
                    "win=%lx,x=%d,y=%d,w=%d,h=%d",
                    cr->window, cr->x, cr->y, cr->width, cr->height);
        }
        break;
    case DestroyNotify:
        {
            XDestroyWindowEvent* des = (XDestroyWindowEvent*)event_return;
            len = snprintf(info, sizeof(info), "win=%lx", des->window);
        }
        break;
    case UnmapNotify:
        {
            XUnmapEvent* unmap = (XUnmapEvent*)event_return;
            len = snprintf(info, sizeof(info), "win=%lx", unmap->window);
        }
        break;
    case MapNotify:
        {
            XMapEvent* map = (XMapEvent*)event_return;
            len = snprintf(info, sizeof(info), "win=%lx", map->window);
        }
        break;
    default:
        break;
    }

    if (len > 0 && g_x11_event_flag[event_return->type]) {
        long res = my_dump(event_return->type, info, len + 1);
        if (g_debug) {
            printf("X11 XNextEvent %d,%s->%ld\n",
                    event_return->type, info, res);
        }
    }

    return ret;
}

// copy from <xcb/xinput.h>, at libxcb-xinput-dev 1.13

typedef int32_t xcb_input_fp1616_t;
typedef uint16_t xcb_input_device_id_t;
typedef struct xcb_input_modifier_info_t {
    uint32_t base;
    uint32_t latched;
    uint32_t locked;
    uint32_t effective;
}xcb_input_modifier_info_t;

typedef struct xcb_input_group_info_t {
    uint8_t base;
    uint8_t latched;
    uint8_t locked;
    uint8_t effective;
}xcb_input_group_info_t;

typedef struct xcb_input_button_press_event_t {
    uint8_t                   response_type;
    uint8_t                   extension;
    uint16_t                  sequence;
    uint32_t                  length;
    uint16_t                  event_type;
    xcb_input_device_id_t     deviceid;
    xcb_timestamp_t           time;
    uint32_t                  detail;
    xcb_window_t              root;
    xcb_window_t              event;
    xcb_window_t              child;
    uint32_t                  full_sequence;
    xcb_input_fp1616_t        root_x;
    xcb_input_fp1616_t        root_y;
    xcb_input_fp1616_t        event_x;
    xcb_input_fp1616_t        event_y;
    uint16_t                  buttons_len;
    uint16_t                  valuators_len;
    xcb_input_device_id_t     sourceid;
    uint8_t                   pad0[2];
    uint32_t                  flags;
    xcb_input_modifier_info_t mods;
    xcb_input_group_info_t    group;
}xcb_input_button_press_event_t;

typedef struct xcb_input_key_press_event_t {
    uint8_t                   response_type;
    uint8_t                   extension;
    uint16_t                  sequence;
    uint32_t                  length;
    uint16_t                  event_type;
    xcb_input_device_id_t     deviceid;
    xcb_timestamp_t           time;
    uint32_t                  detail;
    xcb_window_t              root;
    xcb_window_t              event;
    xcb_window_t              child;
    uint32_t                  full_sequence;
    xcb_input_fp1616_t        root_x;
    xcb_input_fp1616_t        root_y;
    xcb_input_fp1616_t        event_x;
    xcb_input_fp1616_t        event_y;
    uint16_t                  buttons_len;
    uint16_t                  valuators_len;
    xcb_input_device_id_t     sourceid;
    uint8_t                   pad0[2];
    uint32_t                  flags;
    xcb_input_modifier_info_t mods;
    xcb_input_group_info_t    group;
}xcb_input_key_press_event_t;

static inline int fixed1616ToInt(xcb_input_fp1616_t val)
{
    return int(double(val) / 0x10000);
}

xcb_generic_event_t* (*Real_xcb_wait_for_event)(xcb_connection_t *c) = nullptr;
xcb_generic_event_t* xcb_wait_for_event(xcb_connection_t *c)
{
    char info[EVENT_EXTRA_INFO_SIZE];
    int len = 0;
    xcb_generic_event_t* ret = nullptr;
    if (NULL == Real_xcb_wait_for_event){
        *(void**)&Real_xcb_wait_for_event = dlsym(RTLD_NEXT, "xcb_wait_for_event");
    }
    if (NULL != Real_xcb_wait_for_event) {
        ret = Real_xcb_wait_for_event(c);
    }
    if (nullptr == ret)
    {
        return nullptr;
    }

    switch (ret->response_type & ~0x80) {
    case XCB_GE_GENERIC:
        {
            // xinput extension, refer to QXcbWindow::handleXIMouseEvent at :
            // https://code.woboq.org/qt5/qtbase/src/plugins/platforms/xcb/qxcbwindow.cpp.html#2099
            xcb_ge_generic_event_t* ge = (xcb_ge_generic_event_t*)ret;
            switch (ge->event_type) {
            case KeyPress:
            case KeyRelease:
                if (g_x11_event_flag[ge->event_type]) {
                    xcb_input_key_press_event_t* key = (xcb_input_key_press_event_t*)ge;
                    len = snprintf(info, sizeof(info),
                            "win=%x,key=%d", key->event, key->detail);
                }
                break;
            case ButtonPress:
            case ButtonRelease:
                if (g_x11_event_flag[ge->event_type]) {
                    xcb_input_button_press_event_t* btn = (xcb_input_button_press_event_t*)ge;
                    int x = fixed1616ToInt(btn->event_x);
                    int y = fixed1616ToInt(btn->event_y);
                    len = snprintf(info, sizeof(info),
                            "win=%x,x=%d,y=%d", btn->event, x, y);
                }
                break;
            default:
                break;
            }
            if (len > 0) {
                long res = my_dump(ge->event_type, info, len + 1);
                if (g_debug) {
                    printf("X11 xcb_wait_for_event GenericEvent:%d,ext=%d,%s->%ld\n",
                            ge->event_type, ge->extension, info, res);
                }
                len = 0; //reset for avoiding dump twice
            }
        }
        break;
    case XCB_KEY_PRESS:
    case XCB_KEY_RELEASE:
        {
            xcb_key_press_event_t* key = (xcb_key_press_event_t*)ret;
            len = snprintf(info, sizeof(info),
                    "win=%x,key=%d", key->event, key->detail);
        }
        break;
    case XCB_BUTTON_PRESS:
    case XCB_BUTTON_RELEASE:
        {
            xcb_button_press_event_t* btn = (xcb_button_press_event_t*)ret;
            int x = fixed1616ToInt(btn->event_x);
            int y = fixed1616ToInt(btn->event_y);
            len = snprintf(info, sizeof(info),
                    "win=%x,x=%d,y=%d", btn->event, x, y);
        }
        break;
    case XCB_FOCUS_IN:
    case XCB_FOCUS_OUT:
        {
            xcb_focus_in_event_t* focus = (xcb_focus_in_event_t*)ret;
            len = snprintf(info, sizeof(info), "win=%x", focus->event);
        }
        break;
    case XCB_CREATE_NOTIFY:
        {
            xcb_create_notify_event_t* cr = (xcb_create_notify_event_t*)ret;
            len = snprintf(info, sizeof(info),
                    "win=%x,x=%d,y=%d,w=%d,h=%d",
                    cr->window, cr->x, cr->y, cr->width, cr->height);
        }
        break;
    case XCB_DESTROY_NOTIFY:
        {
            xcb_destroy_notify_event_t* des = (xcb_destroy_notify_event_t*)ret;
            len = snprintf(info, sizeof(info), "win=%x", des->event);
        }
        break;
    case XCB_UNMAP_NOTIFY:
        {
            xcb_unmap_notify_event_t* map = (xcb_unmap_notify_event_t*)ret;
            len = snprintf(info, sizeof(info), "win=%x", map->event);
        }
        break;
    case XCB_MAP_NOTIFY:
        {
            xcb_map_notify_event_t* map = (xcb_map_notify_event_t*)ret;
            len = snprintf(info, sizeof(info), "win=%x", map->event);
        }
        break;
    default:
        break;
    }
    if (len > 0 && g_x11_event_flag[ret->response_type]) {
        long res = my_dump(ret->response_type, info, len + 1);
        if (g_debug) {
            printf("X11 xcb_wait_for_event %d,%s->%ld\n",
                    ret->response_type, info, res);
        }
    }

    return ret;
}

#else

typedef struct tagCallbackClosure {
    Display *ctrlDisplay;
    Display *dataDisplay;
    int curX;
    int curY;
}CallbackClosure;

typedef union tagXRecordDatum{
    unsigned char type;
    xEvent event;
    xResourceReq req;
    xGenericReply reply;
    xError error;
    xConnSetupPrefix setup;
} XRecordDatum;

static bool                 g_inited = 0;
static CallbackClosure      g_userData;
static XRecordContext       g_recContext;
static XRecordClientSpec    g_recClientSpec;

static int setupRecordExtension(void) {
    int ver, ver2;
    XRecordRange* g_recRange[2];
    XSynchronize(g_userData.ctrlDisplay, True);

    // Record extension exists?
    if (!XRecordQueryVersion(g_userData.ctrlDisplay, &ver, &ver2)) {
        return -1;
    }

    g_recRange[0] = XRecordAllocRange ();
    g_recRange[1] = XRecordAllocRange ();
    if (!g_recRange[0] || !g_recRange[1]) {
        return -2;
    }
    g_recRange[0]->device_events.first = KeyPress;
    g_recRange[0]->device_events.last = ButtonRelease;
    g_recRange[0]->delivered_events.first = FocusIn;
    g_recRange[0]->delivered_events.last = FocusOut;

    g_recRange[1]->delivered_events.first = CreateNotify;
    g_recRange[1]->delivered_events.last = MapNotify;

    // one of the following constants:
    // XRecordCurrentClients, XRecordFutureClients, or XRecordAllClients.
    g_recClientSpec = XRecordAllClients;

    // Get context with our configuration
    g_recContext = XRecordCreateContext(g_userData.ctrlDisplay,
            0, &g_recClientSpec, 1, g_recRange, 2);
    if (!g_recContext) {
        return -3;
    }

    // TODO: how to free g_recRange ?
    return 0;
}

// Called from Xserver when new event occurs.
static void eventCallback(XPointer priv, XRecordInterceptData *hook)
{
    (void) priv;
    if (hook->category != XRecordFromServer) {
        XRecordFreeData(hook);
        return;
    }

    xEvent * event = (xEvent *)hook->data;

    // TODO: interrupt the tracee to dump its context!
    if (g_debug) {
        printf("x11 :%d, %d, %d\n",
                event->u.u.type,
                event->u.keyButtonPointer.rootX,
                event->u.keyButtonPointer.rootY);
    }

    XRecordFreeData(hook);
}

static int connect_display(const char* displayName)
{
    /* The recommended communication model for a Record application is to open two
     * connections to the server—one connection for recording control and one connection
     * for reading recorded protocol data.
     */
    if (NULL == (g_userData.ctrlDisplay = XOpenDisplay(displayName)) ) {
        return -4;
    }
    if (NULL == (g_userData.dataDisplay = XOpenDisplay(displayName)) ) {
        XCloseDisplay(g_userData.ctrlDisplay);
        g_userData.ctrlDisplay = NULL;
        return -5;
    }

    //TODO: You may want to set custom X error handler here

    return setupRecordExtension();
}

// NOTE: must call this function in a work thread
int start_record_x11(const char* displayName, const char* filter)
{
    if (filter && filter[0] > 0) {
        const char* walk = filter;
        char* stop = nullptr;
        while (*walk > 0) {
            int v = strtol(walk, &stop, 10);
            if (v >= 0 && v <= MapNotify) {
                g_x11_event_flag[v] = 1;
            }
            if (0 == *stop) break;
            walk = stop + 1;
        }
    }
    else {
        memset(g_x11_event_flag, 0x1, sizeof(g_x11_event_flag));
    }

    int ret = connect_display(displayName/*":0"*/);
    if (ret < 0) return ret;

    // block the caller and run into poll mode if successful
    if (!XRecordEnableContext(g_userData.dataDisplay,
                g_recContext, eventCallback, (XPointer)&g_userData)) {
        return -1;
    }
    g_inited = true;

    return 0;
}

void stop_record_x11(void)
{
    if (!g_inited) return;

    g_inited = false;
    XRecordDisableContext (g_userData.ctrlDisplay, g_recContext);
}

#endif /*end #ifdef SIMPLE_X11_HOOK*/

static void __attribute__((constructor)) init_process(void) {
    const char* filter = getenv("ST2_X11_FILTER");
    if (filter && filter[0] > 0) {
        const char* walk = filter;
        char* stop = nullptr;
        while (*walk > 0) {
            int v = strtol(walk, &stop, 10);
            if (v >= 0 && v <= MapNotify) {
                g_x11_event_flag[v] = 1;
            }
            if (0 == *stop) break;
            walk = stop + 1;
        }
    }
    else {
        memset(g_x11_event_flag, 0x1, sizeof(g_x11_event_flag));
    }

    if (getenv("ST2_DEBUG_X11") != nullptr) {
        g_debug = true;
        printf("x11 preload: ST2_X11_FILTER=%s\n", filter);
    }
}

static void __attribute__((destructor)) uninit_process(void) {
}
