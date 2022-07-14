#ifndef TYPES_H__
#define TYPES_H__

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#include "fixed.h"

#define ATTR_CACHE_ALIGNED  __attribute__((aligned(16)))
#define ATTR_DATA_ALIGNED   __attribute__((section(".data"), aligned(16)))

enum {
    DRAWSPR_NORM        = 0,
    DRAWSPR_HFLIP       = 1,
    DRAWSPR_VFLIP       = 2,

    DRAWSPR_OVERWRITE   = 4,
    DRAWSPR_PRECISE     = 8,
    DRAWSPR_SCALE       = 16,
    DRAWSPR_MULTICORE   = 32,
};

typedef struct {
    int16_t x, y;
} point_t;

typedef struct {
    int16_t x1, y1;
    int16_t x2, y2;
} rect_t;

// "on-disk" tilemap
typedef struct {
    int tilew, tileh;
    int numtw, numth;
    int numlayers;
    int wrapX, wrapY;
    int *layerplx;
    uint16_t **layers;
} dtilemap_t;

// in-memory tilemap
typedef struct {
    unsigned tw, th;

    int numlayers;
    int numtiles;
    uint16_t** layers;
    uint8_t **reslist;
    int* lplx;
    fixed_t wrapX, wrapY;

    unsigned tiles_hor, tiles_ver;
    unsigned canvas_tiles_hor, canvas_tiles_ver;
    unsigned scroll_tiles_hor, scroll_interval_hor;
    unsigned scroll_tiles_ver, scroll_interval_ver;
} tilemap_t;

typedef struct {
    void *sdata;
    uint16_t flags;
    int16_t sx, sy;
    uint16_t sw, sh;
    uint16_t x, y;
    uint16_t w, h;
    fixed_t scale;
} drawsprcmd_t;

typedef struct {
    void* sdata;
    uint16_t flags;
    int16_t sx, sy;
    uint16_t sw, sh;
    uint16_t x1, y1;
    uint16_t x2, y2;
    uint16_t x3, y3;
    uint16_t x4, y4;
    uint16_t w, h;
    fixed_t scale;
} drawspr4cmd_t;

typedef struct {
    tilemap_t* tm;
    int16_t startlayer;
    int16_t numlayers;
    int32_t camera_x, camera_y;
    int16_t x, y;
    uint16_t start_tile, end_tile;
    uint16_t scroll_tile_id;
    uint16_t num_tiles_x;
    uint16_t drawmode;
    uint16_t drawcnt;
} drawtilelayerscmd_t;

typedef void(*draw_spritefn_t)(void *dst, drawsprcmd_t* cmd);

#endif
