/*
 * File      : font_hz_bmp.c
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
 * 2010-09-15     Bernard      first version
 */
#include <rtgui/dc.h>
#include <rtgui/font.h>

#ifdef RTGUI_USING_HZ_BMP

static void rtgui_hz_bitmap_font_draw_text(struct rtgui_font *font, struct rtgui_dc *dc, const char *text, rt_ubase_t len, struct rtgui_rect *rect);
static void rtgui_hz_bitmap_font_get_metrics(struct rtgui_font *font, const char *text, rtgui_rect_t *rect);
const struct rtgui_font_engine hz_bmp_font_engine =
{
    RT_NULL,
    RT_NULL,
    rtgui_hz_bitmap_font_draw_text,
    rtgui_hz_bitmap_font_get_metrics
};

#ifdef RTGUI_USING_FONT_COMPACT
extern rt_uint32_t rtgui_font_mph12(const rt_uint16_t key);
extern rt_uint32_t rtgui_font_mph16(const rt_uint16_t key);
rt_inline const rt_uint8_t *_rtgui_hz_bitmap_get_font_ptr(struct rtgui_font_bitmap *bmp_font,
        rt_uint8_t *str,
        rt_base_t font_bytes)
{
    rt_uint16_t cha = *(rt_uint16_t *)str;
    int idx;

    if (bmp_font->height  == 16)
        idx = rtgui_font_mph16(cha);
    else // asume the height is 12
        idx = rtgui_font_mph12(cha);

    /* don't access beyond the data */
    if (idx < 0)
        idx = 0;

    /* get font pixel data */
    return bmp_font->bmp + idx * font_bytes;
}
#else
rt_inline const rt_uint8_t *_rtgui_hz_bitmap_get_font_ptr(struct rtgui_font_bitmap *bmp_font,
        rt_uint8_t *str,
        rt_base_t font_bytes)
{
    rt_ubase_t sect, index;

    /* calculate section and index */
    sect  = *str - 0xA0;
    index = *(str + 1) - 0xA0;

    /* get font pixel data */
    return bmp_font->bmp + (94 * (sect - 1) + (index - 1)) * font_bytes;
}
#endif

static void _rtgui_hz_bitmap_font_draw_text(struct rtgui_font_bitmap *bmp_font, struct rtgui_dc *dc, const char *text, rt_ubase_t len, struct rtgui_rect *rect)
{
    rtgui_color_t bc;
    rt_uint16_t style;
    rt_uint8_t *str;
    register rt_base_t h, word_bytes, font_bytes;

    RT_ASSERT(bmp_font != RT_NULL);

    /* get text style */
    style = rtgui_dc_get_gc(dc)->textstyle;
    bc = rtgui_dc_get_gc(dc)->background;

    /* drawing height */
    h = (bmp_font->height + rect->y1 > rect->y2) ? rect->y2 - rect->y1 : bmp_font->height;
    word_bytes = (bmp_font->width + 7) / 8;
    font_bytes = word_bytes * bmp_font->height;

    str = (rt_uint8_t *)text;

    while (len > 0 && rect->x1 < rect->x2)
    {
        const rt_uint8_t *font_ptr;
        register rt_base_t i, j, k;

        /* get font pixel data */
        font_ptr = _rtgui_hz_bitmap_get_font_ptr(bmp_font, str, font_bytes);
        /* draw word */
        for (i = 0; i < h; i ++)
        {
            for (j = 0; j < word_bytes; j++)
                for (k = 0; k < 8; k++)
                {
                    if (((font_ptr[i * word_bytes + j] >> (7 - k)) & 0x01) != 0 &&
                            (rect->x1 + 8 * j + k < rect->x2))
                    {
                        rtgui_dc_draw_point(dc, rect->x1 + 8 * j + k, rect->y1 + i);
                    }
                    else if (style & RTGUI_TEXTSTYLE_DRAW_BACKGROUND)
                    {
                        rtgui_dc_draw_color_point(dc, rect->x1 + 8 * j + k, rect->y1 + i, bc);
                    }
                }
        }

        /* move x to next character */
        rect->x1 += bmp_font->width;
        str += 2;
        len -= 2;
    }
}

static void rtgui_hz_bitmap_font_draw_text(struct rtgui_font *font, struct rtgui_dc *dc, const char *text, rt_ubase_t length, struct rtgui_rect *rect)
{
    rt_uint32_t len;
    struct rtgui_font *efont;
    struct rtgui_font_bitmap *bmp_font = (struct rtgui_font_bitmap *)(font->data);
    struct rtgui_rect text_rect;

    RT_ASSERT(dc != RT_NULL);

    rtgui_font_get_metrics(rtgui_dc_get_gc(dc)->font, text, &text_rect);
    rtgui_rect_move_to_align(rect, &text_rect, RTGUI_DC_TEXTALIGN(dc));

    /* get English font */
    efont = rtgui_font_refer("asc", bmp_font->height);
    if (efont == RT_NULL) efont = rtgui_font_default(); /* use system default font */

    while (length > 0)
    {
        len = 0;
        while (((rt_uint8_t) * (text + len)) < 0x80 && *(text + len) && len < length) len ++;
        /* draw text with English font */
        if (len > 0)
        {
            rtgui_font_draw(efont, dc, text, len, &text_rect);

            text += len;
            length -= len;
        }

        len = 0;
        while (((rt_uint8_t) * (text + len)) >= 0x80 && len < length) len ++;
        if (len > 0)
        {
            _rtgui_hz_bitmap_font_draw_text(bmp_font, dc, text, len, &text_rect);

            text += len;
            length -= len;
        }
    }

    rtgui_font_derefer(efont);
}

static void rtgui_hz_bitmap_font_get_metrics(struct rtgui_font *font, const char *text, rtgui_rect_t *rect)
{
    int hzlen=0, asclen=0, ascw=8;
    const char *sc = text;
    struct rtgui_font_bitmap *bmp_font = (struct rtgui_font_bitmap *)(font->data);
    struct rtgui_font *asc = rtgui_font_refer("asc", 16);
    if(asc != RT_NULL)
        ascw = ((struct rtgui_font_bitmap *)(asc->data))->width;

    RT_ASSERT(bmp_font != RT_NULL);

    for(; *sc != '\0'; ++sc) 
    {
        if((rt_uint8_t)*sc >= 0x80 && *(sc+1) != '\n') 
            hzlen++, sc++;
        else
            asclen++;
    }
    /* set metrics rect */
    rect->x1 = rect->y1 = 0;
    /* Chinese font is always fixed font */
    rect->x2 = (rt_int16_t)(bmp_font->width * hzlen + ascw * asclen);
    rect->y2 = bmp_font->height;
}

#endif
