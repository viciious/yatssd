#ifndef DRAW_INC_H__
#define DRAW_INC_H__

#include "fixed.h"

#define TOKENPASTE( x, y ) x ## y
#define TOKENPASTE2( x, y ) TOKENPASTE( x, y )
#define TOKENPASTE3( x, y, z ) TOKENPASTE2( x, TOKENPASTE2( y, z ) )

#define DUINT_EVAL(x) TOKENPASTE3(uint,x,_t)
#define DUINT DUINT_EVAL(DRAW_DST_BITS)
#define DUINT_RSH (sizeof(DUINT)-1)

#define DFUNC_EVAL(x,y) TOKENPASTE3(draw,x,y)
#define DFUNC(y) DFUNC_EVAL(DRAW_DST_BITS,y)

#endif

#ifdef DSWAP_BYTE
#undef DSWAP_BYTE
#endif

#if DRAW_DST_BITS > 8
#define DSWAP_BYTE(b) __asm volatile("swap.b %0, %0\n\t" : "+r" (b))
#else
#define DSWAP_BYTE(b)
#endif

extern int32_t canvas_pitch;
extern int nodraw;

typedef void(*DFUNC(_spr8func_t))(DUINT *, drawsprcmd_t *cmd);

void DFUNC(_sprite8_flip0or2)(DUINT* fb, drawsprcmd_t* cmd) __attribute__((section(".data"), aligned(16)));
void DFUNC(_sprite8_flip1)(DUINT* fb, drawsprcmd_t* cmd) __attribute__((section(".data"), aligned(16)));
void DFUNC(_sprite8_scale_flip0or2)(DUINT* fb, drawsprcmd_t* cmd) __attribute__((section(".data"), aligned(16)));
void DFUNC(_sprite8_scale_flip1)(DUINT* fb, drawsprcmd_t* cmd) __attribute__((section(".data"), aligned(16)));
void DFUNC(_sprite8_scale_flip0or2)(DUINT* fb, drawsprcmd_t* cmd) __attribute__((section(".data"), aligned(16)));
void DFUNC(_sprite8_scale_flip1)(DUINT* fb, drawsprcmd_t* cmd) __attribute__((section(".data"), aligned(16)));
void DFUNC(_sprite8_scale_scale_flip0or2)(DUINT* fb, drawsprcmd_t* cmd) __attribute__((section(".data"), aligned(16)));
void DFUNC(_sprite8_scale_scale_flip1)(DUINT* fb, drawsprcmd_t* cmd) __attribute__((section(".data"), aligned(16)));

#define PIX_LOOP(n) do { \
        unsigned i, j, k; \
        for (i = 0; i < h; i++) { \
            DUINT *d = td; \
            const DUINT *s = ts; \
            for (j = 0; j < hw; ) { \
                for (k = 0; k < n; k++, j++) { \
                    *d++ = *s++; \
                } \
            } \
            ts += hsw; \
            td += hdw; \
        } \
    } while (0)

#if 1
#define PIX_LOOP2(n) do { \
        unsigned i, j, k; \
        for (i = 0; i < h; i++) { \
            int *d = (int *)td; \
            const int *s = (const int *)ts; \
            for (j = 0; j < hw; ) { \
                for (k = 0; k < n>>2; k++, j+=4) { \
                    *d++ = *s++; \
                    *d++ = *s++; \
                } \
            } \
            ts += hsw; \
            td += hdw; \
        } \
    } while (0)
#else
#define PIX_LOOP2(n) do { \
        unsigned i, j, k; \
        DUINT*td1 = td, *td2 = td; \
        const DUINT*ts1 = ts, *ts2 = ts; \
        for (i = 0; i < h>>1; i++) { \
            int *d1 = (int *)(td1); \
            int *d2 = (int *)(td2 + hdw); \
            const int *s1 = (const int *)(ts1); \
            const int *s2 = (const int *)(ts2 + hsw); \
            for (j = 0; j < hw; ) { \
                for (k = 0; k < n>>2; k++, j+=4) { \
                    *d1++ = *s1++; \
                    *d2++ = *s2++; \
                    *d1++ = *s1++; \
                    *d2++ = *s2++; \
                } \
            } \
            ts1 += hsw<<1; \
            ts2 += hsw<<1; \
            td1 += hdw<<1; \
            td2 += hdw<<1; \
        } \
    } while (0)
#endif

void DFUNC(_sprite8_flip0or2)(DUINT * fb, drawsprcmd_t * cmd)
{
    DUINT*td;
    const DUINT*ts = (const DUINT*)cmd->sdata;
    unsigned hw, hsw;
    int hdw;
    unsigned x = cmd->x, y = cmd->y;
    unsigned w = cmd->w, h = cmd->h;

    if (nodraw) return;

    hw = w >> DUINT_RSH;
    hsw = cmd->sw >> DUINT_RSH;
    hdw = canvas_pitch >> DUINT_RSH;
    if (hw == 0)
        return;

    if (cmd->flags & (DRAWSPR_HFLIP|DRAWSPR_VFLIP)) {
        y = y + h - 1;
        hdw = -hdw;
    }

    td = (DUINT*)fb + ((y*canvas_pitch + x) >> DUINT_RSH);
    ts += hsw * cmd->sy + (cmd->sx >> DUINT_RSH);

    if (sizeof(DUINT) == 2 && !(hw & 3) && !((intptr_t)ts & 3) && !((intptr_t)td & 3)) {
        PIX_LOOP2(4);
        return;
    }

    if (sizeof(DUINT) == 1 && (x & 1) && (hw > 4) && !((intptr_t)ts & 1) && (cmd->flags & DRAWSPR_OVERWRITE)) {
        unsigned i, count, nn;

        count = (hw - 1) >> 1;
        nn = (count + 15) >> 4;

        for (i = 0; i < h; i++) {
            uint16_t* d = (uint16_t*)(td + 1);
            const uint16_t* s = (const uint16_t*)ts;
            uint32_t sp;
            unsigned n = nn;

            sp = *s++ << 16;
            sp |= *s++;
            sp <<= 8;

            td[0] = ts[0];

#define DO_PIXEL() do { *d++ = sp >> 16; sp = (sp << 16) | (*s++ << 8); } while (0)

            switch (count & 15)
            {
            case 0: do { DO_PIXEL();
            case 15:     DO_PIXEL();
            case 14:     DO_PIXEL();
            case 13:     DO_PIXEL();
            case 12:     DO_PIXEL();
            case 11:     DO_PIXEL();
            case 10:     DO_PIXEL();
            case 9:      DO_PIXEL();
            case 8:      DO_PIXEL();
            case 7:      DO_PIXEL();
            case 6:      DO_PIXEL();
            case 5:      DO_PIXEL();
            case 4:      DO_PIXEL();
            case 3:      DO_PIXEL();
            case 2:      DO_PIXEL();
            case 1:      DO_PIXEL();
            } while (--n > 0);
            }
#undef DO_PIXEL

            td[hw - 1] = ts[hw - 1];
            ts += hsw;
            td += hdw;
        }

        return;
    }

    if (!(hw & 7)) {
        PIX_LOOP(8);
    } else if (!(hw & 3)) {
        PIX_LOOP(4);
    } else if (!(hw & 1)) {
        PIX_LOOP(2);
    } else {
        PIX_LOOP(1);
    }
}

#undef PIX_LOOP

void DFUNC(_sprite8_scale_flip0or2)(DUINT *fb, drawsprcmd_t *cmd)
{
    unsigned i, nn;
    DUINT*td;
    const DUINT*ts = (const DUINT*)cmd->sdata;
    unsigned hw, hsw;
    unsigned step;
    unsigned u, v;
    unsigned umask, vmask, ustart;
    int hdw;
    unsigned x = cmd->x, y = cmd->y;
    unsigned h = cmd->h;

    if (nodraw) return;

    SH2_DIVU_DVSR = cmd->scale; // set 32-bit divisor
    SH2_DIVU_DVDNTH = 1;   // set high bits of the 64-bit dividend
    SH2_DIVU_DVDNTL = 0;   // set low  bits of the 64-bit dividend, start divide

    hw = cmd->w >> DUINT_RSH;
    hsw = cmd->sw >> DUINT_RSH;
    hdw = canvas_pitch >> DUINT_RSH;
    nn = (hw + 15) >> 4;
    if (hw == 0)
        return;

    if (cmd->flags & (DRAWSPR_HFLIP | DRAWSPR_VFLIP)) {
        y = y + h - 1;
        hdw = -hdw;
    }

    td = (DUINT*)fb + ((y*canvas_pitch + x) >> DUINT_RSH);

    umask = hsw - 1;
    vmask = cmd->sh - 1;
    ustart = (cmd->sx >> DUINT_RSH) << 16;
    step = SH2_DIVU_DVDNTL; // get 32-bit quotient

    v = cmd->sy << 16;
    for (i = 0; i < h; i++) {
        const DUINT* s = ts + ((v >> 16) & vmask) * hsw;
        DUINT* d = td;
        unsigned n = nn;

#define DO_PIXEL() do { *d++ = s[(u >> 16) & umask]; u += step; } while (0)

        u = ustart;
        switch (hw & 15)
	    {
	    case 0: do { DO_PIXEL();
        case 15:     DO_PIXEL();
        case 14:     DO_PIXEL();
        case 13:     DO_PIXEL();
        case 12:     DO_PIXEL();
        case 11:     DO_PIXEL();
        case 10:     DO_PIXEL();
        case 9:      DO_PIXEL();
        case 8:      DO_PIXEL();
        case 7:      DO_PIXEL();
	    case 6:      DO_PIXEL();
	    case 5:      DO_PIXEL();
	    case 4:      DO_PIXEL();
	    case 3:      DO_PIXEL();
	    case 2:      DO_PIXEL();
	    case 1:      DO_PIXEL();
	    } while (--n > 0);
	    }

        v += step;
        td += hdw;
    }

#undef DO_PIXEL
}

#define PIX_LOOP(n)  do { \
        unsigned i, j, k; \
        for (i = 0; i < h; i++) { \
            DUINT *d = td + 1, b; \
            const DUINT *s = ts; \
            for (j = 0; j < hw; ) { \
                for (k = 0; k < n; k++, j++) { \
                    b = *s++; \
                    DSWAP_BYTE(b); \
                    *--d = b; \
                } \
            } \
            ts += hsw; \
            td += hdw; \
        } \
    } while (0)

void DFUNC(_sprite8_flip1)(DUINT* fb, drawsprcmd_t* cmd)
{
    DUINT* td;
    const DUINT* ts = (const DUINT*)cmd->sdata;
    unsigned hw, hsw;
    int hdw;
    unsigned x = cmd->x, y = cmd->y;
    unsigned w = cmd->w, h = cmd->h;

    if (nodraw) return;

    hw = cmd->w >> DUINT_RSH;
    hsw = cmd->sw >> DUINT_RSH;
    hdw = canvas_pitch >> DUINT_RSH;
    if (hw == 0)
        return;

    if (cmd->flags & DRAWSPR_VFLIP) {
        y = y + h - 1;
        hdw = -hdw;
    }

    x = x + w - 1;
    td = (DUINT*)fb + ((y * canvas_pitch + x) >> DUINT_RSH);
    ts += hsw * cmd->sy + (cmd->sx >> DUINT_RSH);

    if (sizeof(DUINT) == 1 && !(x & 1) && (w > 3) && !(cmd->sx & 1) && (cmd->flags & DRAWSPR_OVERWRITE)) {
        unsigned i, count, nn;

        count = (hw - 1) >> 1;
        nn = (count + 15) >> 4;

        for (i = 0; i < h; i++) {
            uint16_t* d = (uint16_t*)(td);
            const uint16_t* s = (const uint16_t*)ts;
            uint32_t sp;
            unsigned n = nn;

            sp = *s++ << 16;
            sp |= *s++;
            sp <<= 8;

            td[0] = ts[0];

#define DO_PIXEL() do { uint16_t b = sp>>16; __asm ("swap.b %0, %0\n\t" : "+r" (b)); *--d = b; sp <<= 16; sp |= *s++ << 8; } while (0)

            switch (count & 15)
            {
            case 0: do { DO_PIXEL();
            case 15:     DO_PIXEL();
            case 14:     DO_PIXEL();
            case 13:     DO_PIXEL();
            case 12:     DO_PIXEL();
            case 11:     DO_PIXEL();
            case 10:     DO_PIXEL();
            case 9:      DO_PIXEL();
            case 8:      DO_PIXEL();
            case 7:      DO_PIXEL();
            case 6:      DO_PIXEL();
            case 5:      DO_PIXEL();
            case 4:      DO_PIXEL();
            case 3:      DO_PIXEL();
            case 2:      DO_PIXEL();
            case 1:      DO_PIXEL();
            } while (--n > 0);
            }
#undef DO_PIXEL

            td[-hw + 1] = ts[hw - 1];
            ts += hsw;
            td += hdw;
        }

        return;
    }

    if (!(hw & 7)) {
        PIX_LOOP(8);
    } else if (!(hw & 3)) {
        PIX_LOOP(4);
    } else if (!(hw & 1)) {
        PIX_LOOP(2);
    } else {
        PIX_LOOP(1);
    }
}

#undef PIX_LOOP

void DFUNC(_sprite8_scale_flip1)(DUINT* fb, drawsprcmd_t* cmd)
{
    unsigned i, nn;
    DUINT*td;
    const DUINT*ts = (const DUINT*)cmd->sdata;
    unsigned hw, hsw;
    unsigned step;
    unsigned u, v;
    unsigned umask, vmask, ustart;
    int hdw;
    unsigned x = cmd->x, y = cmd->y;
    unsigned w = cmd->w, h = cmd->h;

    if (nodraw) return;

    SH2_DIVU_DVSR = cmd->scale; // set 32-bit divisor
    SH2_DIVU_DVDNTH = 1;   // set high bits of the 64-bit dividend
    SH2_DIVU_DVDNTL = 0;   // set low  bits of the 64-bit dividend, start divide

    hw = cmd->w >> DUINT_RSH;
    hsw = cmd->sw >> DUINT_RSH;
    hdw = canvas_pitch >> DUINT_RSH;
    nn = (hw + 15) >> 4;
    if (hw == 0)
        return;

    if (cmd->flags & DRAWSPR_VFLIP) {
        y = y + h - 1;
        hdw = -hdw;
    }

    x = x + w - 1;
    td = (DUINT*)fb + ((y * canvas_pitch + x) >> DUINT_RSH);

    umask = hsw - 1;
    vmask = cmd->sh - 1;
    ustart = (cmd->sx >> DUINT_RSH) << 16;
    step = SH2_DIVU_DVDNTL; // get 32-bit quotient

    v = cmd->sy << 16;
    for (i = 0; i < h; i++) {
        const DUINT* s = ts + ((v >> 16) & vmask) * hsw;
        DUINT* d = td + 1;
        unsigned n = nn;

#define DO_PIXEL() do { DUINT b = s[(u >> 16) & umask]; u += step; DSWAP_BYTE(b); *--d = b; } while (0)

        u = ustart;
        switch (hw & 15)
	    {
	    case 0: do { DO_PIXEL();
        case 15:     DO_PIXEL();
        case 14:     DO_PIXEL();
        case 13:     DO_PIXEL();
        case 12:     DO_PIXEL();
        case 11:     DO_PIXEL();
        case 10:     DO_PIXEL();
        case 9:      DO_PIXEL();
        case 8:      DO_PIXEL();
        case 7:      DO_PIXEL();
	    case 6:      DO_PIXEL();
	    case 5:      DO_PIXEL();
	    case 4:      DO_PIXEL();
	    case 3:      DO_PIXEL();
	    case 2:      DO_PIXEL();
	    case 1:      DO_PIXEL();
	    } while (--n > 0);
	    }

        v += step;
        td += hdw;
    }

#undef DO_PIXEL
}

static DFUNC(_spr8func_t) DFUNC(spr8funcs)[] = {
    DFUNC(_sprite8_flip0or2),
    DFUNC(_sprite8_flip1),
    DFUNC(_sprite8_flip1),
    DFUNC(_sprite8_flip0or2),
    DFUNC(_sprite8_scale_flip0or2),
    DFUNC(_sprite8_scale_flip1),
    DFUNC(_sprite8_scale_flip1),
    DFUNC(_sprite8_scale_flip0or2),
};
