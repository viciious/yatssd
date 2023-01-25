#include "32x.h"
#include "types.h"
#include "draw.h"
#include "hw_32x.h"

typedef struct
{
    tilemap_t *tm;
    fixed_t fpcamera_x, fpcamera_y;
    void (*drawspr)(int l, void *p);
    void *sprp;
    int sprites_drawn;
    int parallax;
} drawtilecontext_t;

drawtilelayerscmd_t slave_drawtilelayerscmd;

static uint16_t global_tilemap_id = 1;

static fixed_t old_camera_x, old_camera_y;
static fixed_t main_camera_x, main_camera_y;

static fixed_t camera_x, camera_y;

static char tile_lock;

static int draw_tile_layer(drawtilecontext_t *dc, int layer, int *pclipped)
ATTR_DATA_ALIGNED;

static int draw_drawtile(int x, int y, int w, int h,
    const uint8_t* data, int flags, void* fb, draw_spritefn_t fn)
ATTR_DATA_ALIGNED;

static void lock_tiles(void)
ATTR_DATA_ALIGNED;

static void unlock_tiles(void)
ATTR_DATA_ALIGNED;

static int get_next_tile(void)
ATTR_DATA_ALIGNED;

void init_tilemap(tilemap_t *tm, const dtilemap_t *dtm, uint8_t **reslist)
{
    int i;
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

    for (i = 0; i < 2; i++) {
        const dtilelayer_t *mdpl = tm->mdPlane[i];
        if (mdpl) {
            HwMdSetPlaneBitmap(i, mdpl->bitmap);

            HwMdHScrollPlane(i, mdpl->offset[0]);
            HwMdVScrollPlane(i, -mdpl->offset[1]);
        }
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
    unsigned start_tile_hor, start_tile_ver;
    unsigned end_tile_hor, end_tile_ver;
    int num_tiles_x, num_tiles_y;
    int16_t* extrafb = (int16_t*)&MARS_FRAMEBUFFER + 0x100 + ((canvas_pitch * canvas_yaw) >> 1);

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    if (x >= (int)canvas_pitch) return;
    if (y >= (int)canvas_yaw) return;

    start_tile_hor = (unsigned)x;
    end_tile_hor = (unsigned)(x + w - 1);

    start_tile_ver = (unsigned)y >> 3;
    end_tile_ver = (unsigned)(y + h - 1);

    switch (tm->tw) {
    case 8:
        start_tile_hor >>= 3;
        end_tile_hor >>= 3;
        break;
    case 16:
        start_tile_hor >>= 4;
        end_tile_hor >>= 4;
        break;
    default:
        start_tile_hor /= tm->tw;
        end_tile_hor /= tm->tw;
        break;
    }

    switch (tm->th) {
    case 8:
        start_tile_ver >>= 3;
        end_tile_ver >>= 3;
        break;
    case 16:
        start_tile_ver >>= 4;
        end_tile_ver >>= 4;
        break;
    default:
        start_tile_ver /= tm->th;
        end_tile_ver /= tm->th;
        break;
    }

    num_tiles_x = end_tile_hor - start_tile_hor + 1;
    num_tiles_y = end_tile_ver - start_tile_ver + 1;

    int canvas_tiles_hor = tm->canvas_tiles_hor;
    int canvas_tiles_ver = tm->canvas_tiles_ver;

    if (start_tile_hor + num_tiles_x > canvas_tiles_hor) num_tiles_x = canvas_tiles_hor - start_tile_hor;
    if (start_tile_ver + num_tiles_y > canvas_tiles_ver) num_tiles_y = canvas_tiles_ver - start_tile_ver;

    int16_t* dirty = extrafb + 3;
    dirty += start_tile_ver * canvas_tiles_hor + start_tile_hor;

    for (y = 0; y < num_tiles_y; y++) {
        int16_t* p = dirty;
        for (x = 0; x < num_tiles_x; x++) {
            *p++ = -1;
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

static void lock_tiles(void)
{
    int res;
    do {
        __asm volatile (\
            "tas.b %1\n\t" \
            "movt %0\n\t" \
            : "=&r" (res) \
            : "m" (tile_lock) \
            );
    } while (res == 0);
}

static void unlock_tiles(void)
{
    tile_lock = 0;
}

static int get_next_tile(void)
{
    int n;

    lock_tiles();
    n = MARS_SYS_COMM6;
    MARS_SYS_COMM6 = n + 1;
    unlock_tiles();

    return n;
}

int draw_handle_layercmd(drawtilelayerscmd_t *cmd)
{
    int i, n;
    int x, y;
    tilemap_t* tm = cmd->tm;
    const int w = tm->tw, h = tm->th;
    int16_t* extrafb = (int16_t*)&MARS_FRAMEBUFFER + 0x100 + ((canvas_pitch * canvas_yaw) >> 1);
    int16_t* dirty = extrafb + 3;
    const int canvas_tiles_hor = tm->canvas_tiles_hor;
    const int xx = cmd->x, yy = cmd->y;
    const int start_tile = cmd->start_tile, end_tile = cmd->end_tile;
    const int scroll_tile_id = cmd->scroll_tile_id;
    const int num_tiles_x = cmd->num_tiles_x;
    const uint8_t **reslist = (const uint8_t **)tm->reslist;
    int curdrawmode = cmd->drawmode;
    draw_spritefn_t fn = draw_spritefn(curdrawmode);
    int drawcnt = 0;

    i = 0;
    n = get_next_tile();

    if (cmd->startlayer != 0 && cmd->parallax)
    {
        const dtilelayer_t *tl = &tm->layers[cmd->startlayer];
        const int16_t* layer = (int16_t *)tl->tiles;
        int y_tile;
        int stid = scroll_tile_id;
        int drawmode = cmd->drawmode;

        if (tl->objectLayer)
            return 0;

        y = yy;
        for (y_tile = start_tile; y_tile < end_tile; y_tile += tm->tiles_hor)
        {
            int id;
            int tile;
            int t1 = y_tile;
            int t2 = y_tile + num_tiles_x;

            id = stid;
            x = xx;

            for (tile = t1; tile < t2; tile++)
            {
                if (i == n)
                {
                    int16_t idx = layer[tile];
                    if (idx != 0)
                    {
                        const uint8_t* res = reslist[(idx >> 2)];
                        //if (debug) res = reslist[1];

                        draw_sprite(x, y, w, h, res, drawmode | (idx & 3), 1);
                        drawcnt++;
                    }
                    n = get_next_tile();
                }

                i++;
                id++;
                x += w;
            }

            y += h;
            stid += canvas_tiles_hor;
            if (y >= (int)canvas_pitch)
                break;
        }

        return drawcnt;
    }
    else
    {
        int drawmode = cmd->drawmode;
        unsigned l = cmd->startlayer;
        const dtilelayer_t *tl = &tm->layers[l];
        const int16_t* layer = (int16_t *)tl->tiles;
        const dtilelayer_t *ltl = &tm->layers[tm->numlayers-1];
        int y_tile;
        int stid = scroll_tile_id;
        void* fb;

        if (tl->objectLayer)
            return 0;

        // find the last non-object layer
        while (ltl != tm->layers && ltl->objectLayer)
            ltl--;
        const int16_t* last_layer = (int16_t *)ltl->tiles;

        if (l > 0)
            drawmode |= DRAWSPR_PRECISE | DRAWSPR_OVERWRITE;
        if (!((xx+window_canvas_x) & 1))
            drawmode &= ~DRAWSPR_PRECISE;
        fb = (void*)((drawmode & DRAWSPR_OVERWRITE ? &MARS_OVERWRITE_IMG : &MARS_FRAMEBUFFER) + 0x100);

        y = yy;
        for (y_tile = start_tile; y_tile < end_tile; y_tile += tm->tiles_hor)
        {
            int id;
            int tile;
            int t1 = y_tile;
            int t2 = y_tile + num_tiles_x;

            id = stid;
            x = xx;
            for (tile = t1; tile < t2; tile++)
            {
                if (i == n)
                {
                    int16_t idx = layer[tile];

                    // only redraw the tile if:
                    // 1. the tile in the dirty matrix doesn't match and
                    // 2a. it's not zero-tile or
                    // 2b. it's the base layer and the tile in the upmost layer of the tilemap doesn't match
                    if (dirty[id] != idx)
                    {
                        if (idx != 0 || (l == 0 && last_layer[tile] != dirty[id]))
                        {
                            // for base layer, always update dirty matrix with the current tile id
                            // for other layers, do so only if we're rendering atop of zero-tile
                            // otherwise update to -1 as we were rendering a sprite
                            if (l == 0)
                                dirty[id] = idx;
                            else
                                dirty[id] = dirty[id] == 0 ? idx : -1;

                            const uint8_t* res = reslist[(idx >> 2)];
                            int tiledrawmode = drawmode | (idx & 3);
                            /*if (debug) res = reslist[1];*/

                            fn = draw_spritefn(tiledrawmode);
                            draw_drawtile(x, y, w, h, res, tiledrawmode, fb, fn);
                            drawcnt++;
                        }
                    }
                    n = get_next_tile();
                }

                i++;
                id++;
                x += w;
            }

            y += h;
            stid += canvas_tiles_hor;
            if (y >= (int)canvas_pitch)
                break;
        }
    }

    return drawcnt;
}

static int draw_tile_layer(drawtilecontext_t *dc, int layer, int *pclipped)
{
    int x, y;
    tilemap_t *tm = dc->tm;
    const dtilelayer_t *tl = &tm->layers[layer];
    unsigned w = tm->tw, h = tm->th;
    const fixed_t *plx = tm->layers[layer].parallax;
    int clipped = 0;
    int drawcnt;

    if (tl->objectLayer)
    {
        *pclipped = 0;
        if (!dc->drawspr)
            return 0;
        dc->drawspr(tl->objectLayer, dc->sprp);
        dc->sprites_drawn = 1;
        return 0;
    }

    camera_x = FixedMul(dc->fpcamera_x, plx[0])>>16;
    camera_y = FixedMul(dc->fpcamera_y, plx[1])>>16;

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

    scroll_tiles_hor = tm->scroll_tiles_hor;
    scroll_interval_hor = tm->scroll_interval_hor;

    scroll_tiles_ver = tm->scroll_tiles_ver;
    scroll_interval_ver = tm->scroll_interval_ver;

    scroll_count_hor = camera_x / scroll_interval_hor;
    scroll_count_ver = camera_y / scroll_interval_ver;

    top_scroll_tile_hor = scroll_tiles_hor * scroll_count_hor;
    top_scroll_tile_ver = scroll_tiles_ver * scroll_count_ver;

    int tiles_hor = tm->tiles_hor;
    int tiles_ver = tm->tiles_ver;

    unsigned start_tile_hor, start_tile_ver;
    unsigned end_tile_hor, end_tile_ver;

    start_tile_hor = (unsigned)camera_x;
    end_tile_hor = (unsigned)camera_x + w - 1 + canvas_width;
    start_tile_ver = (unsigned)camera_y;
    end_tile_ver = (unsigned)camera_y + h - 1 + canvas_height;

    switch (w) {
    case 8:
        start_tile_hor >>= 3;
        end_tile_hor >>= 3;
        break;
    case 16:
        start_tile_hor >>= 4;
        end_tile_hor >>= 4;
        break;
    default:
        start_tile_hor /= w;
        end_tile_hor /= w;
        break;
    }

    switch (h) {
    case 8:
        start_tile_ver >>= 3;
        end_tile_ver >>= 3;
        break;
    case 16:
        start_tile_ver >>= 4;
        end_tile_ver >>= 4;
        break;
    default:
        start_tile_ver /= h;
        end_tile_ver /= h;
        break;
    }

    if (start_tile_hor >= tiles_hor) return 0;
    if (start_tile_ver >= tiles_ver) return 0;

    if (start_tile_hor < 0) start_tile_ver = 0;
    if (start_tile_ver < 0) start_tile_ver = 0;

    if (end_tile_hor > tiles_hor) end_tile_hor = tiles_hor;

    if (end_tile_ver < 1) end_tile_ver = 1;
    if (end_tile_ver > tiles_ver) end_tile_ver = tiles_ver;

    int start_tile = start_tile_ver * tiles_hor + start_tile_hor;
    if (start_tile >= tm->numtiles)
        return 0;

    int end_tile = (end_tile_ver - 1) * tiles_hor + end_tile_hor;
    if (end_tile > tm->numtiles)
        end_tile = tm->numtiles;

    int canvas_tiles_hor = tm->canvas_tiles_hor;
    int canvas_tiles_ver = tm->canvas_tiles_ver;

    unsigned scroll_x, scroll_y;

    int xx, yy;

    scroll_x = camera_x - scroll_count_hor * scroll_interval_hor;
    scroll_y = camera_y - scroll_count_ver * scroll_interval_ver;

    xx = ((start_tile_hor - top_scroll_tile_hor) * w) - scroll_x;
    yy = -(scroll_y & (h - 1));

    if (layer == 0)
    {
        main_camera_x = camera_x;
        main_camera_y = camera_y;
        window_canvas_x = scroll_x;
        window_canvas_y = scroll_y;
    }

    if (layer == 0)
    {
        int16_t* extrafb = (int16_t*)&MARS_FRAMEBUFFER + 0x100 + ((canvas_pitch * canvas_yaw) >> 1);
        int16_t *dirty = extrafb + 3;

        MARS_VDP_SHIFTREG = old_camera_x;

        int16_t scrollwords = (scroll_x + (scroll_y * canvas_pitch)) >> 1;        
        if (scrollwords != MARS_FRAMEBUFFER - 0x100)
        {
            Hw32xUpdateLineTable(scroll_x >> 1, scroll_y, 0);
        }

        if (tm->id != *extrafb)
        {
            int16_t* p = dirty;

            // mark all tiles as dirty
            for (y = 0; y < canvas_tiles_ver; y++) {
                for (x = 0; x < canvas_tiles_hor; x++) {
                    *p++ = -1;
                }
            }

            *extrafb = tm->id;
        }
    }

    drawtilelayerscmd_t *scmd = &slave_drawtilelayerscmd;
    scmd->tm = tm;
    scmd->x = xx;
    scmd->y = yy;
    scmd->start_tile = start_tile;
    scmd->end_tile = end_tile;
    scmd->scroll_tile_id = (start_tile_ver - top_scroll_tile_ver) * canvas_tiles_hor + (start_tile_hor - top_scroll_tile_hor);
    scmd->num_tiles_x = end_tile_hor - start_tile_hor;
    scmd->startlayer = layer;
    scmd->camera_x = camera_x, scmd->camera_y = camera_y;
    if (layer == 0) {
        scmd->drawmode = 0;
    } else{
        scmd->drawmode = DRAWSPR_PRECISE|DRAWSPR_OVERWRITE;
    }
    scmd->parallax = dc->parallax;

    while (MARS_SYS_COMM4 != 0);
    MARS_SYS_COMM6 = 0;
    MARS_SYS_COMM4 = 3;

    drawcnt = draw_handle_layercmd(scmd);
    while (MARS_SYS_COMM4 == 3);
    drawcnt += ((MARS_SYS_COMM4 >> 2) - 1);
    MARS_SYS_COMM4 = 4;

    *pclipped = clipped;
    return drawcnt;
}

int draw_tilemap(tilemap_t *tm, int fpcamera_x, int fpcamera_y, int *cameraclip, void (*drawspr)(int l, void *p), void *sprp)
{
    int i;
    int clip, drawcnt;
    char parallax;
    const fixed_t *bplx = tm->layers[0].parallax;
    drawtilecontext_t dc;

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
        const dtilelayer_t *tl = &tm->layers[i];
        const fixed_t *tplx = tl->parallax;
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
            fixed_t camera_y = FixedMul(fpcamera_y, mdpl->parallax[1])>>16;

            HwMdHScrollPlane(i, mdpl->offset[0]+camera_x);
            HwMdVScrollPlane(i, -mdpl->offset[1]-camera_y);
        }
    }

    dc.tm = tm;
    dc.fpcamera_x = fpcamera_x;
    dc.fpcamera_y = fpcamera_y;
    dc.drawspr = drawspr;
    dc.sprp = sprp;
    dc.sprites_drawn = 0;
    dc.parallax = parallax;

    drawcnt = draw_tile_layer(&dc, 0, cameraclip);
    for (i = 1; i < tm->numlayers; i++)
        drawcnt += draw_tile_layer(&dc, i, &clip);

    if (drawspr && !dc.sprites_drawn)
    {
        drawspr(1, sprp);
        dc.sprites_drawn = 1;
    }

    return drawcnt;
}
