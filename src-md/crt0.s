| SEGA 32X support code for the 68000
| by Chilly Willy

        .text

| 0x880800 - entry point for reset/cold-start

        .global _start
_start:

| Clear Work RAM
        moveq   #0,d0
        move.w  #0x3FFF,d1
        suba.l  a1,a1
1:
        move.l  d0,-(a1)
        dbra    d1,1b

| Copy initialized variables from ROM to Work RAM
        lea     __text_end,a0
        move.w  #__data_size,d0
        lsr.w   #1,d0
        subq.w  #1,d0
2:
        move.w  (a0)+,(a1)+
        dbra    d0,2b

        lea     __stack,sp              /* set stack pointer to top of Work RAM */

        bsr     init_hardware           /* initialize the console hardware */

        jsr     main                    /* call program main() */
3:
        stop    #0x2700
        bra.b   3b

        .align  64

| 0x880840 - 68000 General exception handler

        move.l  d0,-(sp)
        move.l  4(sp),d0            /* jump table return address */
        sub.w   #0x206,d0           /* 0 = BusError, 6 = AddrError, etc */

        /* handle exception here */

        move.l  (sp)+,d0
        addq.l  #4,sp               /* pop jump table return address */
        rte

        .align  64

| 0x880880 - 68000 Level 4 interrupt handler - HBlank IRQ

        rte

        .align  64

| 0x8808C0 - 68000 Level 6 interrupt handler - VBlank IRQ

        move.l  d0,-(sp)
        move.l  vblank,d0
        beq.b   1f
        move.l  a0,-(sp)
        movea.l d0,a0
        jmp     (a0)
1:
        move.l  (sp)+,d0
        rte


| Initialize the MD side to a known state for the game

init_hardware:
        lea     0xC00004,a0
        move.w  #0x8104,(a0)            /* display off, vblank disabled */
        move.w  (a0),d0                 /* read VDP Status reg */

| init joyports
        move.b  #0x40,0xA10009
        move.b  #0x40,0xA1000B
        move.b  #0x40,0xA10003
        move.b  #0x40,0xA10005

| init MD VDP
        move.w  #0x8004,(a0) /* reg. 0 - Disable HBL INT */
        move.w  #0x8174,(a0) /* reg. 1 - Enable display, VBL INT, DMA + 28 VCell size */
        move.w  #0x8230,(a0) /* reg. 2 - Plane A =$30*$400=$C000 */
        move.w  #0x832C,(a0) /* reg. 3 - Window  =$2C*$400=$B000 */
        move.w  #0x8407,(a0) /* reg. 4 - Plane B =$7*$2000=$E000 */
        move.w  #0x855E,(a0) /* reg. 5 - sprite table begins at $BC00=$5E*$200 */
        move.w  #0x8600,(a0) /* reg. 6 - not used */
        move.w  #0x8700,(a0) /* reg. 7 - Background Color number*/
        move.w  #0x8800,(a0) /* reg. 8 - not used */
        move.w  #0x8900,(a0) /* reg. 9 - not used */
        move.w  #0x8A01,(a0) /* reg 10 - HInterrupt timing */
        move.w  #0x8B00,(a0) /* reg 11 - $0000abcd a=extr.int b=vscr cd=hscr */
        move.w  #0x8C81,(a0) /* reg 12 - hcell mode + shadow/highight + interlaced mode (40 cell, no shadow, no interlace)*/
        move.w  #0x8D2B,(a0) /* reg 13 = HScroll Tbl = 0xAC00 */
        move.w  #0x8E00,(a0) /* reg 14 - not used */
        move.w  #0x8F02,(a0) /* reg 15 - auto increment data */
        move.w  #0x9011,(a0) /* reg 16 - scrl screen v&h size (64x64) */
        move.w  #0x9100,(a0) /* reg 17 - window hpos */
        move.w  #0x92FF,(a0) /* reg 18 - window vpos */

        move.w  #0,0xA15128         /* controller 1 */
        move.w  #0,0xA1512A         /* controller 2 */
| look for mouse
        lea     0xA10003,a0
0:
        jsr     get_mky
        cmpi.l  #-2,d0
        beq.b   0b                  /* timeout */
        cmpi.l  #-1,d0
        beq.b   1f                  /* no mouse */
        move.w  #0xF001,0xA15128    /* mouse in port 1 */
1:
        lea     2(a0),a0
2:
        jsr     get_mky
        cmpi.l  #-2,d0
        beq.b   2b                  /* timeout */
        cmpi.l  #-1,d0
        beq.b   3f                  /* no mouse */
        move.w  #0xF001,0xA1512A    /* mouse in port 2 */
3:

| allow the 68k to access the FM chip
        move.w  #0x0100,0xA11100    /* Z80 assert bus request */
        move.w  #0x0100,0xA11200    /* Z80 deassert reset */

| wait on Mars side
        move.b  #0,0xA15107             /* clear RV - allow SH2 to access ROM */
0:
        cmp.l   #0x4D5F4F4B,0xA15120    /* M_OK */
        bne.b   0b                      /* wait for master ok */
1:
        cmp.l   #0x535F4F4B,0xA15124    /* S_OK */
        bne.b   1b                      /* wait for slave ok */

        move.l  #vert_blank,vblank  /* set vertical blank interrupt handler */
        move.w  #0x2000,sr          /* enable interrupts */
        rts


| void write_byte(void *dst, unsigned char val)
        .global write_byte
write_byte:
        movea.l 4(sp),a0
        move.l  8(sp),d0
        move.b  d0,(a0)
        rts

| void write_word(void *dst, unsigned short val)
        .global write_word
write_word:
        movea.l 4(sp),a0
        move.l  8(sp),d0
        move.w  d0,(a0)
        rts

| void write_long(void *dst, unsigned int val)
        .global write_long
write_long:
        movea.l 4(sp),a0
        move.l  8(sp),d0
        move.l  d0,(a0)
        rts

| unsigned char read_byte(void *src)
        .global read_byte
read_byte:
        movea.l 4(sp),a0
        move.b  (a0),d0
        rts

| unsigned short read_word(void *src)
        .global read_word
read_word:
        movea.l 4(sp),a0
        move.w  (a0),d0
        rts

| unsigned int read_long(void *src)
        .global read_long
read_long:
        movea.l 4(sp),a0
        move.l  (a0),d0
        rts


        .data

| Put remaining code in data section to lower bus contention for the rom.

        .global do_main
do_main:
        move.b  #1,0xA15107         /* set RV */
        move.b  #2,0xA130F1         /* SRAM disabled, write protected */
        move.b  #0,0xA15107         /* clear RV */

        move.w  0xA15100,d0
        or.w    #0x8000,d0
        move.w  d0,0xA15100         /* set FM - allow SH2 access to MARS hw */
        move.l  #0,0xA15120         /* let Master SH2 run */

main_loop:
        move.w  0xA15120,d0         /* get COMM0 */
        bne.b   handle_req

        nop
        bra.b   main_loop

| process request from Master SH2
handle_req:
        cmpi.w  #0x01FF,d0
        bls     read_sram
        cmpi.w  #0x02FF,d0
        bls     write_sram
        cmpi.w  #0x03FF,d0
        bls     start_music
        cmpi.w  #0x04FF,d0
        bls     stop_music
        cmpi.w  #0x05FF,d0
        bls     read_mouse
        cmpi.w  #0x06FF,d0
        bls     handle_clrplanes
        cmpi.w  #0x07FF,d0
        bls     handle_planeBitmap
        cmpi.w  #0x08FF,d0
        bls     handle_scroll
| unknown command
        move.w  #0,0xA15120         /* done */
        bra.b   main_loop

read_sram:
        move.w  #0x2700,sr          /* disable ints */
        moveq   #0,d1
        moveq   #0,d0
        move.w  0xA15122,d0         /* COMM2 holds offset */
        lea     0x200000,a0
        move.b  #1,0xA15107         /* set RV */
        move.b  #3,0xA130F1         /* SRAM enabled, write protected */
        move.b  1(a0,d0.l),d1       /* read SRAM */
        move.b  #2,0xA130F1         /* SRAM disabled, write protected */
        move.b  #0,0xA15107         /* clear RV */
        move.w  d1,0xA15122         /* COMM2 holds return byte */
        move.w  #0,0xA15120         /* done */
        move.w  #0x2000,sr          /* enable ints */
        bra     main_loop

write_sram:
        move.w  #0x2700,sr          /* disable ints */
        moveq   #0,d1
        move.w  0xA15122,d1         /* COMM2 holds offset */
        lea     0x200000,a0
        move.b  #1,0xA15107         /* set RV */
        move.b  #1,0xA130F1         /* SRAM enabled, write enabled */
        move.b  d0,1(a0,d1.l)       /* write SRAM */
        move.b  #2,0xA130F1         /* SRAM disabled, write protected */
        move.b  #0,0xA15107         /* clear RV */
        move.w  #0,0xA15120         /* done */
        move.w  #0x2000,sr          /* enable ints */
        bra     main_loop

set_rom_bank:
        move.l  a0,d3
        swap    d3
        lsr.w   #4,d3
        andi.w  #3,d3
        move.w  d3,0xA15104         /* set ROM bank select */
        move.l  a0,d3
        andi.l  #0x0FFFFF,d3
        ori.l   #0x900000,d3
        movea.l d3,a1
        rts

start_music:
        tst.w   cd_ok
        beq.b   2f                  /* couldn't init cd */
        tst.b   cd_ok
        bne.b   0f                  /* disc found - try to play track */
        /* check for CD */
10:
        move.b  0xA1200F,d1
        bne.b   10b                 /* wait until Sub-CPU is ready to receive command */
        move.b  #'D,0xA1200E        /* set main comm port to GetDiskInfo command */
11:
        move.b  0xA1200F,d0
        beq.b   11b                 /* wait for acknowledge byte in sub comm port */
        move.b  #0x00,0xA1200E      /* acknowledge receipt of command result */

        cmpi.b  #'D,d0
        bne.b   2f                  /* couldn't get disk info */
        move.w  0xA12020,d0         /* BIOS status */
        cmpi.w  #0x1000,d0
        bhs.b   2f                  /* open, busy, or no disc */
        move.b  #1,cd_ok            /* we have a disc - try to play track */
0:
        move.b  0xA1200F,d1
        bne.b   0b                  /* wait until Sub-CPU is ready to receive command */

        move.b  d0,0xA12012         /* repeat flag */
        move.w  0xA15122,d0
        addq.w  #1,d0
        move.w  d0,0xA12010         /* track no. */
        move.b  #'P,0xA1200E        /* set main comm port to PlayTrack command */
1:
        move.b  0xA1200F,d0
        beq.b   1b                  /* wait for acknowledge byte in sub comm port */
        move.b  #0x00,0xA1200E      /* acknowledge receipt of command result */
2:
        move.w  #0,0xA15120         /* done */
        bra     main_loop

stop_music:
        tst.w   cd_ok
        beq.b   2f
0:
        move.b  0xA1200F,d1
        bne.b   0b                  /* wait until Sub-CPU is ready to receive command */

        move.b  #'S,0xA1200E        /* set main comm port to StopPlayback command */
1:
        move.b  0xA1200F,d0
        beq.b   1b                  /* wait for acknowledge byte in sub comm port */
        move.b  #0x00,0xA1200E      /* acknowledge receipt of command result */
2:
        move.w  #0,0xA15120         /* done */
        bra     main_loop

read_mouse:
        tst.b   d0
        bne.b   1f                  /* skip port 1 */

        move.w  0xA15128,d0
        andi.w  #0xF001,d0
        cmpi.w  #0xF001,d0
        bne.b   1f                  /* no mouse in port 1 */
        lea     0xA10003,a0
        bsr     get_mky
        bset    #31,d0
        move.w  d0,0xA15122
        swap    d0
        move.w  d0,0xA15120
0:
        move.w  0xA15120,d0
        bne.b   0b                  /* wait for SH2 to read mouse value */
        bra     main_loop
1:
        move.w  0xA1512A,d0
        andi.w  #0xF001,d0
        cmpi.w  #0xF001,d0
        bne.b   3f                  /* no mouse in port 2 */
        lea     0xA10005,a0
        bsr     get_mky
        bset    #31,d0
        move.w  d0,0xA15122
        swap    d0
        move.w  d0,0xA15120
2:
        move.w  0xA15120,d0
        bne.b   2b                  /* wait for SH2 to read mouse value */
        bra     main_loop
3:
        move.l  #-1,d0              /* no mouse */
        move.w  d0,0xA15122
        swap    d0
        move.w  d0,0xA15120
4:
        move.w  0xA15120,d0
        bne.b   4b                  /* wait for SH2 to read mouse value */
        bra     main_loop

handle_planeBitmap:
        andi.l  #0x0001,d0
        move.l  0xA1512C,d1
        andi.l  #0x0FFFFF,d1
        move.l  d1,a0
        bsr     set_rom_bank
        move.l  a1,-(sp)
        move.l  d0,-(sp)
        jsr     set_planeBitmap
        lea     4(sp),sp
        movea.l (sp)+,a1
        move.w  #0,0xA15120         /* done */
        bra     main_loop

handle_clrplanes:
        jsr     clear_planes
        move.w  #0,0xA15120         /* done */
        bra     main_loop

handle_scroll:
        moveq   #0,d1
        andi.l  #0x0003,d0
        move.w  0xA15122,d1         /* COMM2 holds scroll amount */
        btst    #1,d0
        andi.l  #0x0001,d0

        move.l  d1,-(sp)
        move.l  d0,-(sp)

        bne     2f
1:
        jsr     hscroll_plane
        lea     8(sp),sp
        move.w  #0,0xA15120         /* done */
        bra     main_loop
2:
        jsr     vscroll_plane
        lea     8(sp),sp
        move.w  #0,0xA15120         /* done */
        bra     main_loop

vert_blank:
        move.l  d1,-(sp)
        move.l  d2,-(sp)

        /* read controllers */
        move.w  0xA15128,d0
        andi.w  #0xF000,d0
        cmpi.w  #0xF000,d0
        beq.b   0f                  /* no pad in port 1 (or mouse) */
        lea     0xA10003,a0
        bsr.b   get_pad
        move.w  d2,0xA15128         /* controller 1 current value */
0:
        move.w  0xA1512A,d0
        andi.w  #0xF000,d0
        cmpi.w  #0xF000,d0
        beq.b   1f                  /* no pad in port 2 (or mouse) */
        lea     0xA10005,a0
        bsr.b   get_pad
        move.w  d2,0xA1512A         /* controller 2 current value */
1:

        tst.w   gen_lvl2
        beq.b   2f
        lea     0xA12000,a0
        move.w  (a0),d0
        ori.w   #0x0100,d0
        move.w  d0,(a0)
2:
        move.l  (sp)+,d2
        move.l  (sp)+,d1
        movea.l (sp)+,a0
        move.l  (sp)+,d0
        rte

| get current pad value
| entry: a0 = pad control port
| exit:  d2 = pad value (0 0 0 1 M X Y Z S A C B R L D U) or (0 0 0 0 0 0 0 0 S A C B R L D U)
get_pad:
        bsr.b   get_input       /* - 0 s a 0 0 d u - 1 c b r l d u */
        move.w  d0,d1
        andi.w  #0x0C00,d0
        bne.b   no_pad
        bsr.b   get_input       /* - 0 s a 0 0 d u - 1 c b r l d u */
        bsr.b   get_input       /* - 0 s a 0 0 0 0 - 1 c b m x y z */
        move.w  d0,d2
        bsr.b   get_input       /* - 0 s a 1 1 1 1 - 1 c b r l d u */
        andi.w  #0x0F00,d0      /* 0 0 0 0 1 1 1 1 0 0 0 0 0 0 0 0 */
        cmpi.w  #0x0F00,d0
        beq.b   common          /* six button pad */
        move.w  #0x010F,d2      /* three button pad */
common:
        lsl.b   #4,d2           /* - 0 s a 0 0 0 0 m x y z 0 0 0 0 */
        lsl.w   #4,d2           /* 0 0 0 0 m x y z 0 0 0 0 0 0 0 0 */
        andi.w  #0x303F,d1      /* 0 0 s a 0 0 0 0 0 0 c b r l d u */
        move.b  d1,d2           /* 0 0 0 0 m x y z 0 0 c b r l d u */
        lsr.w   #6,d1           /* 0 0 0 0 0 0 0 0 s a 0 0 0 0 0 0 */
        or.w    d1,d2           /* 0 0 0 0 m x y z s a c b r l d u */
        eori.w  #0x1FFF,d2      /* 0 0 0 1 M X Y Z S A C B R L D U */
        rts

no_pad:
        move.w  #0xF000,d2
        rts

| read single phase from controller
get_input:
        move.b  #0x00,(a0)
        nop
        nop
        move.b  (a0),d0
        move.b  #0x40,(a0)
        lsl.w   #8,d0
        move.b  (a0),d0
        rts

| get current mouse value
| entry: a0 = mouse control port
| exit:  d0 = mouse value (0  0  0  0  0  0  0  0  YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0) or -2 (timeout) or -1 (no mouse)
get_mky:
        move.w  sr,d2
        move.w  #0x2700,sr      /* disable ints */

        move.b	#0x60,6(a0)     /* set direction bits */
        nop
        nop
        move.b  #0x60,(a0)      /* first phase of mouse packet */
        nop
        nop
0:
        btst    #4,(a0)
        beq.b   0b              /* wait on handshake */
        move.b  (a0),d0
        andi.b  #15,d0
        bne     mky_err         /* not 0 means not mouse */

        move.b  #0x20,(a0)      /* next phase */
        move.w  #254,d1         /* number retries before timeout */
1:
        btst    #4,(a0)
        bne.b   2f              /* handshake */
        dbra    d1,1b
        bra     timeout_err
2:
        move.b  (a0),d0
        andi.b  #15,d0
        move.b  #0,(a0)         /* next phase */
        cmpi.b  #11,d0
        bne     mky_err         /* not 11 means not mouse */
3:
        btst    #4,(a0)
        beq.b   4f              /* handshake */
        dbra    d1,3b
        bra     timeout_err
4:
        move.b  (a0),d0         /* specs say should be 15 */
        nop
        nop
        move.b  #0x20,(a0)      /* next phase */
        nop
        nop
5:
        btst    #4,(a0)
        bne.b   6f
        dbra    d1,5b
        bra     timeout_err
6:
        move.b  (a0),d0         /* specs say should be 15 */
        nop
        nop
        move.b  #0,(a0)         /* next phase */
        moveq   #0,d0           /* clear reg to hold packet */
        nop
7:
        btst    #4,(a0)
        beq.b   8f              /* handshake */
        dbra    d1,7b
        bra     timeout_err
8:
        move.b  (a0),d0         /* YO XO YS XS */
        move.b  #0x20,(a0)      /* next phase */
        lsl.w   #8,d0           /* save nibble */
9:
        btst    #4,(a0)
        bne.b   10f             /* handshake */
        dbra    d1,9b
        bra     timeout_err
10:
        move.b  (a0),d0         /* S  M  R  L */
        move.b  #0,(a0)         /* next phase */
        lsl.b   #4,d0           /* YO XO YS XS S  M  R  L  0  0  0  0 */
        lsl.l   #4,d0           /* YO XO YS XS S  M  R  L  0  0  0  0  0  0  0  0 */
11:
        btst    #4,(a0)
        beq.b   12f             /* handshake */
        dbra    d1,11b
        bra     timeout_err
12:
        move.b  (a0),d0         /* X7 X6 X5 X4 */
        move.b  #0x20,(a0)      /* next phase */
        lsl.b   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 0  0  0  0 */
        lsl.l   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 0  0  0  0  0  0  0  0 */
13:
        btst    #4,(a0)
        bne.b   14f             /* handshake */
        dbra    d1,13b
        bra     timeout_err
14:
        move.b  (a0),d0         /* X3 X2 X1 X0 */
        move.b  #0,(a0)         /* next phase */
        lsl.b   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 0  0  0  0 */
        lsl.l   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 0  0  0  0  0  0  0  0 */
15:
        btst    #4,(a0)
        beq.b   16f             /* handshake */
        dbra    d1,15b
        bra     timeout_err
16:
        move.b  (a0),d0         /* Y7 Y6 Y5 Y4 */
        move.b  #0x20,(a0)      /* next phase */
        lsl.b   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 Y7 Y6 Y5 Y4 0  0  0  0 */
        lsl.l   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 Y7 Y6 Y5 Y4 0  0  0  0  0  0  0  0*/
17:
        btst    #4,(a0)
        beq.b   18f             /* handshake */
        dbra    d1,17b
        bra     timeout_err
18:
        move.b  (a0),d0         /* Y3 Y2 Y1 Y0 */
        move.b  #0x60,(a0)      /* first phase */
        lsl.b   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 0  0  0  0 */
        lsr.l   #4,d0           /* YO XO YS XS S  M  R  L  X7 X6 X5 X4 X3 X2 X1 X0 Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */
19:
        btst    #4,(a0)
        beq.b   19b             /* wait on handshake */

        move.w  d2,sr           /* restore int status */
        rts

timeout_err:
        move.b  #0x60,(a0)      /* first phase */
        nop
        nop
0:
        btst    #4,(a0)
        beq.b   0b              /* wait on handshake */

        move.w  d2,sr           /* restore int status */
        moveq   #-2,d0
        rts

mky_err:
        move.b  #0x40,6(a0)     /* set direction bits */
        nop
        nop
        move.b  #0x40,(a0)

        move.w  d2,sr           /* restore int status */
        moveq   #-1,d0
        rts

| void clear_plane(int plane);
| clear the name table for plane A or B
        .global clear_plane
clear_plane:
        moveq   #0,d0
        move.l  #0x40000003,d1          /* VDP write VRAM at 0xC000 (scroll plane A) */
        movea.l 4(sp),a0                /* plane */
        mov.l   a0,d0
        cmpi.l  #1,d0
        bne.b   0f
        move.l  #0x60000003,d1          /* VDP write VRAM at 0xE000 (scroll plane B) */
0:
        moveq   #0,d0
        lea     0xC00000,a0
        move.w  #0x8F02,4(a0)           /* set INC to 2 */
        move.l  d1,4(a0)                /* write VRAM at plane start */
        move.w  #128*32-1,d1
1:
        move.w  d0,(a0)                 /* clear name pattern */
        dbra    d1,1b

        rts

| void hscroll_plane(int plane, int hscroll);
| set horizontall scroll for plane A or B
        .global hscroll_plane
hscroll_plane:
        move.l  4(sp),d0                /* plane number */

        move.l  #0x6C000002,d1          /* plane A HSCROLL */
        cmpi.l  #1,d0
        bne.b   0f
        move.l  #0x6C020002,d1          /* plane B HSCROLL */
0:
        move.l  8(sp),d0                /* hscroll */
        lea     0xC00000,a0
        move.w  #0x8F01,4(a0)           /* set INC to 1 */
        move.l  d1,4(a0)                /* write VRAM at hscroll table start */

        move.w	d0,(a0)	                /* write hscroll */
        rts

| void vscroll_plane(int plane, int hscroll);
| set vertical scroll for plane A or B
        .global vscroll_plane
vscroll_plane:
        move.l  4(sp),d0                /* plane number */

        move.l  #0x40000010,d1          /* plane A VSCROLL */
        cmpi.l  #1,d0
        bne.b   0f
        move.l  #0x40020010,d1          /* plane B VSCROLL */
0:
        move.l  8(sp),d0                /* vscroll amount */
        lea     0xC00000,a0
        move.l  d1,4(a0)                /* write to VSRAM */

        move.w  d0,(a0)                 /* write vscroll */
        rts

| void map_plane(int plane, int offset, int height, int ormask);
| set the name table and hscroll for plane B for bitmap mode
        .global map_plane
map_plane:
        moveq   #0,d0
        move.l  #0x40000003,d1          /* VDP write VRAM at 0xC000 (scroll plane A) */
        movea.l 4(sp),a0                /* plane */
        mov.l   a0,d0
        cmpi.l  #1,d0
        bne.b   0f
        move.l  #0x60000003,d1          /* VDP write VRAM at 0xE000 (scroll plane B) */
0:
        moveq   #0,d0
        movea.l 8(sp),a0
        move.w  a0,d0                   /* bitmap start */
        movea.l 12(sp),a0

        movem.l d2-d4,-(sp)
        moveq   #0,d3
        move.w  a0,d3                   /* bitmap height */

        moveq   #0,d4
        movea.l 28(sp),a0
        move.w  a0,d4                   /* OR mask for palette */

        lea     0xC00000,a0
        move.w  #0x8F02,4(a0)           /* set INC to 2 */
        move.l  d1,4(a0)                /* write VRAM at plane B start */
  
1:
        or      d4,d0                   /* palette */
        move.w  #39,d2
2:
        move.w  d0,(a0)                 /* name table entry set for next pattern */
        addq.w  #1,d0
        dbra    d2,2b

        subi.w	#40,d0
        or      d4,d0                   /* palette */
        move.w  #39,d2
3:
        move.w  d0,(a0)                 /* name table entry set for next pattern */
        addq.w  #1,d0
        dbra    d2,3b

        moveq	#23,d2
4:
        move.l  #0,(a0)                 /* clear entries to end of row */
        dbra	d2,4b
        dbra    d3,1b

        move.l  #0x6C000002,d1          /* VDP write VRAM at 0xAC00 (HSCROLL table) */
        move.l  d1,4(a0)                /* write VRAM at hscroll table start */
        move.w	#111,d1
5:
        move.l	#0x00000000,(a0)        /* scroll A = 0, scroll B = 0 */
        dbra	d1,5b

        move.w  #0x8B00,4(a0)           /* SCROLL the whole plane */
        move.w  #0x9003,4(a0)           /* scroll size 128x32 */

        movem.l (sp)+,d2-d4
        rts

| void set_vram(int offset, int val);
| store word to vram at offset
| entry: first arg = offset in vram
|        second arg = word to store
        .global set_vram
set_vram:
        lea     0xC00000,a1
        move.w  #0x8F02,4(a1)           /* set INC to 2 */
        move.l  4(sp),d1                /* vram offset */
        lsl.l   #2,d1
        lsr.w   #2,d1
        swap    d1
        ori.l   #0x40000000,d1          /* VDP write VRAM */
        move.l  d1,4(a1)                /* write VRAM at offset*/
        move.l  8(sp),d0                /* data word */
        move.w  d0,(a1)                 /* set vram word */
        rts

| void next_vram(int val);
| store word to vram at next offset
| entry: first arg = word to store
        .global next_vram
next_vram:
        move.l  4(sp),d0                /* data word */
        move.w  d0,0xC00000             /* set vram word */
        rts

| void set_palette(short *pal, int start, int count)
| copy count entries pointed to by pal into the palette starting at the index start
| entry: pal = pointer to an array of words holding the colors
|        start = index of the first color in the palette to set
|        count = number of colors to copy
        .global set_palette
set_palette:
        movea.l 4(sp),a0                /* pal */
        move.l  8(sp),d0                /* start */
        move.l  12(sp),d1               /* count */
        add.w   d0,d0                   /* start*2 */
        swap    d0                      /* high word holds address */
        ori.l   #0xC0000000,d0          /* write CRAM address (0 + index*2) */
        subq.w  #1,d1                   /* for dbra */

        lea     0xC00000,a1
        move.w  #0x8F02,4(a1)           /* set INC to 2 */
        move.l  d0,4(a1)                /* write CRAM */
0:
        move.w  (a0)+,(a1)              /* copy color to palette */
        dbra    d1,0b
        rts

| Global variables for 68000

        .align  4

vblank:
        dc.l    0

        .global gen_lvl2
gen_lvl2:
        dc.w    0

        .global cd_ok
cd_ok:
        dc.w    0

        .align  4
