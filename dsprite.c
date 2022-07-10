#include "32x.h"
#include "types.h"
#include "draw.h"

drawsprcmd_t slave_drawsprcmd ATTR_CACHE_ALIGNED;
drawspr4cmd_t slave_drawspr4cmd ATTR_CACHE_ALIGNED;

static int draw_clipsprite(int x, int y, int w, int h, int sw, int sh,
    rect_t* cliprect, const uint8_t* data, int flags, fixed_t scale)
    ATTR_DATA_ALIGNED;

static int draw_clipsprite(int x, int y, int w, int h, int sw, int sh,
    rect_t* cliprect, const uint8_t* data, int flags, fixed_t scale)
{
    drawsprcmd_t cmd, * scmd;
    unsigned hh;
    int cl = cliprect->x1;
    int cr = cliprect->x2;
    int ct = cliprect->y1;
    int cb = cliprect->y2;
    int sx = 0, sy = 0, sy2;

    x += window_canvas_x;
    y += window_canvas_y;

    x += cl;
    y += ct;
    w -= cr + cl;
    h -= cb + ct;

    if (flags & DRAWSPR_MULTICORE)
        hh = h >> 1;
    else
        hh = h;

    if (flags & DRAWSPR_PRECISE) {
        if (!(x & 1) && !(w & 1) && !(flags & DRAWSPR_SCALE)) {
            flags &= ~DRAWSPR_PRECISE;
        }
    }
    else {
        // snap to even coordinate
        x &= ~1;
        w &= ~1;
    }

    switch (flags & (DRAWSPR_HFLIP | DRAWSPR_VFLIP)) {
    case 0:
    default:
        sx = cl;
        sy = ct;
        sy2 = ct + hh;
        break;
    case DRAWSPR_HFLIP:
        sx = w + cl;
        sy = ct;
        sy2 = ct + hh;
        break;
    case DRAWSPR_VFLIP:
        sx = w + cl;
        sy = cb + h - hh;
        sy2 = cb;
        break;
    case DRAWSPR_HFLIP | DRAWSPR_VFLIP:
        sx = cl;
        sy = cb + h - hh;
        sy2 = cb;
        break;
    }

    if (flags & DRAWSPR_SCALE) {
        int nudge = scale > 0x10000 ? 1 : 0;

        sx = IDiv((sx + nudge) << 16, scale);
        if (sx > sw) sx = sw;
        if (sx < 0) sx = 0;

        sy = IDiv((sy + nudge) << 16, scale);
        if (sy > sh) sy = sh;
        if (sy < 0) sy = 0;

        sy2 = IDiv((sy2 + nudge) << 16, scale);
        if (sy2 > sh) sy2 = sh;
        if (sy2 < 0) sy2 = 0;
    }

    switch (flags & (DRAWSPR_HFLIP | DRAWSPR_VFLIP)) {
    case DRAWSPR_HFLIP:
    case DRAWSPR_VFLIP:
        sx = sw - sx;
        break;
    }

    cmd.flags = flags;
    cmd.x = x, cmd.y = y;
    cmd.w = w, cmd.h = hh;
    cmd.sx = sx, cmd.sy = sy;
    cmd.sw = sw, cmd.sh = sh;
    cmd.sdata = (void*)data;
    cmd.scale = scale;

    if (h > hh)
    {
        while (MARS_SYS_COMM4 != 0) {}

        scmd = &slave_drawsprcmd;
        scmd->flags = flags;
        scmd->x = x, scmd->y = y + hh;
        scmd->w = w, scmd->h = h - hh;
        scmd->sx = sx, scmd->sy = sy2;
        scmd->sw = sw, scmd->sh = sh;
        scmd->sdata = (void*)data;
        scmd->scale = scale;

        MARS_SYS_COMM4 = 3;
    }

    draw_handle_drawspritecmd(&cmd);

    draw_dirtyrect(&tm, x, y, w, h);

    return 1;
}

int draw_sprite(int x, int y, int w, int h, const uint8_t* data, int flags, fixed_t scale)
{
    rect_t cliprect;

    int clip = draw_clip(x, y, w, h, &cliprect);
    if (clip == 2)
        return 0;

    draw_clipsprite(x, y, w, h, w, h, &cliprect, data, flags, scale);
    return 1;
}

void draw_stretch_sprite(int x, int y, int sw, int sh, const uint8_t* data, int flags, fixed_t scale)
{
    int w, h;
    rect_t cliprect;

    w = FixedMul(sw, scale);
    h = FixedMul(sh, scale);

    //w &= ~1;
    //h &= ~1;

    int clip = draw_clip(x, y, w, h, &cliprect);
    if (clip == 2)
        return;

    draw_clipsprite(x, y, w, h, sw, sh, &cliprect, data, flags, scale);
}

void draw_pivot_stretch_sprite(int x, int y, int sw, int sh, const uint8_t* data, int flags, fixed_t scale)
{
    int w, h;
    rect_t cliprect;

    w = FixedMul(sw, scale);
    h = FixedMul(sh, scale);

    x -= (w >> 1);
    y -= (h >> 1);

    int clip = draw_clip(x, y, w, h, &cliprect);
    if (clip == 2)
        return;

    draw_clipsprite(x, y, w, h, sw, sh, &cliprect, data, flags, scale);
}
