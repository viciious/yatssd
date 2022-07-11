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
    const uint8_t* data, int flags, fixed_t scale)
    ATTR_DATA_ALIGNED;

void draw_stretch_sprite(int x, int y, int sw, int sh,
    const uint8_t* data, int flags, fixed_t scale)
    ATTR_DATA_ALIGNED;

void draw_pivot_stretch_sprite(int x, int y, int sw, int sh,
    const uint8_t* data, int flags, fixed_t scale)
    ATTR_DATA_ALIGNED;

void draw_setScissor(int16_t x, int16_t y, int16_t w, int16_t h)
ATTR_DATA_ALIGNED;


int draw_clip(int x, int y, int w, int h, rect_t* cliprect)
ATTR_DATA_ALIGNED;


void init_tilemap(tilemap_t* tm, int tw, int th, int numh, int numv, 
    const uint16_t** tmx, int nl, const int *lplx, fixed_t wrapX, fixed_t wrapY);

void draw_handle_layercmd(drawtilelayerscmd_t* cmd)
ATTR_DATA_ALIGNED;

int draw_tilemap(tilemap_t* tm, int fpcamera_x, int fpcamera_y, int *cameraclip)
ATTR_DATA_ALIGNED;

void draw_dirtyrect(tilemap_t* tm, int x, int y, int w, int h)
ATTR_DATA_ALIGNED;

extern tilemap_t tm;

extern uint16_t canvas_rebuild_id;
extern int window_canvas_x, window_canvas_y;
extern int32_t canvas_width, canvas_height;
extern uint32_t canvas_pitch, canvas_yaw;

extern int debug;

extern drawtilelayerscmd_t slave_drawtilelayerscmd
ATTR_CACHE_ALIGNED;

#endif
