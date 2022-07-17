#ifndef HW_32X_H
#define HW_32X_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HW32X_ATTR_DATA_ALIGNED __attribute__((section(".data"), aligned(16)))

extern void Hw32xSetFGColor(int s, int r, int g, int b);
extern void Hw32xSetBGColor(int s, int r, int g, int b);
void Hw32xSetFGOverlayPriorityBit(int p);
void Hw32xSetBGOverlayPriorityBit(int p);
extern void Hw32xInit(int vmode, int lineskip);
extern int Hw32xScreenGetX();
extern int Hw32xScreenGetY();
extern void Hw32xScreenSetXY(int x, int y);
extern void Hw32xScreenClear();
extern void Hw32xScreenPutChar(int x, int y, unsigned char ch);
extern void Hw32xScreenClearLine(int Y);
extern int Hw32xScreenPrintData(const char *buff, int size);
extern int Hw32xScreenPutsn(const char *str, int len);
extern void Hw32xScreenPrintf(const char *format, ...);
extern void Hw32xDelay(int ticks);
extern void Hw32xScreenFlip(int wait);
extern void Hw32xFlipWait(void);
extern void Hw32xSetPalette(const uint8_t *palette) HW32X_ATTR_DATA_ALIGNED;

extern unsigned short HwMdReadPad(int port);
extern unsigned char HwMdReadSram(unsigned short offset);
extern void HwMdWriteSram(unsigned char byte, unsigned short offset);
extern int HwMdReadMouse(int port);
extern void HwMdClearScreen(void);
extern void HwMdSetOffset(unsigned short offset);
extern void HwMdSetNTable(unsigned short word);
extern void HwMdSetVram(unsigned short word);
extern void HwMdPuts(char *str, int color, int x, int y);
extern void HwMdPutc(char chr, int color, int x, int y);
extern void HwMdPutsf(int x, int y, int color, const char* format, ...);

void Hw32xUpdateLineTable(int hscroll, int vscroll, int lineskip) HW32X_ATTR_DATA_ALIGNED;

void HwMdSetPlaneABitmap(void* data);
void HwMdSetPlaneBBitmap(void* data);
void HwMdClearPlanes(void);

void pri_vbi_handler(void) HW32X_ATTR_DATA_ALIGNED;
void pri_dma1_handler(void) HW32X_ATTR_DATA_ALIGNED;

unsigned Hw32xGetTicks(void) HW32X_ATTR_DATA_ALIGNED;

#ifdef __cplusplus
}
#endif

#endif
