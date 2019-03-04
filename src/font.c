/*
 * File      : font.c
 * This file is part of RT-Thread GUI Engine
 * COPYRIGHT (C) 2006 - 2017, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-16     Bernard      first version
 * 2013-08-31     Bernard      remove the default font setting.
 *                             (which set by theme)
 */
#include <rtgui/font.h>
#include <rtgui/dc.h>
#include <rtgui/filerw.h>

#ifdef GUIENGINE_USING_TTF
#include <rtgui/font_freetype.h>
#endif

static rtgui_list_t _rtgui_font_list;
static struct rtgui_font *rtgui_default_font;

extern struct rtgui_font rtgui_font_asc16;
extern struct rtgui_font rtgui_font_arial16;
extern struct rtgui_font rtgui_font_asc12;
extern struct rtgui_font rtgui_font_arial12;
#ifdef GUIENGINE_USING_FONTHZ
extern struct rtgui_font rtgui_font_hz16;
extern struct rtgui_font rtgui_font_hz12;
#endif

void rtgui_font_system_init(void)
{
    rtgui_list_init(&(_rtgui_font_list));

    /* set default font to NULL */
    rtgui_default_font = RT_NULL;

#ifdef GUIENGINE_USING_FONT12
    rtgui_font_system_add_font(&rtgui_font_asc12);
    rtgui_default_font = &rtgui_font_asc12;
#ifdef GUIENGINE_USING_FONTHZ
    rtgui_font_system_add_font(&rtgui_font_hz12);
    {
        struct rtgui_hz_file_font *hz12 = (struct rtgui_hz_file_font *)rtgui_font_hz12.data;
        if (hz12->fd < 0)
        {
            rtgui_font_system_remove_font(&rtgui_font_hz12);
        }
        else
        {
            rtgui_default_font = &rtgui_font_hz12;
        }
    }
#endif
#endif

#ifdef GUIENGINE_USING_FONT16
    rtgui_font_system_add_font(&rtgui_font_asc16);
    rtgui_default_font = &rtgui_font_asc16;
#ifdef GUIENGINE_USING_FONTHZ
    rtgui_font_system_add_font(&rtgui_font_hz16);
    {
        struct rtgui_hz_file_font *hz16 = (struct rtgui_hz_file_font *)rtgui_font_hz16.data;
        if (hz16->fd < 0)
        {
            rtgui_font_system_remove_font(&rtgui_font_hz16);
        }
        else
        {
            rtgui_default_font = &rtgui_font_hz16;
        }
    }
#endif
#endif

#ifdef GUIENGINE_USING_TTF
    rtgui_ttf_system_init();
#endif
}

void rtgui_font_fd_uninstall(void)
{
#ifdef GUIENGINE_USING_HZ_FILE
    struct rtgui_list_node *node;
    struct rtgui_font *font;

    rtgui_list_foreach(node, &_rtgui_font_list)
    {
        font = rtgui_list_entry(node, struct rtgui_font, list);
        if (font->engine == &rtgui_hz_file_font_engine)
        {
            struct rtgui_hz_file_font *hz_file_font = (struct rtgui_hz_file_font *)font->data;
            if (hz_file_font->fd >= 0)
            {
                close(hz_file_font->fd);
                hz_file_font->fd = -1;
            }
        }
    }
#endif
}

void rtgui_font_system_add_font(struct rtgui_font *font)
{
    rtgui_list_init(&(font->list));
    rtgui_list_append(&_rtgui_font_list, &(font->list));

    /* init font */
    if (font->engine->font_init != RT_NULL)
        font->engine->font_init(font);

    /* first refer, load it */
    if (font->engine->font_load != RT_NULL)
        font->engine->font_load(font);
}
RTM_EXPORT(rtgui_font_system_add_font);

void rtgui_font_system_remove_font(struct rtgui_font *font)
{
    rtgui_list_remove(&_rtgui_font_list, &(font->list));
}
RTM_EXPORT(rtgui_font_system_remove_font);

struct rtgui_font *rtgui_font_default()
{
    return rtgui_default_font;
}

void rtgui_font_set_defaut(struct rtgui_font *font)
{
    // rt_kprintf("set font size to %d\n", font->height);
    rtgui_default_font = font;
}

struct rtgui_font *rtgui_font_refer(const char *family, rt_uint16_t height)
{
    /* search font */
    struct rtgui_list_node *node;
    struct rtgui_font *font;

    rtgui_list_foreach(node, &_rtgui_font_list)
    {
        font = rtgui_list_entry(node, struct rtgui_font, list);
        if ((rt_strcasecmp(font->family, family) == 0) && font->height == height)
        {
            font->refer_count ++;
            return font;
        }
    }

    return RT_NULL;
}
RTM_EXPORT(rtgui_font_refer);

void rtgui_font_derefer(struct rtgui_font *font)
{
    RT_ASSERT(font != RT_NULL);

    font->refer_count --;

    /* no refer, remove font */
    if (font->refer_count == 0)
    {
        rtgui_font_system_remove_font(font);
    }
}
RTM_EXPORT(rtgui_font_derefer);

/* draw a text */
void rtgui_font_draw(struct rtgui_font *font, struct rtgui_dc *dc, const char *text, rt_ubase_t len, struct rtgui_rect *rect)
{
    RT_ASSERT(font != RT_NULL);

    if (font->engine != RT_NULL &&
            font->engine->font_draw_text != RT_NULL)
    {
        font->engine->font_draw_text(font, dc, text, len, rect);
    }
}

int rtgui_font_get_string_width(struct rtgui_font *font, const char *text)
{
    rtgui_rect_t rect;

    /* get metrics */
    rtgui_font_get_metrics(font, text, &rect);

    return rect.x2 - rect.x1;
}
RTM_EXPORT(rtgui_font_get_string_width);

void rtgui_font_get_metrics(struct rtgui_font *font, const char *text, rtgui_rect_t *rect)
{
    RT_ASSERT(font != RT_NULL);

    if (font->engine != RT_NULL &&
            font->engine->font_get_metrics != RT_NULL)
    {
        font->engine->font_get_metrics(font, text, rect);
    }
    else
    {
        /* no font engine found, set rect to zero */
        rt_memset(rect, 0, sizeof(rtgui_rect_t));
    }
}
RTM_EXPORT(rtgui_font_get_metrics);


/* GB18030 encoding:
 *          1st byte    2nd byte    3rd byte    4th byte
 *  1byte: 0x00~0x7F
 * 2bytes: 0x81~0xFE   0x40~0xFE
 * 4bytes: 0x81~0xFE   0x30~0x39   0x81~0xFE   0x30~0x39
 */
struct rtgui_char_position _string_char_width(char *str, rt_size_t len, rt_size_t offset)
{
    struct rtgui_char_position pos = {0, 0};
    unsigned char *pc;

    RT_ASSERT(offset < len);

    pc = (unsigned char*)str;

    while (pc <= (unsigned char*)str + offset)
    {
        if (pc[0] < 0x80)
        {
            pos.char_width = 1;
        }
        else if (0x81 <= pc[0] && pc[0] <= 0xFE)
        {
            if (0x40 <= pc[1] && pc[1] <= 0xFE)
            {
                /* GBK */
                pos.char_width = 2;
            }
            else if (0x30 <= pc[1] && pc[1] <= 0x39)
            {
                /* GB18030 */
                pos.char_width = 4;
            }
            else
            {
                /* FIXME: unknown encoding */
                RT_ASSERT(0);
                pos.char_width = 1;
            }
        }
        else
        {
            /* FIXME: unknown encoding */
            RT_ASSERT(0);
            pos.char_width = 1;
        }
        pc += pos.char_width;
    }
    pos.remain = pc - (unsigned char*)&str[offset];
    return pos;
}
