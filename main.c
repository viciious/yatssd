#include <stdlib.h>
#include <stdint.h>
#include "32x.h"
#include "hw_32x.h"
#include "sound.h"
#include "types.h"
#include "draw.h"
#include "pal.h"
#include "tiles.h"

uint8_t test32x32_trans_smileData[] __attribute__((aligned(16))) =
{
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,02,02,02,02,02,02,02,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,02,02,02,03,03,03,03,03,03,03,02,02,02,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,02,02,03,03,03,03,03,03,03,03,03,03,03,03,03,02,02,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,00,00,00,00,00,00,
    00,00,00,00,00,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,00,00,00,00,00,
    00,00,00,00,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,00,00,00,00,
    00,00,00,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,00,00,00,
    00,00,02,03,03,03,03,03,03,04,04,04,03,03,03,03,03,03,03,03,02,02,02,03,03,03,03,03,02,00,00,00,
    00,00,02,03,03,03,03,03,04,04,04,04,04,03,03,03,03,03,03,02,02,02,02,02,03,03,03,03,02,00,00,00,
    00,02,03,03,03,03,03,03,04,04,04,04,04,03,03,03,03,03,03,02,02,02,02,02,03,03,03,03,03,02,00,00,
    00,02,03,03,03,03,03,03,04,04,04,04,04,03,03,03,03,03,03,02,02,02,02,02,03,03,03,03,03,02,00,00,
    00,02,03,03,03,03,03,03,03,04,04,04,03,03,03,03,03,03,03,03,02,02,02,03,03,03,03,03,03,02,00,00,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,
    00,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,00,
    00,02,03,03,03,03,03,03,02,02,02,02,02,02,02,02,02,02,02,02,02,02,02,02,03,03,03,03,03,02,00,00,
    00,02,03,03,03,03,03,03,03,02,02,02,02,02,02,02,02,02,02,02,02,02,02,03,03,03,03,03,03,02,00,00,
    00,00,02,03,03,03,03,03,03,03,02,02,02,02,02,02,02,02,02,02,02,02,03,03,03,03,03,03,02,00,00,00,
    00,00,02,03,03,03,03,03,03,03,03,03,02,02,02,02,02,02,02,02,03,03,03,03,03,03,03,03,02,00,00,00,
    00,00,00,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,00,00,00,
    00,00,00,00,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,00,00,00,00,
    00,00,00,00,00,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,00,00,00,00,00,
    00,00,00,00,00,00,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,02,02,03,03,03,03,03,03,03,03,03,03,03,03,03,02,02,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,02,02,02,03,03,03,03,03,03,03,02,02,02,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,02,02,02,02,02,02,02,00,00,00,00,00,00,00,00,00,00,00,00,00,
};

uint8_t test32x32_solid_smileData[] __attribute__((aligned(16))) =
{
    05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,05,
    05,05,05,05,05,05,05,05,05,05,05,05,02,02,02,02,02,02,02,05,05,05,05,05,05,05,05,05,05,05,05,05,
    05,05,05,05,05,05,05,05,05,02,02,02,03,03,03,03,03,03,03,02,02,02,05,05,05,05,05,05,05,05,05,05,
    05,05,05,05,05,05,05,02,02,03,03,03,03,03,03,03,03,03,03,03,03,03,02,02,05,05,05,05,05,05,05,05,
    05,05,05,05,05,05,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,05,05,05,05,05,05,
    05,05,05,05,05,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,05,05,05,05,05,
    05,05,05,05,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,05,05,05,05,
    05,05,05,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,05,05,05,
    05,05,02,03,03,03,03,03,03,04,04,04,03,03,03,03,03,03,03,03,02,02,02,03,03,03,03,03,02,05,05,05,
    05,05,02,03,03,03,03,03,04,04,04,04,04,03,03,03,03,03,03,02,02,02,02,02,03,03,03,03,02,05,05,05,
    05,02,03,03,03,03,03,03,04,04,04,04,04,03,03,03,03,03,03,02,02,02,02,02,03,03,03,03,03,02,05,05,
    05,02,03,03,03,03,03,03,04,04,04,04,04,03,03,03,03,03,03,02,02,02,02,02,03,03,03,03,03,02,05,05,
    05,02,03,03,03,03,03,03,03,04,04,04,03,03,03,03,03,03,03,03,02,02,02,03,03,03,03,03,03,02,05,05,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,
    02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,
    05,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,05,
    05,02,03,03,03,03,03,03,02,02,02,02,02,02,02,02,02,02,02,02,02,02,02,02,03,03,03,03,03,02,05,05,
    05,02,03,03,03,03,03,03,03,02,02,02,02,02,02,02,02,02,02,02,02,02,02,03,03,03,03,03,03,02,05,05,
    05,05,02,03,03,03,03,03,03,03,02,02,02,02,02,02,02,02,02,02,02,02,03,03,03,03,03,03,02,05,05,05,
    05,05,02,03,03,03,03,03,03,03,03,03,02,02,02,02,02,02,02,02,03,03,03,03,03,03,03,03,02,05,05,05,
    05,05,05,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,05,05,05,
    05,05,05,05,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,05,05,05,05,
    05,05,05,05,05,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,05,05,05,05,05,
    05,05,05,05,05,05,02,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,02,05,05,05,05,05,05,05,
    05,05,05,05,05,05,05,02,02,03,03,03,03,03,03,03,03,03,03,03,03,03,02,02,05,05,05,05,05,05,05,05,
    05,05,05,05,05,05,05,05,05,02,02,02,03,03,03,03,03,03,03,02,02,02,05,05,05,05,05,05,05,05,05,05,
    05,05,05,05,05,05,05,05,05,05,05,05,02,02,02,02,02,02,02,05,05,05,05,05,05,05,05,05,05,05,05,05,
};

extern drawsprcmd_t slave_drawsprcmd;
extern drawspr4cmd_t slave_drawspr4cmd;
extern drawtilelayerscmd_t slave_drawtilelayerscmd;

int old_camera_x, old_camera_y;
int camera_x, camera_y;

float fcamera_x, fcamera_y;
float fmoveinc_x = 1.0, fmoveinc_y = 1.0;

uint16_t canvas_rebuild_id;

int nodraw = 0;
int debug = 0;
int sprmode = -1;

int window_canvas_x = 0, window_canvas_y = 0;

volatile unsigned mars_pwdt_ovf_count = 0;
volatile unsigned mars_swdt_ovf_count = 0;
unsigned mars_frtc2msec_frac = 0;

const int NTSC_CLOCK_SPEED = 23011360; // HZ
const int PAL_CLOCK_SPEED = 22801467; // HZ

int slave_task(int cmd) HW32X_ATTR_DATA_ALIGNED;
void secondary(void) HW32X_ATTR_DATA_ALIGNED;

tilemap_t tm;

int sec;

int Mars_FRTCounter2Msec(int c)
{
    return (c * mars_frtc2msec_frac) >> 16;
}

int Mars_GetFRTCounter(void)
{
    unsigned int cnt = SH2_WDT_RTCNT;
    return (int)((mars_pwdt_ovf_count << 8) | cnt);
}

int slave_task(int cmd)
{
    switch (cmd) {
    case 1:
        snddma_slave_init(22050);
        return 1;
    case 2:
        return 1;
    case 3:
        ClearCacheLines(&slave_drawsprcmd, (sizeof(drawsprcmd_t) + 15) / 16);
        draw_handle_drawspritecmd(&slave_drawsprcmd);
        return 1;
    case 4:
        return 1;
    case 5:
        ClearCacheLines((uintptr_t)&canvas_width & ~15, 1);
        ClearCacheLines((uintptr_t)&canvas_height & ~15, 1);
        ClearCacheLines((uintptr_t)&window_canvas_x & ~15, 1);
        ClearCacheLines((uintptr_t)&window_canvas_y & ~15, 1);
        ClearCacheLines((uintptr_t)&old_camera_x & ~15, 1);
        ClearCacheLines((uintptr_t)&old_camera_x & ~15, 1);
        ClearCacheLines((uintptr_t)&canvas_pitch & ~15, 1);
        ClearCacheLines((uintptr_t)&canvas_yaw & ~15, 1);
        ClearCacheLines((uintptr_t)&camera_x & ~15, 1);
        ClearCacheLines((uintptr_t)&camera_y & ~15, 1);
        ClearCacheLines((uintptr_t)&nodraw & ~15, 1);
        ClearCacheLines(&slave_drawtilelayerscmd, (sizeof(drawtilelayerscmd_t) + 15) / 16);
        ClearCacheLines(&tm, (sizeof(tilemap_t) + 15) / 16);
        draw_tile_layer(&slave_drawtilelayerscmd);
        return 1;
    default:
        break;
    }

    return 0;
}

void secondary(void)
{
    ClearCache();

    while (1) {
        int cmd;

        while ((cmd = MARS_SYS_COMM4) == 0) {}

        int res = slave_task(cmd);
        if (res > 0) {
            MARS_SYS_COMM4 = 0;
        }
    }
}

void display(int framecount, int hudenable, int fpscount, int totaltics, int clearhud)
{
    int i, j;
    int start = Mars_GetFRTCounter(), total;
    int drawcnt = 0;
    static int prevsec;
    static int maxdrawcnt;

    if (prevsec != sec)
    {
        maxdrawcnt = 0;
        prevsec = sec;
    }

    draw_setScissor(0, 0, 320, 224);

    drawcnt = draw_tilemap(&tm);
    if (drawcnt > maxdrawcnt)
    {
        maxdrawcnt = drawcnt;
    }
    total = Mars_GetFRTCounter() - start;

    draw_setScissor(0, 0, 320, 224);

    if (sprmode >= 0)
    {
        int mode = DRAWSPR_OVERWRITE;
        if (sprmode < 3)
            mode |= sprmode | DRAWSPR_PRECISE;
        else
            mode |= (sprmode - 3);
        for (j = 0; j < 5; j++)
        {
            for (i = 0; i < 5; i++)
                draw_sprite(i * 64 + 16, j * 64 + 16, 32, 32, test32x32_trans_smileData, DRAWSPR_OVERWRITE | mode, 1);
        }
        draw_pivot_stretch_sprite(160, 112, 32, 32, test32x32_trans_smileData, DRAWSPR_SCALE | mode, 0x10000 + ((start * 16) & 0xffff));
    }

    Hw32xScreenSetXY(0, 23);

    switch (hudenable) {
    case 1:
        Hw32xScreenPrintf("fps:%02d %03d ms:%02d", fpscount, maxdrawcnt, Mars_FRTCounter2Msec(total));
        break;
    default:
        break;
    }
}

int main(void)
{
    int framecount;
    int fpscount;
    int prevsec;
    int prevsecframe;
    int totaltics;
    char hud = 0, clearhud = 0;
    int ticksperframe;
    int buttons, oldbuttons;
    unsigned i;
    char NTSC;

    Hw32xInit(MARS_VDP_MODE_256, 0);

    SetSH2SR(1);

    while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0);

    NTSC = (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT) != 0;

    SH2_WDT_WTCSR_TCNT = 0x5A00; /* WDT TCNT = 0 */
    SH2_WDT_WTCSR_TCNT = 0xA53E; /* WDT TCSR = clr OVF, IT mode, timer on, clksel = Fs/4096 */

/* init hires timer system */
    SH2_WDT_VCR = (65 << 8) | (SH2_WDT_VCR & 0x00FF); // set exception vector for WDT
    SH2_INT_IPRA = (SH2_INT_IPRA & 0xFF0F) | 0x0020; // set WDT INT to priority 2

    // change 4096.0f to something else if WDT TCSR is changed!
    mars_frtc2msec_frac = 4096.0f * 1000.0f / (NTSC ? NTSC_CLOCK_SPEED : PAL_CLOCK_SPEED) * 65536.0f;

    for (i = 0; i < 256; i++) {
        if (palette[i * 3 + 0] == 0 && palette[i * 3 + 1] == 0 && palette[i * 3 + 2] == 0)
            Hw32xSetBGColor(i, palette[i * 3 + 0] >> 3, palette[i * 3 + 1] >> 3, palette[i * 3 + 2] >> 3);
        Hw32xSetFGColor(i, palette[i * 3 + 0] >> 3, palette[i * 3 + 1] >> 3, palette[i * 3 + 2] >> 3);
    }

    MARS_SYS_COMM4 = 0;
    MARS_SYS_COMM6 = 0;

    ticksperframe = 1;

    fpscount = 0;
    framecount = 0;

    prevsec = 0;
    prevsecframe = 0;

    totaltics = 0;

    fcamera_x = fcamera_y = 0;
    camera_x = camera_y = 0;

    canvas_rebuild_id = 1;

    buttons = oldbuttons = 0;

    Hw32xScreenFlip(0);

    init_tilemap(&tm, tmx.tilew, tmx.tileh, tmx.numtw, tmx.numth, (const uint16_t **)tmx.layers, tmx.numlayers);

    while (1) {
        int starttics;
        int waittics;

        starttics = Hw32xGetTicks();

        sec = starttics / (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT ? 60 : 50); // FIXME: add proper NTSC vs PAL rate detection
        if (sec != prevsec) {
            fpscount = (framecount - prevsecframe) / (sec - prevsec);
            prevsec = sec;
            prevsecframe = framecount;
        }

        old_camera_x = camera_x;
        old_camera_y = camera_y;

        oldbuttons = buttons;
        buttons = MARS_SYS_COMM8;
        int newbuttons = (buttons ^ oldbuttons) & buttons;

        if (MARS_SYS_COMM8 & SEGA_CTRL_RIGHT) {
            fcamera_x += fmoveinc_x;
        }
        else if (MARS_SYS_COMM8 & SEGA_CTRL_LEFT) {
            fcamera_x -= fmoveinc_x;
        }

        if (MARS_SYS_COMM8 & SEGA_CTRL_DOWN) {
            fcamera_y += fmoveinc_x;
        }
        else if (MARS_SYS_COMM8 & SEGA_CTRL_UP) {
            fcamera_y -= fmoveinc_x;
        }

        if (fcamera_x < 0)
            fcamera_x = 0;
        if (fcamera_x > tm.tiles_hor * tm.tw - canvas_width)
            fcamera_x = tm.tiles_hor * tm.tw - canvas_width;

        if (fcamera_y < 0)
            fcamera_y = 0;
        if (fcamera_y > tm.tiles_ver * tm.th - canvas_height)
            fcamera_y = tm.tiles_ver * tm.th - canvas_height;

        camera_x = (int)fcamera_x;
        camera_y = (int)fcamera_y;

        if (newbuttons & SEGA_CTRL_B)
        {
            hud = (hud + 1) % 2;
            clearhud = 2;
        }

        if (newbuttons & SEGA_CTRL_C)
        {
            sprmode++;
            if (sprmode == 8)
                sprmode = -1;
        }

        Hw32xFlipWait();

        display(framecount, hud, fpscount, totaltics, clearhud);

        clearhud--;
        if (clearhud < 0)
            clearhud = 0;

        totaltics = Hw32xGetTicks() - starttics;

        waittics = totaltics;
        while (waittics < ticksperframe) {
            waittics = Hw32xGetTicks() - starttics;
        }

        framecount++;

        Hw32xScreenFlip(0);
    }

    return 0;
}
