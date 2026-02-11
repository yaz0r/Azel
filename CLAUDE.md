# Azel (Panzer Dragoon Saga) Decompilation

## Project Overview
Saturn game decompilation/reimplementation using bgfx for rendering, SDL2 for input/windowing.

## Build
```
./build_linux.sh              # build (default RelWithDebInfo)
./build_linux.sh --deps       # install dependencies first
cd build_linux/PDS && ./PDS   # run (must be in PDS dir, or binary auto-chdirs)
```
Binary auto-detects exe directory via /proc/self/exe on Linux.

## Architecture

### VDP2 Background Layers
- **NBG0-NBG3, RBG0**: Saturn VDP2 scroll planes
- GPU mode: rendered via bgfx framebuffer + VDP2 shader (reads VRAM/CRAM textures)
- Software mode: CPU renders to buffer, uploads as BGRA8 texture via bgfx::updateTexture2D
- Toggle via ImGui checkboxes (BG0_GPU, BG1_GPU, etc.)
- When switching modes, texture resources must be recreated (FB vs standalone)

### Texture Format / Sampler Type (Critical)
- VDP2 RAM/CRAM textures use `R8U` (unsigned integer) format
- Shader must use `USAMPLER2D` + `uvec4 texel = texelFetch(...)` + `int(texel.r)`
- Compiled shader binaries in shared `data/` must match the C++ texture format

### Shader Compilation
- bgfx shaders compiled with shaderc, output to `data/shaders/generated/`
- `data/` is symlinked to `/storage01/code/Azel/data` (shared with reference version)
- Linux builds: GLSL 150 + SPIRV profiles
- Shader loading uses relative path `shaders/generated/` from exe directory
- If shaders are stale (from different shaderc), delete `data/shaders/generated/*.spirv.bin *.glsl.bin` and rebuild

### Resolution Handling (renderer.cpp azelSdl2_EndFrame)
VDP1 (3D/sprites) and VDP2 (backgrounds) render at different resolutions.
VDP1 output resolution (used for 3D geometry):
```
HRESO 0,4: 320px    HRESO 1,5: 352px
HRESO 2,6: 320px    HRESO 3,7: 352px
```
VDP2 native resolution (backgrounds render at this, may be wider):
```
HRESO 0,1,4,5: same as VDP1
HRESO 2,6: 640px    HRESO 3,7: 704px
```
LSMD >= 2: doubles height (224 -> 448, interlace)
Title screen uses HRESO=3 + LSMD=3 -> VDP2 renders at 704x448, VDP1 at 352x224.

### Compositing (renderer.cpp)
Layers composited in priority order (0-7) into CompositeView:
- SPRITE_POLY rendered first at its priority
- Then NBG0, NBG1, NBG3, RBG0 at their respective priorities
- Priority registers: PRINA (NBG0/NBG1), PRINB (NBG2/NBG3), PRIR (RBG0)
- Title screen: PRINA=0x706 -> NBG0=priority 6, NBG1=priority 7

### Renderer Backend
- Linux: env var `AZEL_RENDERER=vulkan|opengl` (auto tries Vulkan first, falls back to OpenGL)
- Wayland: uses `wmi.info.wl.surface` (not egl_window)

## Title Screen Flow

### Task Hierarchy
```
startLoadWarningTask
  -> createTitleScreenTask (s_titleScreenWorkArea)
       Init: TITLE_OVERLAY::overlayStart() loads graphics
       States 0-3: wait for overlay + 60 frame delay
       State 4: spawn "PRESS START BUTTON" blink task
       State 5: wait for Start -> state 7 (or timeout -> Sega logo)
       State 7: pendingTask = createTitleMenuTask
            -> s_titleMenuWorkArea (selection menu)
```

### VDP2 Layers on Title Screen
- **NBG0** (priority 6): Background art, 256 colors
  - TITLEE.SCB at VRAM 0x20000 (character data)
  - TITLEE.PNB at VRAM 0x10000 (pattern name table)
  - titleScreenPalette at CRAM 0x000 and 0xE00
  - HRESO=3 (704px), LSMD=interlace
- **NBG1** (priority 7): Text overlay, 16 colors
  - Font from ASCII.CGZ decompressed to VRAM 0x0000
  - Pattern name table at vdp2TextMemoryOffset (0x6000)
  - CAOS=7 -> palette offset 0xE00
- **NBG3**: Debug/system text layer (initVdp2TextLayer)

### Selection Menu (s_titleMenuWorkArea in titleScreen.cpp)
- Debug menu: NEW GAME, CONTINUE, TOWN, FIELD, BATTLE
- Start/A selects, Up/Down cycles (wraps, skips disabled entries)
- Selected item blinks via drawLineLargeFont / clearVdp2TextLargeFont
- Menu text position: vdp2DebugPrintSetPosition(0x11, verticalLocation + 0x17)
  - 0x11 = column 17, 0x17 = row 23 in the 64x64 tile map
  - Large font: each char is 2x2 tiles (16x16 pixels)
  - Pixel position in 704px mode: column 17 * 8 = pixel 136, row 23 * 8 = pixel 184
- Arrow indicators at columns 0x0F and 0x1C (palette 0x9000)
- Uses palette 0xC000 for menu text, 0xD000 for "PRESS START BUTTON"
- "PANZER DRAGOON SAGA" subtitle: setupVDP2StringRendering(6, 26, 40, 2) on NBG1

### VDP2 Text Rendering
- `loadFont()`: decompresses ASCII.CGZ to VRAM 0. 4bpp 8x8 cells, char number = ASCII + offset
- `drawLineLargeFont(text)`: 16x16 chars (2x2 cells) into pattern name table at vdp2TextMemoryOffset
- `VDP2DrawString(text)`: higher-level with cursor/word-wrap, used for "PANZER DRAGOON SAGA" subtitle
- `clearVdp2TextLargeFont()`: zeroes pattern entries at cursor position

## Current Status
- Background image (NBG0) renders correctly on title screen
- Menu text (NBG1) is NOT rendering - needs investigation
  - Text is written to VDP2 VRAM pattern name table at 0x6000
  - NBG1 layer config: 16 colors, 1-word patterns, CNSM=1, CAOS=7
  - Possible issues: NBG1 not enabled in BGON, priority/compositing, scroll offset wrong, or software render path not working for NBG1
