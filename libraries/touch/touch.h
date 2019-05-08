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
#ifndef __TOUCH_H__
#define __TOUCH_H__

#include "rtthread.h"
#include "rtdevice.h"

#define TOUCH_DBG_LEVEL DBG_LOG

#define TOUCH_EVENT_UP      (0x01)
#define TOUCH_EVENT_DOWN    (0x02)
#define TOUCH_EVENT_MOVE    (0x03)
#define TOUCH_EVENT_NONE    (0x80)

struct touch_message
{
    rt_uint16_t x;
    rt_uint16_t y;
    rt_uint8_t event;
};
typedef struct touch_message *touch_msg_t;

struct touch_ops
{
    void (* isr_enable)(rt_bool_t);
    rt_err_t (* read_point)(touch_msg_t);
    void (* init)(struct rt_i2c_bus_device *);
    void (* deinit)(void);
};
typedef struct touch_ops *touch_ops_t;

struct touch_drivers
{
    rt_list_t       list;
    rt_bool_t (*probe)(struct rt_i2c_bus_device *i2c_bus);
    touch_ops_t     ops;
    void           *user_data;
};
typedef struct touch_drivers *touch_drv_t;

extern void rt_touch_drivers_register(touch_drv_t drv);
extern void rt_touch_release_sem(void);

#endif
