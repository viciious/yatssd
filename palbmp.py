from PIL import Image
import sys

fn = "pla_VGA.bmp"
if len(sys.argv) > 1:
    fn = sys.argv[1]

bmp = Image.open(fn)
pal = bmp.getpalette()

print("#include <stdint.h>")
print("const uint8_t palette[] __attribute__((aligned(16))) = {")
#print("uint8_t palette[] __attribute__((aligned(16))) = {")
for i in range(0, 768, 3):
    print("0x%02x,0x%02x,0x%02x," % (pal[i+0], pal[i+1], pal[i+2]))
print("};")
print("")

