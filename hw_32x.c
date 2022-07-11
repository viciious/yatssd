/*
 * Licensed under the BSD license
 *
 * debug_32x.c - Debug screen functions.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * Altered for 32X by Chilly Willy
 */

#include "32x.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static int X = 0, Y = 0;
static int MX = 40, MY = 25;
static int init = 0;
static unsigned short fgc = 0, bgc = 0;
static unsigned char fgs = 0, bgs = 0;

static volatile const char *new_palette;

static volatile unsigned int mars_vblank_count = 0;

volatile unsigned short currentFB = 0;

uint32_t canvas_width = 320+4; // +4 to avoid hitting that 0xXXFF bug in the shift register
uint32_t canvas_height = 224;

// 384 seems to be the ideal value - anything thing 
// increases the odds of hitting the "0xFF screen shift
// register bug"
uint32_t canvas_pitch = 384; // canvas_width + scrollwidth
uint32_t canvas_yaw = 288; // canvas_height + scrollheight

#define UNCACHED_CURFB (*(short *)((int)&currentFB|0x20000000))

void pri_vbi_handler(void)
{
    mars_vblank_count++;

    if (new_palette)
    {
        int i;
        volatile unsigned short *palette = &MARS_CRAM;

        if ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0)
		    return;

        for (i = 0; i < 256; i++)
        {
             palette[i] = COLOR(new_palette[0], new_palette[1], new_palette[2]);
             new_palette += 3;
        }

        new_palette = NULL;
    }
}

unsigned Hw32xGetTicks(void)
{
    return mars_vblank_count;
}

void pri_dma1_handler(void)
{
    SH2_DMA_CHCR1; // read TE
    SH2_DMA_CHCR1 = 0; // clear TE
}

void Hw32xSetFGColor(int s, int r, int g, int b)
{
    volatile unsigned short *palette = &MARS_CRAM;
    fgs = s;
    fgc = COLOR(r, g, b);
    palette[fgs] = fgc;
}

void Hw32xSetBGColor(int s, int r, int g, int b)
{
    volatile unsigned short *palette = &MARS_CRAM;
    bgs = s;
    bgc = COLOR(r, g, b);
    palette[bgs] = bgc;
}

void Hw32xSetPalette(const char *palette)
{
    new_palette = palette;
}

void Hw32xUpdateLineTable(int hscroll, int vscroll, int lineskip)
{
    int i;
    int i_lineskip;
    const int ymask = canvas_yaw - 1;
    const int pitch = canvas_pitch >> 1;
    uint16_t *frameBuffer16 = (uint16_t *)&MARS_FRAMEBUFFER;

    hscroll += 0x100;

    if (lineskip == 0)
    {
        unsigned count = canvas_height;
        unsigned n = ((unsigned)count + 7) >> 3;
        const int mpitch = pitch * canvas_yaw;

#define DO_LINE() do { \
            if (ppitch >= mpitch) ppitch -= mpitch; \
            *frameBuffer16++ = ppitch + hscroll; /* word offset of line */ \
            ppitch += pitch; \
        } while(0)

        int ppitch = pitch * vscroll;
        switch (count & 7)
        {
        case 0: do { DO_LINE();
        case 7:      DO_LINE();
        case 6:      DO_LINE();
        case 5:      DO_LINE();
        case 4:      DO_LINE();
        case 3:      DO_LINE();
        case 2:      DO_LINE();
        case 1:      DO_LINE();
        } while (--n > 0);
        }
        return;
    }

    i_lineskip = 0;
    for (i = 0; i < canvas_height / (lineskip + 1); i++)
    {
        int j = lineskip + 1;
        while (j)
        {
            frameBuffer16[i_lineskip + (lineskip + 1 - j)] = pitch * (vscroll & ymask) + hscroll; /* word offset of line */
            j--;
        }
        vscroll++;
        i_lineskip += lineskip + 1;
    }
}

void Hw32xInit(int vmode, int lineskip)
{
    volatile unsigned short *frameBuffer16 = &MARS_FRAMEBUFFER;
    int i;

    // Wait for the SH2 to gain access to the VDP
    while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0) ;

    if (vmode == MARS_VDP_MODE_256)
    {
        // Set 8-bit paletted mode, 224 lines
        MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_256;

        // init both framebuffers

        // Flip the framebuffer selection bit and wait for it to take effect
        MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
        // rewrite line table
        Hw32xUpdateLineTable(0, 0, lineskip);
        // clear screen
        for (i=0x100; i<0x10000; i++)
            frameBuffer16[i] = 0;

        // Flip the framebuffer selection bit and wait for it to take effect
        MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
        // rewrite line table
        Hw32xUpdateLineTable(0, 0, lineskip);
        // clear screen
        for (i=0x100; i<0x10000; i++)
            frameBuffer16[i] = 0;

        MX = 40;
        MY = 28/(lineskip+1);
    }
    else if (vmode == MARS_VDP_MODE_32K)
    {
        // Set 16-bit direct mode, 224 lines
        MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_32K;

        // init both framebuffers

        // Flip the framebuffer selection bit and wait for it to take effect
        MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
        // rewrite line table
        for (i=0; i<canvas_height/(lineskip+1); i++)
        {
            if (lineskip)
            {
                int j = lineskip + 1;
                while (j)
                {
                    frameBuffer16[i*(lineskip+1) + (lineskip + 1 - j)] = i* canvas_pitch + 0x100; /* word offset of line */
                    j--;
                }
            }
            else
            {
                if (i<200)
                    frameBuffer16[i] = i* canvas_pitch + 0x100; /* word offset of line */
                else
                    frameBuffer16[i] = 200* canvas_pitch + 0x100; /* word offset of line */
            }
        }
        // clear screen
        for (i=0x100; i<0x10000; i++)
            frameBuffer16[i] = 0;

        // Flip the framebuffer selection bit and wait for it to take effect
        MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
        // rewrite line table
        for (i=0; i<canvas_height/(lineskip+1); i++)
        {
            if (lineskip)
            {
                int j = lineskip + 1;
                while (j)
                {
                    frameBuffer16[i*(lineskip+1) + (lineskip + 1 - j)] = i* canvas_pitch + 0x100; /* word offset of line */
                    j--;
                }
            }
            else
            {
                if (i<200)
                    frameBuffer16[i] = i* canvas_pitch + 0x100; /* word offset of line */
                else
                    frameBuffer16[i] = 200* canvas_pitch + 0x100; /* word offset of line */
            }
        }
        // clear screen
        for (i=0x100; i<0x10000; i++)
            frameBuffer16[i] = 0;

        MX = 40;
        MY = 25/(lineskip+1);
    }

    Hw32xSetFGColor(255,31,31,31);
    Hw32xSetBGColor(0,0,0,0);
    X = Y = 0;
    init = vmode;
}

int Hw32xScreenGetX()
{
    return X;
}

int Hw32xScreenGetY()
{
    return Y;
}

void Hw32xScreenSetXY(int x, int y)
{
    if( x<MX && x>=0 )
        X = x;
    if( y<MY && y>=0 )
        Y = y;
}

void Hw32xScreenClear()
{
    int i;
    int l = (init == MARS_VDP_MODE_256) ? canvas_pitch *224/2 + 0x100 : canvas_pitch *200 + 0x100;
    volatile unsigned short *frameBuffer16 = &MARS_FRAMEBUFFER;

    // clear screen
    for (i=0x100; i<l; i++)
        frameBuffer16[i] = 0;

    // Flip the framebuffer selection bit and wait for it to take effect
    MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
    while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
    UNCACHED_CURFB ^= 1;

    // clear screen
    for (i=0x100; i<l; i++)
        frameBuffer16[i] = 0;

    Hw32xSetFGColor(255,31,31,31);
    Hw32xSetBGColor(0,0,0,0);
    X = Y = 0;
}

extern unsigned char msx[];

static void debug_put_char_16(int x, int y, unsigned char ch)
{
    volatile unsigned short *fb = &MARS_FRAMEBUFFER;
    int i,j;
    unsigned char *font;
    int vram, vram_ptr;

    if(!init)
    {
        return;
    }

    vram = 0x100 + x * 8;
    vram += (y * 8 * canvas_pitch);

    font = &msx[ (int)ch * 8];

    for (i=0; i<8; i++, font++)
    {
        vram_ptr  = vram;
        for (j=0; j<8; j++)
        {
            if ((*font & (128 >> j)))
                fb[vram_ptr] = fgc;
            else
                fb[vram_ptr] = bgc;
            vram_ptr++;
        }
        vram += canvas_pitch;
    }
}

static void debug_put_char_8(int x, int y, unsigned char ch)
{
    volatile unsigned char *fb = (volatile unsigned char *)&MARS_FRAMEBUFFER;
    int i,j;
    unsigned char *font;
    int vram, vram_ptr;

    if(!init)
    {
        return;
    }

    vram = 0x200 + x * 8;
    vram += (y * 8 * canvas_pitch);

    font = &msx[ (int)ch * 8];

    for (i=0; i<8; i++, font++)
    {
        vram_ptr  = vram;
        for (j=0; j<8; j++)
        {
            if ((*font & (128 >> j)))
                fb[vram_ptr] = fgs;
            else
                fb[vram_ptr] = bgs;
            vram_ptr++;
        }
        vram += canvas_pitch;
    }
}

void Hw32xScreenPutChar(int x, int y, unsigned char ch)
{
    if (init == MARS_VDP_MODE_256)
    {
        debug_put_char_8(x, y, ch);
    }
    else if (init == MARS_VDP_MODE_32K)
    {
        debug_put_char_16(x, y, ch);
    }
}

void Hw32xScreenClearLine(int Y)
{
    int i;

    for (i=0; i < MX; i++)
    {
        Hw32xScreenPutChar(i, Y, ' ');
    }
}

/* Print non-nul terminated strings */
int Hw32xScreenPrintData(const char *buff, int size)
{
    int i;
    char c;

    if(!init)
    {
        return 0;
    }

    for (i = 0; i<size; i++)
    {
        c = buff[i];
        switch (c)
        {
            case '\r':
                X = 0;
                break;
            case '\n':
                X = 0;
                Y++;
                if (Y >= MY)
                    Y = 0;
                Hw32xScreenClearLine(Y);
                break;
            case '\t':
                X = (X + 4) & ~3;
                if (X >= MX)
                {
                    X = 0;
                    Y++;
                    if (Y >= MY)
                        Y = 0;
                    Hw32xScreenClearLine(Y);
                }
                break;
            default:
                Hw32xScreenPutChar(X, Y, c);
                X++;
                if (X >= MX)
                {
                    X = 0;
                    Y++;
                    if (Y >= MY)
                        Y = 0;
                    Hw32xScreenClearLine(Y);
                }
        }
    }

    return i;
}

int Hw32xScreenPutsn(const char *str, int len)
{
    int ret;

    // Flip the framebuffer selection bit and wait for it to take effect
    //MARS_VDP_FBCTL = currentFB ^ 1;
    //while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
    //currentFB ^= 1;

    ret = Hw32xScreenPrintData(str, len);

    // Flip the framebuffer selection bit and wait for it to take effect
    //MARS_VDP_FBCTL = currentFB ^ 1;
    //while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
    //currentFB ^= 1;

    return ret;
}

void Hw32xScreenPrintf(const char *format, ...)
{
   va_list  opt;
   char     buff[128];
   int      n;

   va_start(opt, format);
   n = vsnprintf(buff, (size_t)sizeof(buff), format, opt);
   va_end(opt);
   buff[sizeof(buff) - 1] = 0;

   Hw32xScreenPutsn(buff, n);
}

void Hw32xDelay(int ticks)
{
    unsigned long ct = mars_vblank_count + ticks;
    while (mars_vblank_count < ct) ;
}

void Hw32xScreenFlip(int wait)
{
    // Flip the framebuffer selection bit
    MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
    if (wait)
    {
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
    }
}

void Hw32xFlipWait(void)
{
    while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB);
    UNCACHED_CURFB ^= 1;
}
