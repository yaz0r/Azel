#!/usr/bin/env python3
import argparse
import os
import re
import struct
from typing import List, Tuple

try:
    from PIL import Image
    HAS_PIL = True
except ImportError:
    HAS_PIL = False


VRAM_SIZE = 0x80000
VRAM_PAD = 0x20000
VRAM_BASE = VRAM_PAD
CRAM_SIZE = 0x1000


def load_file(path: str) -> bytes:
    with open(path, "rb") as f:
        return f.read()


def parse_palette_from_o_title(o_title_path: str) -> List[int]:
    text = load_file(o_title_path).decode("utf-8", errors="ignore")
    marker = "u8 titleScreenPalette"
    start = text.find(marker)
    if start == -1:
        raise RuntimeError("Failed to find titleScreenPalette in o_title.cpp")
    brace_start = text.find("{", start)
    brace_end = text.find("};", brace_start)
    if brace_start == -1 or brace_end == -1:
        raise RuntimeError("Failed to parse titleScreenPalette block")
    body = text[brace_start + 1:brace_end]
    vals = re.findall(r"0x[0-9A-Fa-f]+|\\d+", body)
    data = [int(v, 16) if v.lower().startswith("0x") else int(v) for v in vals]
    if len(data) < 512:
        raise RuntimeError(f"Parsed palette has {len(data)} bytes, expected 512")
    return data[:512]


def get_vram_u8(vram: bytearray, offset: int) -> int:
    return vram[VRAM_BASE + (offset & 0x7FFFF)]


def get_vram_u16(vram: bytearray, offset: int) -> int:
    hi = get_vram_u8(vram, offset)
    lo = get_vram_u8(vram, offset + 1)
    return (hi << 8) | lo


def get_cram_u16(cram: bytearray, offset: int) -> int:
    hi = cram[offset & 0xFFF]
    lo = cram[(offset + 1) & 0xFFF]
    return (hi << 8) | lo


def init_layer_map(layer: int, plane_a: int, plane_b: int, plane_c: int, plane_d: int,
                   chsz: int, pnb: int) -> Tuple[int, int, int, int, int]:
    # Mirrors VDP2.cpp::initLayerMap
    # pnb: 0 -> 2 words, 1 -> 1 word
    if pnb:
        shift_value = 11 if chsz else 13
    else:
        shift_value = 12 if chsz else 14

    map_offset = (plane_a >> (shift_value + 6)) & 7

    def mk_mpabn(pa: int, pb: int) -> int:
        return ((pb >> shift_value) & 0x3F) << 8 | ((pa >> shift_value) & 0x3F)

    def mk_mpcdn(pc: int, pd: int) -> int:
        return ((pd >> shift_value) & 0x3F) << 8 | ((pc >> shift_value) & 0x3F)

    if layer == 0:
        mpofn = map_offset
    elif layer == 1:
        mpofn = map_offset << 4
    elif layer == 2:
        mpofn = map_offset << 8
    else:
        mpofn = map_offset << 12

    mpabn = mk_mpabn(plane_a, plane_b)
    mpcdn = mk_mpcdn(plane_c, plane_d)
    return mpofn, mpabn, mpcdn, shift_value, map_offset


def render_layer(vram: bytearray, cram: bytearray, width: int, height: int,
                 chsz: int, chcn: int, pnb: int, cnsm: int, caos: int,
                 plsz: int, scn: int, scroll_x: int, scroll_y: int,
                 plane_offsets: List[int]) -> List[int]:
    # Mirrors renderer.cpp::renderLayer
    out = [0x00000000] * (width * height)

    cell_dot_dimension = 8
    char_pattern_dot_dimension = cell_dot_dimension * (1 if chsz == 0 else 2)
    page_dot_dimension = 8 * 64
    plane_dot_width = page_dot_dimension * (2 if (plsz & 1) else 1)
    plane_dot_height = page_dot_dimension * (2 if (plsz & 2) else 1)

    page_dimension = 64 if chsz == 0 else 32
    pattern_size = 4 if pnb == 0 else 2
    page_size = page_dimension * page_dimension * pattern_size

    cell_size_in_bytes = 8 * 8
    if chcn == 0:
        cell_size_in_bytes //= 2
    elif chcn == 1:
        cell_size_in_bytes *= 1
    else:
        return out

    for raw_y in range(height):
        for raw_x in range(width):
            output_x = raw_x + scroll_x
            output_y = raw_y + scroll_y
            if output_x < 0 or output_y < 0:
                continue

            plane_x = output_x // plane_dot_width
            plane_y = output_y // plane_dot_height
            dot_in_plane_x = output_x % plane_dot_width
            dot_in_plane_y = output_y % plane_dot_height

            page_x = dot_in_plane_x // page_dot_dimension
            page_y = dot_in_plane_y // page_dot_dimension
            dot_in_page_x = dot_in_plane_x % page_dot_dimension
            dot_in_page_y = dot_in_plane_y % page_dot_dimension

            char_pattern_x = dot_in_page_x // char_pattern_dot_dimension
            char_pattern_y = dot_in_page_y // char_pattern_dot_dimension
            dot_in_char_x = dot_in_page_x % char_pattern_dot_dimension
            dot_in_char_y = dot_in_page_y % char_pattern_dot_dimension

            cell_x = dot_in_char_x // cell_dot_dimension
            cell_y = dot_in_char_y // cell_dot_dimension
            dot_in_cell_x = dot_in_char_x % cell_dot_dimension
            dot_in_cell_y = dot_in_char_y % cell_dot_dimension

            plane_number = plane_y * (2 if (plsz & 1) else 1) + plane_x
            start_of_plane = plane_offsets[plane_number]

            page_number = page_y * page_dimension + page_x
            start_of_page = start_of_plane + page_number * page_size

            pattern_number = char_pattern_y * page_dimension + char_pattern_x
            start_of_pattern = start_of_page + pattern_number * pattern_size

            if pattern_size == 2:
                pattern_name = get_vram_u16(vram, start_of_pattern)
                palette_number = (pattern_name >> 12) & 0xF

                if cnsm == 0:
                    if chsz == 0:
                        character_number = pattern_name & 0x3FF
                        character_number |= (scn & 0x1F) << 10
                    else:
                        character_number = (pattern_name & 0x3FF) << 2
                        character_number |= scn & 3
                        character_number |= (scn & 0x1C) << 10
                else:
                    if chsz == 0:
                        character_number = pattern_name & 0xFFF
                        character_number |= (scn & 0x1C) << 10
                    else:
                        character_number = (pattern_name & 0xFFF) << 2
                        character_number |= scn & 3
                        character_number |= (scn & 0x10) << 10

                character_offset = character_number * 0x20
            else:
                data1 = get_vram_u16(vram, start_of_pattern)
                data2 = get_vram_u16(vram, start_of_pattern + 2)
                character_number = data2 & 0x7FFF
                palette_number = data1 & 0x7F
                character_offset = character_number

            cell_index = cell_x + cell_y * 2
            cell_offset = character_offset + cell_index * cell_size_in_bytes

            if chcn == 0:
                dot_offset = cell_offset + dot_in_cell_y * 4 + (dot_in_cell_x // 2)
                dot_color = get_vram_u8(vram, dot_offset)
                if dot_in_cell_x & 1:
                    dot_color &= 0xF
                else:
                    dot_color >>= 4
            else:
                dot_offset = cell_offset + dot_in_cell_y * 8 + dot_in_cell_x
                dot_color = get_vram_u8(vram, dot_offset)

            if dot_color != 0:
                palette_offset = ((palette_number << 4) + dot_color) * 2 + caos * 0x200
                color = get_cram_u16(cram, palette_offset)
                final_color = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9))
                out[(height - 1 - raw_y) * width + raw_x] = final_color

    return out


def write_ppm(path: str, width: int, height: int, pixels: List[int], flip_y: bool) -> None:
    with open(path, "wb") as f:
        header = f"P6 {width} {height} 255\n"
        f.write(header.encode("ascii"))
        if flip_y:
            for y in range(height):
                row = (height - 1 - y) * width
                for x in range(width):
                    c = pixels[row + x]
                    r = (c >> 16) & 0xFF
                    g = (c >> 8) & 0xFF
                    b = c & 0xFF
                    f.write(bytes([r, g, b]))
        else:
            for c in pixels:
                r = (c >> 16) & 0xFF
                g = (c >> 8) & 0xFF
                b = c & 0xFF
                f.write(bytes([r, g, b]))


def write_png(path: str, width: int, height: int, pixels: List[int], flip_y: bool) -> None:
    if not HAS_PIL:
        raise RuntimeError("Pillow not installed. Install with: pip install Pillow")
    img = Image.new("RGB", (width, height))
    rgb = []
    if flip_y:
        for y in range(height):
            row = (height - 1 - y) * width
            for x in range(width):
                c = pixels[row + x]
                r = (c >> 16) & 0xFF
                g = (c >> 8) & 0xFF
                b = c & 0xFF
                rgb.append((r, g, b))
    else:
        for c in pixels:
            r = (c >> 16) & 0xFF
            g = (c >> 8) & 0xFF
            b = c & 0xFF
            rgb.append((r, g, b))
    img.putdata(rgb)
    img.save(path)


def main() -> None:
    parser = argparse.ArgumentParser(description="Standalone VDP2 title screen decoder")
    parser.add_argument("--data", default="/storage01/code/Azel/data", help="Path to data directory")
    parser.add_argument("--src", default="/storage01/code/Azel/AzelLib/o_title.cpp", help="Path to o_title.cpp for palette")
    parser.add_argument("--out", default="/tmp", help="Output directory")
    parser.add_argument("--format", choices=["ppm", "png"], default="png", help="Output format")
    parser.add_argument("--width", type=int, default=704, help="Output width")
    parser.add_argument("--height", type=int, default=448, help="Output height")
    parser.add_argument("--flip-y", action="store_true", help="Flip output vertically")
    args = parser.parse_args()

    vram = bytearray(VRAM_PAD + VRAM_SIZE)
    cram = bytearray(CRAM_SIZE)

    scb = load_file(os.path.join(args.data, "TITLEE.SCB"))
    pnb = load_file(os.path.join(args.data, "TITLEE.PNB"))

    vram[VRAM_BASE + 0x20000:VRAM_BASE + 0x20000 + len(scb)] = scb
    vram[VRAM_BASE + 0x10000:VRAM_BASE + 0x10000 + len(pnb)] = pnb

    palette = parse_palette_from_o_title(args.src)
    # Match engine: title palette in CRAM 0x0000 and text palette in CRAM 0x0E00.
    cram[0:0x200] = bytes(palette)
    cram[0xE00:0xE00 + 0x200] = bytes(palette)

    # Title screen layer configs from o_title.cpp
    nbg0 = {
        "chcn": 1, "chsz": 1, "pnb": 1, "cnsm": 1,
        "caos": 0, "plsz": 0, "scn": 0,
        "scroll_x": 0, "scroll_y": 0,
        "plane_a": 0x10000, "plane_b": 0x10800, "plane_c": 0x10000, "plane_d": 0x10800,
    }
    nbg1 = {
        "chcn": 0, "chsz": 0, "pnb": 1, "cnsm": 1,
        "caos": 7, "plsz": 0, "scn": 0,
        "scroll_x": 0, "scroll_y": 0,
        "plane_a": 0x6000, "plane_b": 0x6000, "plane_c": 0x6000, "plane_d": 0x6000,
    }

    # Load font (ASCII.CGZ) into VRAM at base like loadFont()
    ascii_cgz = load_file(os.path.join(args.data, "ASCII.CGZ"))
    # Decompress with the same algorithm as unpackGraphicsToVDP2
    def unpack_graphics_to_vdp2(compressed: bytes, dest: bytearray, dest_offset: int) -> None:
        # Port of VDP2.cpp::unpackGraphicsToVDP2 (no wrapping; fail if out of range).
        p = dest_offset
        idx = 0
        r6 = compressed[idx]
        idx += 1
        r7 = 9

        def check_range(addr: int) -> None:
            if addr < 0 or addr >= len(dest):
                raise IndexError(f"VRAM write/read out of range: 0x{addr:X}")

        while True:
            r7 -= 1
            if r7 == 0:
                r6 = compressed[idx]
                idx += 1
                r7 = 8

            bit = r6 & 1
            r6 >>= 1
            if bit:
                check_range(p)
                dest[p] = compressed[idx]
                idx += 1
                p += 1
                continue

            r7 -= 1
            if r7 == 0:
                r6 = compressed[idx]
                idx += 1
                r7 = 8

            bit2 = r6 & 1
            r6 >>= 1

            if bit2:
                r0 = compressed[idx]; idx += 1
                r1 = compressed[idx]; idx += 1
                composite = (r1 << 8) | r0
                if composite == 0:
                    return
                # composite is signed 16, then >>= 3, then sign-extend to 13 bits.
                if composite & 0x8000:
                    composite -= 0x10000
                composite >>= 3
                if composite & 0x1000:
                    composite -= 0x2000

                r0 &= 7
                if r0:
                    r0 += 2
                    source = p + composite
                    for _ in range(r0):
                        check_range(p)
                        check_range(source)
                        dest[p] = dest[source]
                        p += 1
                        source += 1
                else:
                    r0 = compressed[idx]; idx += 1
                    source = p + composite
                    for _ in range(r0 + 1):
                        check_range(p)
                        check_range(source)
                        dest[p] = dest[source]
                        p += 1
                        source += 1
            else:
                r0 = 0
                r7 -= 1
                if r7 == 0:
                    r6 = compressed[idx]
                    idx += 1
                    r7 = 8
                bit3 = r6 & 1
                r6 >>= 1
                r0 |= bit3

                r7 -= 1
                if r7 == 0:
                    r6 = compressed[idx]
                    idx += 1
                    r7 = 8
                bit4 = r6 & 1
                r6 >>= 1
                r0 = (r0 << 1) | bit4

                r2 = compressed[idx]; idx += 1
                if r2 & 0x80:
                    r2 -= 0x100
                r0 += 2
                source = p + r2
                for _ in range(r0):
                    check_range(p)
                    check_range(source)
                    dest[p] = dest[source]
                    p += 1
                    source += 1

    unpack_graphics_to_vdp2(ascii_cgz, vram, VRAM_BASE + 0x0000)

    # Write a debug string into the text map at vdp2TextMemoryOffset (0x6000)
    def write_text_map(text: str, x: int, y: int) -> None:
        base = VRAM_BASE + 0x6000 + ((y * 64 + x) * 2)
        r11 = (12 << 12) + 0x63
        for ch in text:
            if ch == "\n":
                break
            glyph = r11 + (ord(ch) - 0x20) * 2
            off = base
            vram[off] = (glyph >> 8) & 0xFF
            vram[off + 1] = glyph & 0xFF
            vram[off + 0x80] = ((glyph + 1) >> 8) & 0xFF
            vram[off + 0x81] = (glyph + 1) & 0xFF
            base += 2

    write_text_map("PANZER DRAGOON SAGA", 6, 26)

    width = args.width
    height = args.height

    rendered_layers = {}
    for layer_name, layer in [("nbg0", nbg0), ("nbg1", nbg1)]:
        mpofn, mpabn, mpcdn, _, _ = init_layer_map(
            0 if layer_name == "nbg0" else 1,
            layer["plane_a"], layer["plane_b"], layer["plane_c"], layer["plane_d"],
            layer["chsz"], layer["pnb"]
        )

        page_dimension = 64 if layer["chsz"] == 0 else 32
        pattern_size = 4 if layer["pnb"] == 0 else 2
        page_size = page_dimension * page_dimension * pattern_size

        if layer_name == "nbg0":
            offset = (mpofn & 7) << 6
        else:
            offset = ((mpofn >> 4) & 7) << 6

        plane_offsets = [
            (offset + (mpabn & 0x3F)) * page_size,
            (offset + ((mpabn >> 8) & 0x3F)) * page_size,
            (offset + (mpcdn & 0x3F)) * page_size,
            (offset + ((mpcdn >> 8) & 0x3F)) * page_size,
        ]

        pixels = render_layer(
            vram, cram, width, height,
            layer["chsz"], layer["chcn"], layer["pnb"], layer["cnsm"], layer["caos"],
            layer["plsz"], layer["scn"], layer["scroll_x"], layer["scroll_y"],
            plane_offsets,
        )

        ext = "png" if args.format == "png" else "ppm"
        out_path = os.path.join(args.out, f"title_{layer_name}.{ext}")
        if args.format == "png":
            if not HAS_PIL:
                raise RuntimeError("Pillow not installed. Install with: pip install Pillow")
            write_png(out_path, width, height, pixels, args.flip_y)
        else:
            write_ppm(out_path, width, height, pixels, args.flip_y)
        print(f"Wrote {out_path}")
        rendered_layers[layer_name] = pixels

    # Composite NBG1 over NBG0 into a single image.
    if "nbg0" in rendered_layers and "nbg1" in rendered_layers:
        base = rendered_layers["nbg0"]
        overlay = rendered_layers["nbg1"]
        merged = base[:]
        for i, c in enumerate(overlay):
            if c & 0x00FFFFFF:
                merged[i] = c
        ext = "png" if args.format == "png" else "ppm"
        out_path = os.path.join(args.out, f"title_full.{ext}")
        if args.format == "png":
            if not HAS_PIL:
                raise RuntimeError("Pillow not installed. Install with: pip install Pillow")
            write_png(out_path, width, height, merged, args.flip_y)
        else:
            write_ppm(out_path, width, height, merged, args.flip_y)
        print(f"Wrote {out_path}")


if __name__ == "__main__":
    main()
