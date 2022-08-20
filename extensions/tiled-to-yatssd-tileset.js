/*
 * tiled-to-yatssd-export.js
 *
 * This extension adds the "YATSSD resource and palette files - regular" type to the "Export As" menu
 *
 * Copyright (c) 2020 Jay van Hutten
 * Copyright (c) 2022 Victor Luchits
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

function dec(v) {
    return v.toString(10);
}

function hex(v) {
   return "0x" + v.toString(16);
}

function hex2(v) {
    return "0x" + v.toString(16).padStart(2, "0");
}

var customTilesFormat = {
    name: "YATSSD tileset and palette files",
    extension: "h",
    write:

    function(tileset, filename) {
        console.time("Export completed in");

        // Split full filename path into the filename (without extension) and the directory
        let fileBaseName = FileInfo.completeBaseName(filename).replace(/[^a-zA-Z0-9-_]/g, "_");
        let filePath = FileInfo.path(filename);

        let resourceName = fileBaseName;
        let c = fileBaseName.slice(0, 1)
        if (c >= '0' && c <= '9') {
            resourceName = "_" + resourceName;
        }

        let imagePath = tileset.image;
        let image = new Image(imagePath);
        let colorTable = image.colorTable();
        let backgroundColor = tileset.backgroundColor;

        let paletteFileData = "";
        paletteFileData += "#ifndef "+resourceName.toUpperCase()+"_PALETTE_H\n";
        paletteFileData += "#define "+resourceName.toUpperCase()+"_PALETTE_H\n\n";
        paletteFileData += "#include <stdint.h>\n";
        paletteFileData += "\n";
        paletteFileData += "const uint8_t "+resourceName+"_Palette[] __attribute__((aligned(16))) = {\n";

        let lastind = 0;
        for (let [ind, value] of Object.entries(colorTable)) {
            let rgb = [];

            if (ind == 0 && backgroundColor)
            {
                value = parseInt(backgroundColor.toString().slice(1),16);
            }

            rgb.push(hex2((value >> 16) & 0xff));
            rgb.push(hex2((value >> 8 ) & 0xff));
            rgb.push(hex2((value >> 0 ) & 0xff));
            paletteFileData += rgb.join(",")+",\n";
            lastind = ind;
        }
        for (; lastind < 256; lastind++)
        {
            paletteFileData += "0x0,0x0,0x0,\n";
        }

        if (paletteFileData.slice(-2) == ",\n")
            paletteFileData = paletteFileData.slice(0,-2) + "\n";
        paletteFileData += "};\n";
        paletteFileData += "\n#endif\n";

        let revColorTable = {};
        for (const [ind, value] of Object.entries(colorTable)) {
            revColorTable[value] = ind | 0
        }

        let resourceFileData = "";
        resourceFileData += "#ifndef "+resourceName.toUpperCase()+"_H\n";
        resourceFileData += "#define "+resourceName.toUpperCase()+"_H\n\n";
        resourceFileData += "#include <stdint.h>\n";
        resourceFileData += "\n";

        let tilesetData = "";

        let x = 0
        let y = 0
        let id = 0
        let res = []

        // empty dummy tile
        let name = resourceName+"_Res"+dec(id)
        res.push(name)

        tilesetData += "uint8_t "+name+"[] __attribute__((aligned(16))) = {\n"
        for (let i = 0; i < tileset.tileHeight; i++)
        {
            for (j = 0; j < tileset.tileWidth; j++) {
                tilesetData += hex2(0)+","
            }
            tilesetData += "\n"
        }
        if (tilesetData.slice(-2) == ",\n")
            tilesetData = tilesetData.slice(0,-2) + "\n";
        tilesetData += "};\n\n"

        id = 1

        for (let y = 0; y < image.height; y += tileset.tileHeight)
        {
            for (let x = 0; x < image.width; x += tileset.tileWidth)
            {
                let name = resourceName+"_Res"+dec(id)
                res.push(name)

                tilesetData += "uint8_t "+name+"[] __attribute__((aligned(16))) = {\n"

                for (let i = 0; i < tileset.tileHeight; i++)
                {
                    for (let j = 0; j < tileset.tileWidth; j++)
                    {
                        let p = image.pixel(x+j, y+i)
                        tilesetData += hex2(revColorTable[p])+","
                    }
                    tilesetData += "\n"
                }

                if (tilesetData.slice(-2) == ",\n")
                    tilesetData = tilesetData.slice(0,-2) + "\n";
                tilesetData += "};\n\n"
                id += 1
            }
        }

        resourceFileData += tilesetData + "\n";

        resourceFileData += "const uint8_t * "+resourceName+"_Reslist[] = {\n";
        resourceFileData += res.join(",\n");
        resourceFileData += "\n};\n";

        resourceFileData += "\n#endif\n";

        // Write header data to disk
        let headerFile = new TextFile(FileInfo.joinPaths(filePath,fileBaseName+".h"), TextFile.WriteOnly);
        headerFile.write(resourceFileData);
        headerFile.commit();

        let paletteFile = new TextFile(FileInfo.joinPaths(filePath,fileBaseName+"_palette.h"), TextFile.WriteOnly);
        paletteFile.write(paletteFileData);
        paletteFile.commit();

        console.log("Tileset exported to "+FileInfo.joinPaths(filePath,fileBaseName+".h"));

        console.timeEnd("Export completed in");
    }
}

tiled.registerTilesetFormat("yatssd", customTilesFormat)
