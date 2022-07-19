#include "32x.h"
#include "types.h"
#include "draw.h"
#include "hw_32x.h"

drawtilelayerscmd_t slave_drawtilelayerscmd;

static uint16_t global_tilemap_id;

static fixed_t old_camera_x, old_camera_y;
static fixed_t main_camera_x, main_camera_y;

static fixed_t camera_x, camera_y;

static int draw_tile_layer(tilemap_t *tm, int layer, int fpcamera_x, 
int fpcamera_y, int numlayers, int *pclipped)
ATTR_DATA_ALIGNED;

static int draw_drawtile(int x, int y, int w, int h,
    const uint8_t* data, int flags, void* fb, draw_spritefn_t fn)
ATTR_DATA_ALIGNED;

void init_tilemap(tilemap_t *tm, const dtilemap_t *dtm, uint8_t **reslist)
{
    int tw = dtm->tilew;
    int th = dtm->tileh;

    tm->id = global_tilemap_id++;

    tm->tw = tw;
    tm->th = th;

    tm->layers = dtm->layers;
    tm->numlayers = dtm->numlayers;
    tm->layers = dtm->layers;
    tm->mdPlane[0] = (dtilelayer_t *)&dtm->mdPlaneA;
    tm->mdPlane[1] = (dtilelayer_t *)&dtm->mdPlaneB;
    tm->reslist = reslist;

    tm->tiles_hor = dtm->numtw;
    tm->tiles_ver = dtm->numth;

    tm->scroll_tiles_hor = (canvas_pitch - canvas_width) / tw;
    tm->scroll_interval_hor = tm->scroll_tiles_hor * tw;

    tm->scroll_tiles_ver = (canvas_yaw - canvas_height) / th;
    tm->scroll_interval_ver = tm->scroll_tiles_ver * th;

    tm->canvas_tiles_hor = canvas_pitch / tw;
    tm->canvas_tiles_ver = canvas_yaw / th;

    tm->numtiles = tm->tiles_hor * tm->tiles_ver;

    set_tilemap_wrap(tm, dtm->wrapX, dtm->wrapY);

    HwMdClearPlanes();

    if (dtm->mdPlaneA.bitmap) {
        HwMdSetPlaneBitmap('A', dtm->mdPlaneA.bitmap);
    }
    if (dtm->mdPlaneB.bitmap) {
        HwMdSetPlaneBitmap('B', dtm->mdPlaneB.bitmap);
    }

    Hw32xSetBGOverlayPriorityBit(0);

    Hw32xSetFGOverlayPriorityBit(dtm->mdPriority^1);

    Hw32xUpdateLineTable(0, 0, 0);
}

void set_tilemap_wrap(tilemap_t *tm, fixed_t wrapX, fixed_t wrapY)
{
    tm->wrapX = wrapX * (1<<16);
    tm->wrapY = wrapY * (1<<16);
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

    switch (tm->tw) {
    case 8:
        start_tile_hor = (unsigned)x >> 3;
        end_tile_hor = (unsigned)(x + w - 1) >> 3;

        start_tile_ver = (unsigned)y >> 3;
        end_tile_ver = (unsigned)(y + h - 1) >> 3;
        break;
    case 16:
        start_tile_hor = (unsigned)x >> 4;
        end_tile_hor = (unsigned)(x + w - 1) >> 4;

        start_tile_ver = (unsigned)y >> 4;
        end_tile_ver = (unsigned)(y + h - 1) >> 4;
        break;
    default:
        start_tile_hor = (unsigned)x / tm->tw;
        end_tile_hor = (unsigned)(x + w - 1) / tm->tw;

        start_tile_ver = (unsigned)y / tm->th;
        end_tile_ver = (unsigned)(y + h - 1) / tm->th;
        break;
    }

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

    x += window_canvas_x;
    y += window_canvas_y;

    cmd.flags = flags;
    cmd.x = x, cmd.y = y;
    cmd.w = w, cmd.h = h;
    cmd.sx = 0, cmd.sy = 0;
    cmd.sw = w, cmd.sh = h;
    cmd.sdata = (void*)data;
    cmd.scale = 0;
    fn(fb, &cmd);

    return 1;
}

void draw_handle_layercmd(drawtilelayerscmd_t *cmd)
{
    int l;
    int x, y;
    tilemap_t* tm = cmd->tm;
    const int w = tm->tw, h = tm->th;
    uint16_t* extrafb = (uint16_t*)&MARS_FRAMEBUFFER + 0x100 + ((canvas_pitch * canvas_yaw) >> 1);
    uint16_t* dirty = extrafb + 3;
    const int scroll_y = cmd->camera_y;
    const int canvas_tiles_hor = tm->canvas_tiles_hor;
    const int xx = cmd->x, yy = cmd->y;
    const int start_tile = cmd->start_tile, end_tile = cmd->end_tile;
    const int scroll_tile_id = cmd->scroll_tile_id;
    const int num_tiles_x = cmd->num_tiles_x;
    const uint8_t **reslist = (const uint8_t **)tm->reslist;
    int curdrawmode = cmd->drawmode;
    draw_spritefn_t fn = draw_spritefn(curdrawmode);
    int drawcnt = 0;

    if (cmd->startlayer != 0)
    {
        const uint16_t* layer = tm->layers[cmd->startlayer].tiles;
        int y_tile;
        int stid = scroll_tile_id;
        int drawmode = cmd->drawmode;

        y = yy;
        for (y_tile = start_tile; y_tile <= end_tile; y_tile += tm->tiles_hor)
        {
            int id;
            int tile;
            int t1 = y_tile;
            int t2 = y_tile + num_tiles_x;

            id = stid;
            x = xx;

            for (tile = t1; tile <= t2; tile++)
            {
                uint16_t idx = layer[tile];
                if (idx != 0)
                {
                    const uint8_t* res = reslist[(idx >> 2)];
                    //if (debug) res = reslist[0];
                    draw_sprite(x, y, w, h, res, drawmode | (idx & 3), 1);
                    drawcnt++;
                }
                id++;
                x += w;
            }

            y += h;
            stid += canvas_tiles_hor;
            if (y >= yy + scroll_y + canvas_height)
                break;
        }
        cmd->drawcnt = drawcnt;
        return;
    }

    for (l = 0; l < cmd->numlayers; l++)
    {
        int drawmode = cmd->drawmode;
        const uint16_t* layer = tm->layers[cmd->startlayer+l].tiles;
        int y_tile;
        int stid = scroll_tile_id;
        void* fb;

        if (l > 0)
            drawmode |= DRAWSPR_PRECISE | DRAWSPR_OVERWRITE;
        if (!(xx & 1))
            drawmode &= ~DRAWSPR_PRECISE;
        fb = (void*)(drawmode & DRAWSPR_OVERWRITE ? &MARS_OVERWRITE_IMG : &MARS_FRAMEBUFFER + 0x100);

        y = yy;
        for (y_tile = start_tile; y_tile <= end_tile; y_tile += tm->tiles_hor)
        {
            int id;
            int tile;
            int t1 = y_tile;
            int t2 = y_tile + num_tiles_x;

            id = stid;
            x = xx;
            for (tile = t1; tile <= t2; tile++)
            {
                uint16_t idx = layer[tile];
                if (dirty[id] != idx)
                {
                    if (idx != 0 || !l)
                    {
                        int tiledrawmode;
                        const uint8_t* res = reslist[(idx >> 2)];

                        tiledrawmode = drawmode | (idx & 3);
                        fn = draw_spritefn(tiledrawmode);

                        draw_drawtile(x, y, w, h, res, tiledrawmode, fb, fn);
                        drawcnt++;
                    }
                    dirty[id] = idx;
                }

                id++;
                x += w;
            }

            y += h;
            stid += canvas_tiles_hor;
            if (y >= yy + scroll_y + canvas_height)
                break;
        }

        layer++;
    }

    cmd->drawcnt = drawcnt;
}

static int draw_tile_layer(tilemap_t *tm, int layer, int fpcamera_x, int fpcamera_y, int numlayers, int *pclipped)
{
    int x, y;
    int w = tm->tw, h = tm->th;
    const fixed_t *plx = tm->layers[layer].parallax;
    int clipped = 0;

    camera_x = FixedMul(fpcamera_x, plx[0])>>16;
    camera_y = FixedMul(fpcamera_y, plx[1])>>16;

    if (camera_x < 0)
    {
        camera_x = 0;
        clipped |= 1;
    }

    if (camera_x > tm->tiles_hor * tm->tw - canvas_width)
    {
        camera_x = tm->tiles_hor * tm->tw - canvas_width;
        clipped |= 2;
    }

    if (camera_y < 0)
    {
        camera_y = 0;
        clipped |= 4;
    }

    if (camera_y > tm->tiles_ver * tm->th - canvas_height)
    {
        camera_y = tm->tiles_ver * tm->th - canvas_height;
        clipped |= 8;
    }

    int scroll_tiles_hor = 0, scroll_interval_hor = 0;
    int scroll_tiles_ver = 0, scroll_interval_ver = 0;
    int scroll_count_hor = 0, scroll_count_ver = 0;
    int top_scroll_tile_hor = 0, top_scroll_tile_ver = 0;

    if (layer == 0)
    {
        scroll_tiles_hor = tm->scroll_tiles_hor;
        scroll_interval_hor = tm->scroll_interval_hor;

        scroll_tiles_ver = tm->scroll_tiles_ver;
        scroll_interval_ver = tm->scroll_interval_ver;
    
        scroll_count_hor = camera_x / scroll_interval_hor;
        scroll_count_ver = camera_y / scroll_interval_ver;

        top_scroll_tile_hor = scroll_tiles_hor * scroll_count_hor;
        top_scroll_tile_ver = scroll_tiles_ver * scroll_count_ver;
    }

    int tiles_hor = tm->tiles_hor;
    int tiles_ver = tm->tiles_ver;

    int start_tile_hor = camera_x / w;
    if (start_tile_hor < 0) start_tile_hor = 0;
    if (start_tile_hor >= tiles_hor) return 0;

    int start_tile_ver = camera_y / h;
    if (start_tile_ver < 0) start_tile_ver = 0;
    if (start_tile_ver >= tiles_ver) return 0;

    int end_tile_hor = (camera_x + canvas_width) / w;
    if (end_tile_hor < 0) end_tile_hor = 0;
    if (end_tile_hor >= tiles_hor) end_tile_hor = tiles_hor - 1;

    int end_tile_ver = (camera_y + canvas_height) / h;
    if (end_tile_ver < 0) end_tile_ver = 0;
    if (end_tile_ver >= tiles_ver) end_tile_ver = tiles_ver - 1;

    int start_tile = start_tile_ver * tiles_hor + start_tile_hor;
    if (start_tile >= tm->numtiles)
        return 0;

    int end_tile = end_tile_ver * tiles_hor + end_tile_hor;
    if (end_tile >= tm->numtiles)
        end_tile = tm->numtiles-1;

    int half_tiles_hor = (end_tile_hor - start_tile_hor) >> 1;
    if (half_tiles_hor < 0)
        half_tiles_hor = 0;

    int canvas_tiles_hor = tm->canvas_tiles_hor;
    int canvas_tiles_ver = tm->canvas_tiles_ver;

    int scroll_x, scroll_y;

    int xx, yy;

    if (layer == 0)
    {
        main_camera_x = camera_x;
        main_camera_y = camera_y;
    }

    scroll_x = camera_x - scroll_count_hor * scroll_interval_hor;
    scroll_y = camera_y - scroll_count_ver * scroll_interval_ver;

    xx = ((start_tile_hor - top_scroll_tile_hor) * w) - scroll_x;
    yy = -(scroll_y & (h - 1));

    if (layer == 0)
    {
        window_canvas_x = scroll_x;
        window_canvas_y = scroll_y;
    }

    if (layer == 0)
    {
        uint16_t* extrafb = (uint16_t*)&MARS_FRAMEBUFFER + 0x100 + ((canvas_pitch * canvas_yaw) >> 1);
        uint16_t *dirty = extrafb + 3;

        MARS_VDP_SHIFTREG = old_camera_x;

        int16_t scrollwords = (scroll_x + (scroll_y * canvas_pitch)) >> 1;        
        if (scrollwords != MARS_FRAMEBUFFER - 0x100)
        {
            Hw32xUpdateLineTable(scroll_x >> 1, scroll_y, 0);
        }

        if (tm->id != *extrafb)
        {
            uint16_t* p = dirty;

            // mark all tiles as dirty
            for (y = 0; y < canvas_tiles_ver; y++) {
                for (x = 0; x < canvas_tiles_hor; x++) {
                    *p++ = UINT16_MAX;
                }
            }

            *extrafb = tm->id;
        }
    }

    drawtilelayerscmd_t cmd, *scmd = &slave_drawtilelayerscmd;
    cmd.tm = tm;
    cmd.x = xx;
    cmd.y = yy;
    cmd.start_tile = start_tile;
    cmd.end_tile = end_tile;
    cmd.scroll_tile_id = (start_tile_ver - top_scroll_tile_ver) * canvas_tiles_hor + (start_tile_hor - top_scroll_tile_hor);
    cmd.num_tiles_x = half_tiles_hor;
    cmd.startlayer = layer;
    cmd.numlayers = numlayers;
    cmd.camera_x = camera_x, cmd.camera_y = camera_y;
    if (layer == 0) {
        cmd.drawmode = 0;
    } else{
        cmd.drawmode = DRAWSPR_PRECISE|DRAWSPR_OVERWRITE;
    }

    scmd->tm = tm;
    scmd->x = xx + half_tiles_hor * w;
    scmd->y = yy;
    scmd->start_tile = start_tile + half_tiles_hor;
    scmd->end_tile = end_tile;
    scmd->scroll_tile_id = cmd.scroll_tile_id + half_tiles_hor;
    scmd->num_tiles_x = end_tile_hor - start_tile_hor - half_tiles_hor;
    scmd->startlayer = layer;
    scmd->numlayers = numlayers;
    scmd->camera_x = camera_x, scmd->camera_y = camera_y;
    if (layer == 0) {
        scmd->drawmode = 0;
    } else{
        scmd->drawmode = DRAWSPR_PRECISE|DRAWSPR_OVERWRITE;
    }

    while (MARS_SYS_COMM4 != 0) {}
    MARS_SYS_COMM4 = 5;

    draw_handle_layercmd(&cmd);

    while (MARS_SYS_COMM4 != 0) {}

    ClearCacheLines(&scmd->drawcnt, 1);

    *pclipped = clipped;
    return cmd.drawcnt + scmd->drawcnt;
}

int draw_tilemap(tilemap_t *tm, int fpcamera_x, int fpcamera_y, int *cameraclip)
{
    int i;
    int clip, drawcnt;
    char parallax;
    const fixed_t *bplx = tm->layers[0].parallax;

    *cameraclip = 0;
    old_camera_x = main_camera_x;
    old_camera_y = main_camera_y;

    if (tm->wrapX) {
        while (fpcamera_x >= tm->wrapX)
            fpcamera_x -= tm->wrapX;
    }
    if (tm->wrapY) {
        while (fpcamera_y >= tm->wrapY)
            fpcamera_y -= tm->wrapY;
    }

    // test for parallax in the upper layers
    parallax = 0;
    for (i = 1; i < tm->numlayers; i++)
    {
        const fixed_t *tplx = tm->layers[i].parallax;
        if (tplx[0] != bplx[0] || tplx[1] != bplx[1])
        {
            parallax = 1;
            break;
        }
    }

    for (i = 0; i < 2; i++) {
        const dtilelayer_t *mdpl = tm->mdPlane[i];
        if (mdpl->bitmap) {
            fixed_t camera_x = FixedMul(fpcamera_x, mdpl->parallax[0])>>16;
            HwMdHScrollPlane(i, mdpl->offset[0]+camera_x);
        }
    }

    if (!parallax)
        return draw_tile_layer(tm, 0, fpcamera_x, fpcamera_y, tm->numlayers, cameraclip);

    drawcnt = draw_tile_layer(tm, 0, fpcamera_x, fpcamera_y, 1, cameraclip);
    for (i = 1; i < tm->numlayers; i++)
        drawcnt += draw_tile_layer(tm, i, fpcamera_x, fpcamera_y, 1, &clip);

    return drawcnt;
}
