#include <stdint.h>
#include <stddef.h>

void set_planeBBitmap(short* imgptr) __attribute__((section(".data")));
void set_palette(short* pal, int start, int count);
void clear_screen(void);
void map_screen(void);
void set_vram(int offset, int val);
void next_vram(int val);

#define VRAM_OFFS(base,y) (0x1C00 + (((((y) & 0xF8) << 3) + (((y) & 0xF8) << 5)) << 2) + (((y) & 7) << 2))

void set_planeBBitmap(short* imgptr)
{
    int x, y;
    int w = imgptr[0];
    int h = imgptr[1];

    set_palette(imgptr + 2, 0, 16);

    clear_screen();

    imgptr += 18;
    for (y = 0; y < h; y++)
    {
        int offs = VRAM_OFFS(0x1C00, y);
        for (x = 0; x < w; x += 8)
        {
            set_vram(offs, imgptr[0]);
            next_vram(imgptr[1]);
            imgptr += 2;
            offs += 32;
        }
    }

    for (y = h; y < 224; y++)
    {
        int offs = VRAM_OFFS(0x1C00, y);
        for (x = 0; x < w; x += 8)
        {
            set_vram(offs, 0);
            next_vram(0);
            offs += 32;
        }
    }

    // show bitmap
    map_screen();
}
