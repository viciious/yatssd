#include "32x.h"
#include "types.h"
#include "draw.h"
#include "res.h"
#include "hw_32x.h"

drawtilelayerscmd_t slave_drawtilelayerscmd;

void init_tilemap(tilemap_t *tm, int tw, int th, int numh, int numv, const uint16_t **tmx, int nl)
{
    tm->tw = tw;
    tm->th = th;

    tm->tmx = (uint16_t **)tmx;
    tm->numlayers = nl;

    tm->tiles_hor = numh;
    tm->tiles_ver = numv;

    tm->scroll_tiles_hor = (canvas_pitch - canvas_width) / tw;
    tm->scroll_interval_hor = tm->scroll_tiles_hor * tw;

    tm->scroll_tiles_ver = (canvas_yaw - canvas_height) / th;
    tm->scroll_interval_ver = tm->scroll_tiles_ver * th;

    tm->canvas_tiles_hor = canvas_pitch / tw;
    tm->canvas_tiles_ver = canvas_yaw / th;

    tm->numtiles = numh * numv;
}

// in window coordinates
void draw_dirtyrect(tilemap_t* tm, int x, int y, int w, int h)
{
    int start_tile_hor, start_tile_ver;
    int end_tile_hor, end_tile_ver;
    int num_tiles_x, num_tiles_y;
    uint16_t* extrafb = (uint16_t*)&MARS_FRAMEBUFFER + 0x100 + ((canvas_pitch * canvas_yaw) >> 1);

    if (x >= canvas_pitch) return;
    if (y >= canvas_yaw) return;

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    start_tile_hor = x / tm->tw;
    start_tile_ver = y / tm->th;
    end_tile_hor = (x + w - 1) / tm->tw;
    end_tile_ver = (y + h - 1) / tm->th;
    num_tiles_x = end_tile_hor - start_tile_hor + 1;
    num_tiles_y = end_tile_ver - start_tile_ver + 1;

    int canvas_tiles_hor = tm->canvas_tiles_hor;
    int canvas_tiles_ver = tm->canvas_tiles_ver;

    if (start_tile_hor + num_tiles_x > canvas_tiles_hor) num_tiles_x = canvas_tiles_hor - start_tile_hor;
    if (start_tile_ver + num_tiles_y > canvas_tiles_ver) num_tiles_y = canvas_tiles_ver - start_tile_ver;

    uint16_t* dirty = extrafb + 3;
    dirty += start_tile_ver * canvas_tiles_hor + start_tile_hor;

    for (y = 0; y < num_tiles_y; y++) {
        uint16_t* p = dirty;
        for (x = 0; x < num_tiles_x; x++) {
            *p++ = UINT16_MAX;
        }
        dirty += canvas_tiles_hor;
    }
}

static int draw_drawtile(int x, int y, int w, int h,
    const uint8_t* data, int flags, void *fb, draw_spritefn_t fn)
{
    drawsprcmd_t cmd;
    int sx = 0, sy = 0;

    x += window_canvas_x;
    y += window_canvas_y;

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
    case DRAWSPR_HFLIP:
    case DRAWSPR_VFLIP:
        sx = w;
        break;
    }

    cmd.flags = flags;
    cmd.x = x, cmd.y = y;
    cmd.w = w, cmd.h = h;
    cmd.sx = sx, cmd.sy = sy;
    cmd.sw = w, cmd.sh = h;
    cmd.sdata = (void*)data;
    cmd.scale = 0;
    fn(fb, &cmd);

    return 1;
}

void draw_tile_layer(drawtilelayerscmd_t *cmd)
{
    int l;
    int x, y;
    tilemap_t* tm = cmd->tm;
    int w = tm->tw, h = tm->th;
    int y_tile;
    uint16_t* extrafb = (uint16_t*)&MARS_FRAMEBUFFER + 0x100 + ((canvas_pitch * canvas_yaw) >> 1);
    uint16_t* dirty = extrafb + 3;
    int scroll_y = camera_y;
    int canvas_tiles_hor = tm->canvas_tiles_hor;
    int xx = cmd->x, yy = cmd->y;
    int start_tile = cmd->start_tile, end_tile = cmd->end_tile;
    int scroll_tile_id = cmd->scroll_tile_id;
    int num_tiles_x = cmd->num_tiles_x;
    int drawmode = cmd->drawmode;
    void* fb = (void*)((uint16_t*)(drawmode & DRAWSPR_OVERWRITE ? &MARS_OVERWRITE_IMG : &MARS_FRAMEBUFFER) + 0x100);
    draw_spritefn_t fn = draw_spritefn(drawmode);
    int drawcnt = 0;

    if (end_tile >= tm->numtiles)
        end_tile = tm->numtiles - 1;

    for (l = 0; l < tm->numlayers; l++)
    {
        const uint16_t* layer = tm->tmx[l];

        y = yy;
        for (y_tile = start_tile; y_tile <= end_tile; y_tile += tm->tiles_hor)
        {
            int id;
            int tile;
            int t1 = y_tile;
            int t2 = y_tile + num_tiles_x;

            id = scroll_tile_id;
            x = xx;
            for (tile = t1; tile <= t2; tile++)
            {
                uint16_t idx = layer[tile];
                if (dirty[id] != idx)
                {
                    if (idx != 0)
                    {
                        uint8_t* res = reslist[idx - 1];
                        //if (debug) res = reslist[0];
                        draw_drawtile(x, y, w, h, res, drawmode, fb, fn);
                        drawcnt++;
                    }
                    dirty[id] = idx;
                }

                id++;
                x += w;
            }

            y += h;
            scroll_tile_id += canvas_tiles_hor;
            if (y >= yy + scroll_y + canvas_height)
                break;
        }

        drawmode |= DRAWSPR_OVERWRITE;
    }

    cmd->drawcnt = drawcnt;
}

int draw_tilemap(tilemap_t *tm)
{
    int x, y;
    int w = tm->tw, h = tm->th;
    int drawmode = 0;
    uint16_t* extrafb = (uint16_t*)&MARS_FRAMEBUFFER + 0x100 + ((canvas_pitch * canvas_yaw) >> 1);

    int scroll_tiles_hor = tm->scroll_tiles_hor;
    int scroll_interval_hor = tm->scroll_interval_hor;

    int scroll_tiles_ver = tm->scroll_tiles_ver;
    int scroll_interval_ver = tm->scroll_interval_ver;

    int start_tile_hor = camera_x / w;
    int start_tile_ver = camera_y / h;

    int end_tile_hor = (camera_x + canvas_width) / w;
    int end_tile_ver = (camera_y + canvas_height) / h;

    int scroll_count_hor = camera_x / scroll_interval_hor;
    int scroll_count_ver = camera_y / scroll_interval_ver;

    int tiles_hor = tm->tiles_hor;
    int tiles_ver = tm->tiles_ver;

    if (start_tile_hor < 0) start_tile_hor = 0;
    if (start_tile_hor > tiles_hor) start_tile_hor = tiles_hor;
    if (end_tile_hor < 0) end_tile_hor = 0;
    if (end_tile_hor > tiles_hor) end_tile_hor = tiles_hor;

    int half_tiles_hor = (end_tile_hor - start_tile_hor) >> 1;

    if (start_tile_ver < 0) start_tile_ver = 0;
    if (start_tile_ver > tiles_ver) start_tile_ver = tiles_ver;
    if (end_tile_ver < 0) end_tile_ver = 0;
    if (end_tile_ver > tiles_ver) end_tile_ver = tiles_ver;

    int start_tile = start_tile_ver * tiles_hor + start_tile_hor;
    int end_tile = end_tile_ver * tiles_hor + end_tile_hor;

    int top_scroll_tile_hor = scroll_tiles_hor * scroll_count_hor;
    int top_scroll_tile_ver = scroll_tiles_ver * scroll_count_ver;

    int scroll_x = camera_x - scroll_count_hor * scroll_interval_hor;
    int scroll_y = camera_y - scroll_count_ver * scroll_interval_ver;

    window_canvas_x = scroll_x;
    window_canvas_y = scroll_y;
    MARS_VDP_SHIFTREG = old_camera_x;

    int canvas_tiles_hor = tm->canvas_tiles_hor;
    int canvas_tiles_ver = tm->canvas_tiles_ver;

    int xx = ((start_tile_hor - top_scroll_tile_hor) * w) - scroll_x;
    int yy = -(scroll_y & (h - 1));

    uint16_t *dirty = extrafb + 3;

    int16_t scrollwords = (scroll_x + (scroll_y * canvas_pitch)) >> 1;
    if (scrollwords != MARS_FRAMEBUFFER - 0x100)
    {
        Hw32xUpdateLineTable(scroll_x >> 1, scroll_y, 0);
    }

    if (canvas_rebuild_id != *extrafb)
    {
        uint16_t* p = dirty;

        // mark all tiles as dirty
        for (y = 0; y < canvas_tiles_ver; y++) {
            for (x = 0; x < canvas_tiles_hor; x++) {
                *p++ = UINT16_MAX;
            }
        }

        *extrafb = canvas_rebuild_id;
    }
 
    drawtilelayerscmd_t cmd, *scmd = &slave_drawtilelayerscmd;
    cmd.tm = tm;
    cmd.x = xx;
    cmd.y = yy;
    cmd.start_tile = start_tile;
    cmd.end_tile = end_tile;
    cmd.scroll_tile_id = (start_tile_ver - top_scroll_tile_ver) * canvas_tiles_hor + (start_tile_hor - top_scroll_tile_hor);
    cmd.num_tiles_x = half_tiles_hor;
    cmd.drawmode = drawmode;

    scmd->tm = tm;
    scmd->x = xx + half_tiles_hor * w;
    scmd->y = yy;
    scmd->start_tile = start_tile + half_tiles_hor;
    scmd->end_tile = end_tile;
    scmd->scroll_tile_id = cmd.scroll_tile_id + half_tiles_hor;
    scmd->num_tiles_x = half_tiles_hor;
    scmd->drawmode = drawmode;

    while (MARS_SYS_COMM4 != 0) {}
    MARS_SYS_COMM4 = 5;

    draw_tile_layer(&cmd);

    while (MARS_SYS_COMM4 != 0) {}

    ClearCacheLines(&scmd->drawcnt, 1);

    return cmd.drawcnt + scmd->drawcnt;
}
