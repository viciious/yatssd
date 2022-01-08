import xml.etree.ElementTree as ET
import csv

tree = ET.parse('Platform.tmx')
root = tree.getroot()

numlayers = 0
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

    numlayers = numlayers + 1
    break

print("const uint16_t* tmx[] = {");
for l in range(numlayers):
    print("layer%02d," % l)
print("};")

#ET.dump(root)

