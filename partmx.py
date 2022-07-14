import xml.etree.ElementTree as ET
import csv
import sys

fn = "Platform.tmx"
outpref = ""

if len(sys.argv) > 1:
    fn = sys.argv[1]
if len(sys.argv) > 2:
    outpref = sys.argv[2] + "_"

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

    print("const uint16_t %slayer%02d[] = {" % (outpref, numlayers))
    for rr in r:
        if len(rr) == 0:
            continue
        for x in rr:
            if len(x) == 0:
                break

            x = int(x)
            flip = (x >> 30) & 3

            newflip = 0
            if flip & 3 == 1:
                newflip = 3
            elif flip & 3 == 2:
                newflip = 1
            elif flip & 3 == 3:
                newflip = 2

            x &= (1<<14)-1
            x <<= 2
            x |= newflip

            print(x, end=",")

        print("")
    print("};")

    if numlayers == 0:
        numtw = int(l.attrib["width"])
        numth = int(l.attrib["height"])
    numlayers = numlayers + 1
    #break

print("const uint16_t* %stmxl[] = {" % outpref);
for l in range(numlayers):
    print("%slayer%02d," % (outpref, l))
print("};")

print("const int %stmxlplx[][2] = {" % outpref);
for l in root.findall('layer'):
    plx = 1.0
    ply = 1.0
    if "parallaxx" in l.attrib:
        plx = float(l.attrib["parallaxx"])
    if "parallaxy" in l.attrib:
        ply = float(l.attrib["parallaxy"])
    print("{%d,%d}," % (int(plx*65536), int(ply*65536)))
    #break
print("};")

wrapX = wrapY = 0
pa = root.find("properties")
if pa:
    for p in pa.findall("property"):
        if "name" not in p.attrib:
            continue

        name = p.attrib["name"]
        if name == "wrap-x":
            wrapX = int(p.attrib["value"])
        if name == "wrap-y":
            wrapY = int(p.attrib["value"])

print("const dtilemap_t %stmx = {%d,%d,%d,%d,%d,%d,%d,(int *)&%stmxlplx[0][0],(uint16_t **)%stmxl};" % (outpref, tilew, tileh, numtw, numth, numlayers, wrapX, wrapY, outpref, outpref))

#ET.dump(root)

