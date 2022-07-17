#include <stdint.h>
#include <stddef.h>

void set_planeBitmap(int plane, int start, short* imgptr) __attribute__((section(".data")));
void set_palette(short* pal, int start, int count);
void clear_screen(int plane);
void map_screen(int plane, int offset, int height, int ormask);
void set_vram(int offset, int val);
void next_vram(int val);

static int base_offs;

#define VRAM_OFFS(base,y) (base + (((((y) & 0xF8) << 3) + (((y) & 0xF8) << 5)) << 2) + (((y) & 7) << 2))

void clear_planesAandB(void)
{
    clear_screen(0);
    clear_screen(1);
    base_offs = 0;
}

void set_planeBitmap(int plane, int start, short* imgptr)
{
    int x, y;
    int w, h;
    int offs = 0;
    int vram_inc = 0;

    clear_screen(plane);

    if (!imgptr)
        return;

    w = imgptr[0];
    h = imgptr[1];
    if (!w || !h)
        return;

    // use palette 0 for plane A, palette 1 for plane B
    set_palette(imgptr + 2, plane*16, 16);

    imgptr += 18;
    for (y = 0; y < h; y++)
    {
        offs = VRAM_OFFS(base_offs, y);
        for (x = 0; x < w; x += 8)
        {
            set_vram(offs, imgptr[0]);
            next_vram(imgptr[1]);
            vram_inc += 4;
            imgptr += 2;
            offs += 32;
        }
    }
    base_offs += vram_inc;

    // bitmask 0x2000 sets palette 1 for plane B
    map_screen(plane, start, h/8-1, plane ? 0x2000 : 0);
}

void set_planeABitmap(short* imgptr)
{
    set_planeBitmap(0, base_offs/32, imgptr);
}

void set_planeBBitmap(short* imgptr)
{
    set_planeBitmap(1, base_offs/32, imgptr);
}
