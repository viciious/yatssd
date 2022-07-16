/*
 * tiled-to-yatssd-export.js
 *
 * This extension adds the "YATSSD header files - regular" type to the "Export As" menu
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

var customMapFormat = {
    name: "YATSSD tilemap header files",
    extension: "h",
    write:

    function(map, fileName) {
        console.time("Export completed in");

        if (map.tileWidth % 8 != 0 || map.tileHeight %8 != 0) {
            return "Export failed! Tile width and height must be a multiple of 8.";
        }

        // Split full filename path into the filename (without extension) and the directory
        let fileBaseName = FileInfo.completeBaseName(fileName).replace(/[^a-zA-Z0-9-_]/g, "_");
        let filePath = FileInfo.path(fileName);

        let resourceName = fileBaseName;
        let c = fileBaseName.slice(0, 1)
        if (c >= '0' && c <= '9') {
            resourceName = "_" + resourceName;
        }

        let headerFileData = "";
        headerFileData += "#ifndef "+resourceName.toUpperCase()+"_H\n";
        headerFileData += "#define "+resourceName.toUpperCase()+"_H\n\n";
        headerFileData += "#include <stdint.h>\n";
        headerFileData += "\n";

        let tileData = "";
        let layerData = "const uint16_t *" + resourceName + "_Layers[] = {";
        let parallaxData = "const int " + resourceName + "_Parallax[][2] = {";

        let numLayers = 0
        for (let i = 0; i < map.layerCount; ++i) {
            let layer = map.layerAt(i);

            if (!layer.isTileLayer) {
                continue;
            }
            if (!layer.visible) {
                continue;
            }

            numLayers++

            // Replace special characters for an underscore
            let layerName = layer.name.replace(/[^a-zA-Z0-9-_]/g, "_");
            layerName = resourceName+"_Tiles_"+layerName;

            tileData += "const uint16_t "+layerName+"[] = {\n";

            for (y = 0; y < layer.height; ++y) {
                for (x = 0; x < layer.width; ++x) {
                    let tile = layer.cellAt(x, y);
                    let id = tile.tileId+1;
                    let flip = 0

                    if (tile.flippedHorizontally && tile.flippedVertically)
                        flip = 2
                    else if (tile.flippedHorizontally)
                        flip = 1
                    else if (tile.flippedVertically)
                        flip = 3
                    id = ((id & 0x3FFF) << 2) | flip

                    tileData += hex(id)+",";
                }

                tileData += "\n";
            }

            if (tileData.slice(-2) == ",\n")
                tileData = tileData.slice(0,-2) + "\n";
            tileData += "};\n";

            layerData += layerName+",";
            parallaxData += "{" + dec((layer.parallaxFactor.x*65536) | 0) + "," + dec((layer.parallaxFactor.y*65536) | 0) + "},";
        }

        if (layerData.slice(-1) == ",")
            layerData = layerData.slice(0,-1);
        layerData += "};\n";

        if (parallaxData.slice(-1) == ",")
            parallaxData = parallaxData.slice(0,-1);
        parallaxData += "};\n";

        let mapData = "const dtilemap_t " + resourceName + "_Map = {";
        mapData += dec(map.tileWidth)+","+dec(map.tileHeight) +",";
        mapData += dec(map.width)+","+dec(map.height)+",";
        mapData += dec(numLayers)+",";
        mapData += dec(map.property("wrap-x") || 0)+"," + dec(map.property("wrap-y") || 0)+",";
        mapData += "(int *)"+resourceName + "_Parallax,";
        mapData += "(uint16_t **)"+resourceName + "_Layers";
        mapData += "};\n";

        headerFileData += tileData + "\n";
        headerFileData += layerData + "\n";
        headerFileData += parallaxData + "\n";
        headerFileData += mapData + "\n";

        headerFileData += "#endif\n";

        // Write header data to disk
        let headerFile = new TextFile(FileInfo.joinPaths(filePath,fileBaseName+".h"), TextFile.WriteOnly);
        headerFile.write(headerFileData);
        headerFile.commit();

        console.log("Tilemap exported to "+FileInfo.joinPaths(filePath,fileBaseName+".h"));

        console.timeEnd("Export completed in");
    }
}

tiled.registerMapFormat("yatssd", customMapFormat)
