# Yet Another Tilemap and Super Scaler Demo
Yet Another Tilemap and Super Scaler Demo for the Sega 32X

![image](https://user-images.githubusercontent.com/1173058/148651754-11cb3fbd-5d09-4de7-bf23-469dc957359c.png)

![image](https://user-images.githubusercontent.com/1173058/185707152-72ef304a-cc94-4ff7-a975-050904bfef40.png)

Features
============
- works on real hardware
- draws tilemap of arbitrary size, exported from Tiled
- scrolls in any direction, avoiding redraw as much as possible using the dirty rectangles approach
- uses the hardware screen shift-register for smooth scrolling
- can handle an arbitrary number of clipped sprites
- adjustable clipping region for sprites: see the draw_setScissor call
- uses both CPUs to draw tiles and sprites
- sprites can be flipped on X and Y axis and/or scaled
- "imprecise" rendering: sprites can be scaled using a cheaper and faster algorithm, the X coordinate is also snapped to the nearest even value
- the code also supports flipping of tiles, although the tilemap format would need to be extended for that
- it can handle an arbitrary number of tile layers along with parallax
- the sprites and tiles can be of any size in either dimension, although if you to plan to use scaling, the size should be a power of 2
- sprites can be scaled to an arbitrary size
- support for dedicated sprite tile layers 
- can use the two MD layers as bitmap layers with parallax

Tiled plugins
============

The extensions/ directory contains Javascript plugins for exporting Tilemaps and Tilesets in YATSSD format.

The plugins require Tiled version 1.5 or newer.

[More information on Tiled plugins.](https://doc.mapeditor.org/en/stable/reference/scripting/)

License
============
If a source file does not have a license header stating otherwise, then it is covered by the MIT license.

The demo uses graphical assets from Little Game Engine: https://github.com/mills32/Little-Game-Engine-for-VGA

Credits
============

Original code by Victor Luchitz aka Vic

32X devkit by Joseph Fenton aka Chilly Willy
