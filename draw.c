#include <stdlib.h>
#include "32x.h"
#include "types.h"

#define DRAW_DST_BITS 8
#include "draw_inc.h"
#undef DRAW_DST_BITS

#define DRAW_DST_BITS 16
#include "draw_inc.h"
#undef DRAW_DST_BITS

rect_t scissor;

draw_spritefn_t draw_spritefn(int flags)
{
    if (flags & DRAWSPR_PRECISE)
    {
        draw8_spr8func_t* funcs = draw8spr8funcs;
        if (flags & DRAWSPR_SCALE) funcs += 4;
        return (draw_spritefn_t)funcs[(flags & 3) ^ 3];
    }
    else
    {
        draw16_spr8func_t* funcs = draw16spr8funcs;
        if (flags & DRAWSPR_SCALE) funcs += 4;
        return (draw_spritefn_t)funcs[(flags & 3) ^ 3];
    }
}

void draw_handle_drawspritecmd(drawsprcmd_t *cmd)
{
    void* fb = (void *)((uint16_t *)(cmd->flags & DRAWSPR_OVERWRITE ? &MARS_OVERWRITE_IMG : &MARS_FRAMEBUFFER) + 0x100);
    draw_spritefn_t fn = draw_spritefn(cmd->flags);
    fn(fb, cmd);
}

void draw_setScissor(int16_t x, int16_t y, int16_t w, int16_t h)
{
    scissor.x1 = x;
    scissor.y1 = y;
    scissor.x2 = scissor.x1 + w;
    scissor.y2 = scissor.y1 + h;
}

// returns 2 if fully clipped
// returns 1 if partially clipped
// otherwise returns 0 
int draw_clip(int x, int y, int w, int h, rect_t* cliprect)
{
    int cl, cr, ct, cb;

    if (x + w < scissor.x1 || y + h < scissor.y1)
        return 2;
    if (x >= scissor.x2 || y >= scissor.y2)
        return 2;

    cl = cr = 0;
    ct = cb = 0;

    if (w + x > scissor.x2)
        cr = w + x - scissor.x2;
    if (x < scissor.x1)
        cl = scissor.x1 - x;
    if (h + y > scissor.y2)
        cb = h + y - scissor.y2;
    if (y < scissor.y1)
        ct = scissor.y1 - y;

    if (w <= cr + cl || h <= cb + ct)
        return 2;

    cliprect->x1 = cl;
    cliprect->x2 = cr;
    cliprect->y1 = ct;
    cliprect->y2 = cb;

    return (ct | cl | cr | cb ? 1 : 0);
}
