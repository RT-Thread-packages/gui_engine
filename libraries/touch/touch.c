/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-08     Zhangyihong  the first version
 * 2019-05-06     jhb          Adaptation package
 */
#include "touch.h"
#include <string.h>
#ifdef PKG_USING_TOUCH
#ifdef PKG_USING_GUIENGINE
#include <rtgui/event.h>
#include <rtgui/rtgui_server.h>
#endif

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
/*Global device list */
static rt_list_t driver_list;
/*Global semaphore for thread synchronization*/
static rt_sem_t  touch_isr_sem;

void rt_touch_drivers_register(touch_drv_t drv)
{
    rt_list_insert_before(&driver_list, &drv->list);
}

/*Release semaphore interface*/
void rt_touch_release_sem(void)
{
    rt_sem_release(touch_isr_sem);
}

static void post_down_event(rt_uint16_t x, rt_uint16_t y, rt_tick_t ts)
{
#ifdef PKG_USING_GUIENGINE
    struct rtgui_event_mouse emouse;

    emouse.parent.sender = RT_NULL;
    emouse.wid = RT_NULL;

    emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
    emouse.button = RTGUI_MOUSE_BUTTON_LEFT | RTGUI_MOUSE_BUTTON_DOWN;
    emouse.x = x;
    emouse.y = y;
    emouse.ts = rt_tick_get();
    emouse.id = ts;
    rtgui_server_post_event(&emouse.parent, sizeof(emouse));

#endif
}

static void post_motion_event(rt_uint16_t x, rt_uint16_t y, rt_tick_t ts)
{
#ifdef PKG_USING_GUIENGINE
    struct rtgui_event_mouse emouse;

    emouse.parent.sender = RT_NULL;
    emouse.wid = RT_NULL;

    emouse.button = RTGUI_MOUSE_BUTTON_LEFT | RTGUI_MOUSE_BUTTON_DOWN;
    emouse.parent.type = RTGUI_EVENT_MOUSE_MOTION;
    emouse.x = x;
    emouse.y = y;
    emouse.ts = rt_tick_get();
    emouse.id = ts;
    rtgui_server_post_event(&emouse.parent, sizeof(emouse));

#endif
}

static void post_up_event(rt_uint16_t x, rt_uint16_t y, rt_tick_t ts)
{
#ifdef PKG_USING_GUIENGINE
    struct rtgui_event_mouse emouse;

    emouse.parent.sender = RT_NULL;
    emouse.wid = RT_NULL;

    emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
    emouse.button = RTGUI_MOUSE_BUTTON_LEFT | RTGUI_MOUSE_BUTTON_UP;
    emouse.x = x;
    emouse.y = y;
    emouse.ts = rt_tick_get();
    emouse.id = ts;
    rtgui_server_post_event(&emouse.parent, sizeof(emouse));

#endif
}

static void touch_thread_entry(void *parameter)
{
    touch_drv_t touch = (touch_drv_t)parameter;
    struct touch_message msg;
    rt_tick_t emouse_id = 0;
    touch->ops->isr_enable(RT_TRUE);
    while (1)
    {

        if (rt_sem_take(touch_isr_sem, 10) != RT_EOK)
        {
            continue;
        }
        while(touch->ops->read_point(&msg) == RT_EOK)
        {
            switch (msg.event)
            {
            case TOUCH_EVENT_UP:
                post_up_event(msg.x, msg.y, emouse_id);
                break;
            case TOUCH_EVENT_DOWN:
                emouse_id = rt_tick_get();
                post_down_event(msg.x, msg.y, emouse_id);
                break;
            case TOUCH_EVENT_MOVE:
                post_motion_event(msg.x, msg.y, emouse_id);
                break;
            default:
                break;
            }

#ifdef DBG_ENABLE
            LOG_I("x:[%3d] y:[%3d]",msg.x,msg.y);
            LOG_E("Please turn off the debug option after the test passes, otherwise it will affect the performance.");
#endif
            rt_thread_delay(RT_TICK_PER_SECOND / PKG_TOUCH_SAMPLE_HZ);
        }
        touch->ops->isr_enable(RT_TRUE);
    }
}

static int rt_touch_driver_init(void)
{
    rt_list_init(&driver_list);
    return 0;
}
INIT_BOARD_EXPORT(rt_touch_driver_init);

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;
static int rt_touch_thread_init(void)
{
    rt_list_t *l;
    touch_drv_t current_driver;
    rt_thread_t tid = RT_NULL;
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(PKG_TOUCH_I2C_NAME);
    RT_ASSERT(i2c_bus);
    current_driver = RT_NULL;
    if (rt_device_open((rt_device_t)i2c_bus, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
        return -1;
    for (l = driver_list.next; l != &driver_list; l = l->next)
    {
        if (rt_list_entry(l, struct touch_drivers, list)->probe(i2c_bus))
        {
            current_driver = rt_list_entry(l, struct touch_drivers, list);
            break;
        }
    }
    if (current_driver == RT_NULL)
    {
#ifdef DBG_ENABLE
        LOG_E("no touch screen or do not have driver\r\n");
#endif
        rt_device_close((rt_device_t)i2c_bus);
        return -1;
    }
    touch_isr_sem = rt_sem_create("ts", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(touch_isr_sem);

    current_driver->ops->init(i2c_bus);
#ifdef DBG_ENABLE
    LOG_I("touch screen found driver\r\n");
#endif
    tid = rt_thread_create("touch", touch_thread_entry, current_driver, 2048, 27, 20);
    if (tid == RT_NULL)
    {
        current_driver->ops->deinit();
        rt_device_close((rt_device_t)i2c_bus);
        if (touch_isr_sem)
        {
            rt_sem_delete(touch_isr_sem);
            touch_isr_sem = RT_NULL;
        }
        return -1;
    }
    rt_thread_startup(tid);
    return 0;
}

static void touch_init_thread_entry(void *parameter)
{
    rt_touch_thread_init();
}
static int touc_bg_init(void)
{
    rt_thread_t tid = RT_NULL;
    tid = rt_thread_create("touchi", touch_init_thread_entry, RT_NULL, 2048, 28, 20);
    if (tid == RT_NULL)
    {
        return -1;
    }
    rt_thread_startup(tid);
    return 0;
}
INIT_APP_EXPORT(touc_bg_init);


#endif
