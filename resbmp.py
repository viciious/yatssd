from PIL import Image
import sys

fn = "pla_VGA.bmp"
if len(sys.argv) > 1:
    fn = sys.argv[1]

bmp = Image.open(fn)
pal = bmp.getpalette()

#print(pal)
#print(bmp.size[0], bmp.size[1])

#print("const uint8_t palette[] __attribute__((aligned(16))) = {")

#print("uint8_t palette[] __attribute__((aligned(16))) = {")
#for i in range(0, 768, 3):
#    print("0x%02x,0x%02x,0x%02x," % (pal[i+0], pal[i+1], pal[i+2]))
#print("};")
#print("")

print("#include <stdint.h>")

resid = 0
for y in range(0, bmp.size[1], 16):
    for x in range (0, bmp.size[0], 16):
        #print("const uint8_t res%02d[] __attribute__((aligned(16))) = {" % resid)
        print("uint8_t res%02d[] __attribute__((aligned(16))) = {" % resid)
        for i in range(0, 16):
            for j in range (0, 16):
                print("0x%02x," % bmp.getpixel((x+j,y+i)), end='')
            print("")
        print("};")
        print("")
        resid = resid + 1


#print("const uint8_t * reslist[] = {");
print("uint8_t * reslist[] = {");
for i in range(resid):
    print("res%02d," % i)
print("};")

