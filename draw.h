#ifndef DRAW_H__
#define DRAW_H__

#include <stdint.h>
#include <stdlib.h>

#include "types.h"

draw_spritefn_t draw_spritefn(int flags)
ATTR_DATA_ALIGNED;

void draw_handle_drawspritecmd(drawsprcmd_t* cmd)
ATTR_DATA_ALIGNED;

int draw_sprite(int x, int y, int w, int h,
    int stride, const uint8_t* data, int flags)
    ATTR_DATA_ALIGNED;

void draw_stretch_sprite(int x, int y, int sw, int sh,
    int stride, const uint8_t* data, int flags, fixed_t scale)
    ATTR_DATA_ALIGNED;

void draw_pivot_stretch_sprite(int x, int y, int sw, int sh,
    int stride, const uint8_t* data, int flags, fixed_t scale)
    ATTR_DATA_ALIGNED;

void draw_setScissor(int16_t x, int16_t y, int16_t w, int16_t h)
ATTR_DATA_ALIGNED;


int draw_clip(int x, int y, int w, int h, rect_t* cliprect)
ATTR_DATA_ALIGNED;


void init_tilemap(tilemap_t *tm, const dtilemap_t *dtm, uint8_t **reslist);

void set_tilemap_wrap(tilemap_t *tm, fixed_t wrapX, fixed_t wrapY);

int draw_handle_layercmd(drawtilelayerscmd_t* cmd)
ATTR_DATA_ALIGNED;

int draw_tilemap(tilemap_t *tm, int fpcamera_x, int fpcamera_y, int *cameraclip, void (*drawspr)(int l, void *p), void *sprp)
ATTR_DATA_ALIGNED;

void draw_dirtyrect(tilemap_t* tm, int x, int y, int w, int h)
ATTR_DATA_ALIGNED;

extern tilemap_t tm;

extern int window_canvas_x, window_canvas_y;
extern int32_t canvas_width, canvas_height;
extern uint32_t canvas_pitch, canvas_yaw;

extern int debug;

extern drawtilelayerscmd_t slave_drawtilelayerscmd
ATTR_CACHE_ALIGNED;

#endif
