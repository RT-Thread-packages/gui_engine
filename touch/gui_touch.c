/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-06-01     tyustli     the first version
 */
#include "touch.h"
#include <string.h>
#include <rtgui/event.h>
#include <rtgui/rtgui_server.h>

#ifndef PKG_TOUCH_SAMPLE_HZ
#define PKG_TOUCH_SAMPLE_HZ    (50)
#endif
/*Enable debug information*/
#define DBG_ENABLE

#ifdef  DBG_ENABLE
#define DBG_SECTION_NAME  "TOUCH"
#define DBG_LEVEL         DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>
#endif

static void post_down_event(rt_uint16_t x, rt_uint16_t y, rt_tick_t id)
{
    struct rtgui_event_mouse emouse;

    emouse.parent.sender = RT_NULL;
    emouse.wid = RT_NULL;

    emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
    emouse.button = RTGUI_MOUSE_BUTTON_LEFT | RTGUI_MOUSE_BUTTON_DOWN;
    emouse.x = x;
    emouse.y = y;
    emouse.ts = rt_tick_get();
    emouse.id = id;
    rtgui_server_post_event(&emouse.parent, sizeof(emouse));
}

static void post_motion_event(rt_uint16_t x, rt_uint16_t y, rt_tick_t id)
{
    struct rtgui_event_mouse emouse;

    emouse.parent.sender = RT_NULL;
    emouse.wid = RT_NULL;

    emouse.button = RTGUI_MOUSE_BUTTON_LEFT | RTGUI_MOUSE_BUTTON_DOWN;
    emouse.parent.type = RTGUI_EVENT_MOUSE_MOTION;
    emouse.x = x;
    emouse.y = y;
    emouse.ts = rt_tick_get();
    emouse.id = id;
    rtgui_server_post_event(&emouse.parent, sizeof(emouse));
}

static void post_up_event(rt_uint16_t x, rt_uint16_t y, rt_tick_t id)
{
    struct rtgui_event_mouse emouse;

    emouse.parent.sender = RT_NULL;
    emouse.wid = RT_NULL;

    emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
    emouse.button = RTGUI_MOUSE_BUTTON_LEFT | RTGUI_MOUSE_BUTTON_UP;
    emouse.x = x;
    emouse.y = y;
    emouse.ts = rt_tick_get();
    emouse.id = id;
    rtgui_server_post_event(&emouse.parent, sizeof(emouse));
}

#define THREAD_PRIORITY   25
#define THREAD_STACK_SIZE 1024
#define THREAD_TIMESLICE  5

static rt_thread_t  touch_thread;
static rt_sem_t     touch_sem;
static rt_device_t  dev;
static struct       rt_touch_data *read_data;

static void gui_touch_entry(void *parameter)
{
    static rt_tick_t emouse_id = 0;
    read_data = (struct rt_touch_data *)rt_malloc(sizeof(struct rt_touch_data));

    while (1)
    {
        rt_sem_take(touch_sem, RT_WAITING_FOREVER);

        if (rt_device_read(dev, 0, read_data, 1) == 1)
        {
            switch(read_data->event)
            {
            case RT_TOUCH_EVENT_UP:
                post_up_event(read_data->x_coordinate, read_data->y_coordinate, emouse_id);
                break;
            case RT_TOUCH_EVENT_DOWN:
                emouse_id = rt_tick_get();
                post_down_event(read_data->x_coordinate, read_data->y_coordinate, emouse_id);
                break;
            case RT_TOUCH_EVENT_MOVE:
                post_motion_event(read_data->x_coordinate, read_data->y_coordinate, emouse_id);
                break;
            default:
                break;
            }
        }

        rt_device_control(dev, RT_TOUCH_CTRL_ENABLE_INT, RT_NULL);
    }
}

static rt_err_t rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(touch_sem);
    rt_device_control(dev, RT_TOUCH_CTRL_DISABLE_INT, RT_NULL);

    return RT_EOK;
}

int gui_touch(const char* name, rt_uint16_t x, rt_uint16_t y)
{
    dev = rt_device_find(name);  /* you touch device name*/
    if (dev == RT_NULL)
    {
        LOG_E("can't find device:%s\n", name);
        return -1;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        LOG_E("open device failed!");
        return -1;
    }

    rt_device_control(dev, RT_TOUCH_CTRL_SET_X_RANGE, &x); /* if possible you can set your x y coordinate*/
    rt_device_control(dev, RT_TOUCH_CTRL_SET_Y_RANGE, &y);

    rt_device_set_rx_indicate(dev, rx_callback);
    touch_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_FIFO);

    if (touch_sem == RT_NULL)
    {
        LOG_E("create dynamic semaphore failed.\n");
        return -1;
    }

    touch_thread = rt_thread_create("gui_touch",
                                    gui_touch_entry,
                                    RT_NULL,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY,
                                    THREAD_TIMESLICE);

    if (touch_thread != RT_NULL)
        rt_thread_startup(touch_thread);

    return 0;
}
