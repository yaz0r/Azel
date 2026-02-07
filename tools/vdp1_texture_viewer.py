#!/usr/bin/env python3
"""
VDP1 Texture Viewer for Panzer Dragoon Saga
Displays raw CGB (Character Graphics) files used by the Sega Saturn VDP1

Usage: python vdp1_texture_viewer.py <CGB_FILE> [--width W] [--height H] [--bpp 4|8] [--offset O]
"""

import sys
import os
import struct
import argparse
from pathlib import Path

try:
    from PIL import Image
    import numpy as np
    HAS_PIL = True
except ImportError:
    HAS_PIL = False
    print("Warning: PIL/Pillow not installed. Install with: pip install Pillow numpy")


def sat_rgb555_to_rgb888(color16):
    """Convert Saturn 15-bit RGB (RGB555) to 24-bit RGB"""
    r = ((color16 >> 10) & 0x1F) << 3
    g = ((color16 >> 5) & 0x1F) << 3
    b = (color16 & 0x1F) << 3
    return (r, g, b)


def create_default_palette(num_colors=256):
    """Create a default gradient palette for visualization"""
    palette = []
    for i in range(num_colors):
        # Create a colorful gradient to make patterns visible
        if num_colors == 16:
            # For 16 colors, use distinct colors
            hue = (i * 360 // 16) % 360
            r = int(127 + 127 * np.sin(np.radians(hue)))
            g = int(127 + 127 * np.sin(np.radians(hue + 120)))
            b = int(127 + 127 * np.sin(np.radians(hue + 240)))
        else:
            # For 256 colors, use a gradient
            r = (i * 7) % 256
            g = (i * 13) % 256
            b = (i * 23) % 256
        palette.append((r, g, b))
    # Color 0 is usually transparent - make it magenta for visibility
    palette[0] = (255, 0, 255)
    return palette


def load_cgb_4bpp(data, width, height, offset=0):
    """Load 4 bits-per-pixel (16 color) texture data"""
    pixels = []
    idx = offset
    for y in range(height):
        row = []
        for x in range(0, width, 2):
            if idx >= len(data):
                row.extend([0, 0])
                continue
            byte = data[idx]
            # High nibble first, then low nibble
            pixel1 = (byte >> 4) & 0x0F
            pixel2 = byte & 0x0F
            row.append(pixel1)
            row.append(pixel2)
            idx += 1
        pixels.append(row[:width])
    return pixels


def load_cgb_8bpp(data, width, height, offset=0):
    """Load 8 bits-per-pixel (256 color) texture data"""
    pixels = []
    idx = offset
    for y in range(height):
        row = []
        for x in range(width):
            if idx >= len(data):
                row.append(0)
                continue
            row.append(data[idx])
            idx += 1
        pixels.append(row)
    return pixels


def load_palette_from_file(palette_file):
    """Try to load a palette from a file (various formats)"""
    with open(palette_file, 'rb') as f:
        data = f.read()

    palette = []
    # Try to interpret as 16-bit RGB555 colors
    for i in range(0, min(len(data), 512), 2):
        if i + 1 < len(data):
            # Big-endian (Saturn format)
            color16 = (data[i] << 8) | data[i + 1]
            palette.append(sat_rgb555_to_rgb888(color16))

    if not palette:
        return None
    return palette


def guess_dimensions(file_size, bpp=4):
    """Try to guess texture dimensions based on file size"""
    bytes_per_pixel = bpp / 8
    total_pixels = int(file_size / bytes_per_pixel)

    # Common VDP1 texture sizes
    common_widths = [8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 128, 160, 256, 320, 352]

    guesses = []
    for w in common_widths:
        if total_pixels % w == 0:
            h = total_pixels // w
            if 1 <= h <= 512:
                guesses.append((w, h))

    return guesses


def display_texture_ascii(pixels, palette, max_width=80):
    """Display texture as ASCII art in terminal"""
    height = len(pixels)
    width = len(pixels[0]) if pixels else 0

    # Scale down if too wide
    scale = max(1, width // max_width)

    chars = " .:-=+*#%@"

    for y in range(0, height, scale * 2):  # *2 because terminal chars are taller than wide
        line = ""
        for x in range(0, width, scale):
            if y < height and x < width:
                idx = pixels[y][x]
                # Use brightness to select character
                if palette:
                    r, g, b = palette[idx % len(palette)]
                    brightness = (r + g + b) // 3
                else:
                    brightness = idx * (255 // 15) if idx < 16 else idx
                char_idx = min(len(chars) - 1, brightness * len(chars) // 256)
                line += chars[char_idx]
            else:
                line += " "
        print(line)


def create_image(pixels, palette):
    """Create a PIL Image from pixel data and palette"""
    if not HAS_PIL:
        return None

    height = len(pixels)
    width = len(pixels[0]) if pixels else 0

    img = Image.new('RGB', (width, height))
    img_data = []

    for y in range(height):
        for x in range(width):
            idx = pixels[y][x]
            if palette and idx < len(palette):
                img_data.append(palette[idx])
            else:
                # Fallback grayscale
                v = (idx * 17) % 256 if idx < 16 else idx
                img_data.append((v, v, v))

    img.putdata(img_data)
    return img


def scan_for_textures(data, bpp=4):
    """Scan file and try to find texture-like patterns"""
    print(f"\nScanning {len(data)} bytes for texture patterns...")

    # Look for repeating patterns that might indicate texture rows
    min_width = 8
    max_width = 256

    for test_width in [8, 16, 32, 64, 128, 256]:
        if bpp == 4:
            row_bytes = test_width // 2
        else:
            row_bytes = test_width

        if row_bytes > len(data):
            continue

        # Check how many complete rows we could have
        num_rows = len(data) // row_bytes
        if num_rows >= 8:
            print(f"  Possible: {test_width}x{num_rows} ({bpp}bpp)")


def main():
    parser = argparse.ArgumentParser(description='VDP1 Texture Viewer for Panzer Dragoon Saga')
    parser.add_argument('cgb_file', help='CGB (Character Graphics) file to view')
    parser.add_argument('--width', '-w', type=int, default=0, help='Texture width (auto-detect if 0)')
    parser.add_argument('--height', '-H', type=int, default=0, help='Texture height (auto-detect if 0)')
    parser.add_argument('--bpp', type=int, default=4, choices=[4, 8], help='Bits per pixel (4 or 8)')
    parser.add_argument('--offset', '-o', type=int, default=0, help='Byte offset into file')
    parser.add_argument('--palette', '-p', help='Optional palette file')
    parser.add_argument('--output', help='Output image file (PNG)')
    parser.add_argument('--scan', action='store_true', help='Scan file for texture patterns')
    parser.add_argument('--grid', '-g', action='store_true', help='Show multiple texture guesses in a grid')
    parser.add_argument('--ascii', '-a', action='store_true', help='Display as ASCII art')

    args = parser.parse_args()

    if not os.path.exists(args.cgb_file):
        print(f"Error: File not found: {args.cgb_file}")
        return 1

    # Load file
    with open(args.cgb_file, 'rb') as f:
        data = f.read()

    print(f"Loaded: {args.cgb_file}")
    print(f"Size: {len(data)} bytes")

    if args.scan:
        scan_for_textures(data, args.bpp)
        return 0

    # Load or create palette
    if args.palette and os.path.exists(args.palette):
        palette = load_palette_from_file(args.palette)
        if palette:
            print(f"Loaded palette: {len(palette)} colors")
    else:
        palette = create_default_palette(16 if args.bpp == 4 else 256)
        print("Using default palette (specify --palette for actual colors)")

    # Determine dimensions
    if args.width == 0 or args.height == 0:
        guesses = guess_dimensions(len(data) - args.offset, args.bpp)
        if not guesses:
            print("Could not auto-detect dimensions. Please specify --width and --height")
            return 1

        if args.grid and HAS_PIL and len(guesses) > 1:
            # Show multiple possibilities in a grid
            print(f"\nShowing {len(guesses)} possible dimension combinations:")
            images = []
            for w, h in guesses[:9]:  # Max 9 images
                print(f"  {w}x{h}")
                if args.bpp == 4:
                    pixels = load_cgb_4bpp(data, w, h, args.offset)
                else:
                    pixels = load_cgb_8bpp(data, w, h, args.offset)
                img = create_image(pixels, palette)
                if img:
                    images.append((img, f"{w}x{h}"))

            if images:
                # Create grid
                cols = min(3, len(images))
                rows = (len(images) + cols - 1) // cols
                max_w = max(img.width for img, _ in images)
                max_h = max(img.height for img, _ in images)

                grid = Image.new('RGB', (cols * (max_w + 10), rows * (max_h + 20)), (64, 64, 64))
                for i, (img, label) in enumerate(images):
                    x = (i % cols) * (max_w + 10) + 5
                    y = (i // cols) * (max_h + 20) + 5
                    grid.paste(img, (x, y))

                output = args.output or f"{Path(args.cgb_file).stem}_grid.png"
                grid.save(output)
                print(f"\nSaved grid to: {output}")

                # Try to show
                try:
                    grid.show()
                except:
                    pass
            return 0

        # Use first guess
        args.width, args.height = guesses[0]
        print(f"Auto-detected dimensions: {args.width}x{args.height}")
        print(f"Other possibilities: {guesses[1:5]}")

    # Load texture
    print(f"\nLoading {args.bpp}bpp texture: {args.width}x{args.height}")
    if args.bpp == 4:
        pixels = load_cgb_4bpp(data, args.width, args.height, args.offset)
    else:
        pixels = load_cgb_8bpp(data, args.width, args.height, args.offset)

    # ASCII display
    if args.ascii or not HAS_PIL:
        print("\nASCII Preview:")
        display_texture_ascii(pixels, palette)

    # Create and save/show image
    if HAS_PIL:
        img = create_image(pixels, palette)
        if img:
            # Scale up small images for better viewing
            scale = max(1, 256 // max(args.width, args.height))
            if scale > 1:
                img = img.resize((args.width * scale, args.height * scale), Image.NEAREST)

            if args.output:
                img.save(args.output)
                print(f"\nSaved to: {args.output}")
            else:
                output = f"{Path(args.cgb_file).stem}_{args.width}x{args.height}.png"
                img.save(output)
                print(f"\nSaved to: {output}")

            # Try to display
            try:
                img.show()
            except Exception as e:
                print(f"(Could not open image viewer: {e})")

    return 0


if __name__ == '__main__':
    sys.exit(main())
