from PIL import Image
import sys

fn = "pla_VGA.bmp"
outpref = ""
if len(sys.argv) > 1:
    fn = sys.argv[1]
if len(sys.argv) > 2:
    outpref = sys.argv[2] + "_"

bmp = Image.open(fn)
pal = bmp.getpalette()

print("#include <stdint.h>")
print("const uint8_t %spalette[] __attribute__((aligned(16))) = {" % outpref)
#print("uint8_t palette[] __attribute__((aligned(16))) = {")
for i in range(0, 768, 3):
    print("0x%02x,0x%02x,0x%02x," % (pal[i+0], pal[i+1], pal[i+2]))
print("};")
print("")

