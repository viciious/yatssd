import xml.etree.ElementTree as ET
import csv
import sys

fn = "Platform.tmx"
if len(sys.argv) > 1:
    fn = sys.argv[1]

tree = ET.parse(fn)
root = tree.getroot()

numlayers = 0
numtw = 0
numth = 0
tilew = 0
tileh = 0

m = root
if "tilewidth" in m.attrib:
    tilew = int(m.attrib["tilewidth"])
    tileh = int(m.attrib["tileheight"])

if tilew == 0:
    for s in root.findall('tileset'):
        tilew = int(s.attrib["tilewidth"])
        tileh = int(s.attrib["tileheight"])
        break

print("#include <stdint.h>")

for l in root.findall('layer'):
    d = l.find('data')
    r = csv.reader(d.text.split('\n'), delimiter=',', skipinitialspace=True)

    print("const uint16_t layer%02d[] = {" % numlayers)
    for rr in r:
        if len(rr) == 0:
            continue
        for x in rr:
            if len(x) == 0:
                break
            print(int(x), end=",")
        print("")
    print("};")

    if numlayers == 0:
        numtw = int(l.attrib["width"])
        numth = int(l.attrib["height"])
    numlayers = numlayers + 1
    #break

print("const uint16_t* tmxl[] = {");
for l in range(numlayers):
    print("layer%02d," % l)
print("};")

print("const int16_t tmxlplx[][2] = {");
for l in root.findall('layer'):
    plx = 1.0
    ply = 1.0
    if "parallaxx" in l.attrib:
        plx = float(l.attrib["parallaxx"])
    if "parallaxy" in l.attrib:
        ply = float(l.attrib["parallaxy"])
    print("{%d,%d}," % (int(plx*32767), int(ply*32767)))
print("};")

print("typedef struct {");
print("int tilew, tileh;");
print("int numtw, numth;");
print("int numlayers;");
print("int16_t **layerplx;");
print("uint16_t **layers;");
print("} dtilemap_t;");

print("const dtilemap_t tmx = {%d,%d,%d,%d,%d,(int16_t **)%s,(uint16_t **)%s};" % (tilew, tileh, numtw, numth, numlayers, "tmxlplx", "tmxl"))

#ET.dump(root)

