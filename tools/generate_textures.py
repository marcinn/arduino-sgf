#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
import struct
from pathlib import Path

TEX_SIZE = 16
BLOB_MAGIC = 0x5754  # "WT"


def write_text_if_changed(path: Path, content: str) -> None:
    if path.exists():
        existing = path.read_text(encoding="ascii")
        if existing == content:
            return
    path.write_text(content, encoding="ascii")


def lerp(a: int, b: int, step: int, count: int) -> int:
    if count <= 1:
        return a
    return int(round(a + (b - a) * (step / float(count - 1))))


def ramp(start: tuple[int, int, int], end: tuple[int, int, int], count: int = 16) -> list[tuple[int, int, int]]:
    return [
        (
            lerp(start[0], end[0], i, count),
            lerp(start[1], end[1], i, count),
            lerp(start[2], end[2], i, count),
        )
        for i in range(count)
    ]


def build_palette() -> list[tuple[int, int, int]]:
    ramps = [
        ((0, 0, 0), (255, 255, 255)),
        ((28, 16, 8), (236, 208, 164)),
        ((36, 0, 0), (255, 164, 148)),
        ((52, 16, 0), (255, 212, 72)),
        ((54, 42, 0), (255, 244, 172)),
        ((0, 26, 8), (176, 255, 196)),
        ((0, 24, 28), (168, 252, 255)),
        ((12, 8, 36), (248, 156, 255)),
    ]
    palette: list[tuple[int, int, int]] = []
    for start, end in ramps:
        palette.extend(ramp(start, end))
    assert len(palette) == 128
    return palette


def write_palette_gpl(path: Path, palette: list[tuple[int, int, int]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "GIMP Palette",
        "Name: Wolf 128",
        "Columns: 8",
        "#",
    ]
    for idx, (r, g, b) in enumerate(palette):
        lines.append(f"{r:3d} {g:3d} {b:3d} C{idx:03d}")
    write_text_if_changed(path, "\n".join(lines) + "\n")


def read_palette_gpl(path: Path) -> list[tuple[int, int, int]]:
    if not path.exists():
        palette = build_palette()
        write_palette_gpl(path, palette)
        return palette

    palette: list[tuple[int, int, int]] = []
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("GIMP Palette"):
            continue
        if line.startswith("Name:"):
            continue
        if line.startswith("Columns:"):
            continue

        parts = line.split()
        if len(parts) < 3:
            continue

        try:
            r = int(parts[0])
            g = int(parts[1])
            b = int(parts[2])
        except ValueError as exc:
            raise ValueError(f"invalid palette entry in {path.name}: {raw_line!r}") from exc

        if not (0 <= r <= 255 and 0 <= g <= 255 and 0 <= b <= 255):
            raise ValueError(f"palette value out of range in {path.name}: {raw_line!r}")
        palette.append((r, g, b))

    if len(palette) != 128:
        raise ValueError(f"{path.name} must contain exactly 128 colors, got {len(palette)}")
    return palette


def read_bmp(path: Path) -> tuple[list[tuple[int, int, int]], list[int] | None]:
    data = path.read_bytes()
    if data[:2] != b"BM":
        raise ValueError("not a BMP file")

    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    dib_size = struct.unpack_from("<I", data, 14)[0]
    if dib_size < 40:
        raise ValueError("unsupported BMP header")

    width = struct.unpack_from("<i", data, 18)[0]
    height = struct.unpack_from("<i", data, 22)[0]
    planes = struct.unpack_from("<H", data, 26)[0]
    bitcount = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]
    colors_used = struct.unpack_from("<I", data, 46)[0]

    if planes != 1:
        raise ValueError("invalid BMP planes")
    if compression != 0:
        raise ValueError("compressed BMP is unsupported")

    top_down = height < 0
    height = abs(height)
    if width != TEX_SIZE or height != TEX_SIZE:
        raise ValueError(f"expected {TEX_SIZE}x{TEX_SIZE}, got {width}x{height}")

    row_size = ((bitcount * width + 31) // 32) * 4
    pixels: list[tuple[int, int, int]] = []

    if bitcount == 8:
        palette_offset = 14 + dib_size
        if colors_used == 0:
            colors_used = 256
        bmp_palette = []
        for i in range(colors_used):
            b, g, r, _ = struct.unpack_from("<BBBB", data, palette_offset + i * 4)
            bmp_palette.append((r, g, b))
        indices: list[int] = []
        for y in range(height):
            src_y = y if top_down else (height - 1 - y)
            row = pixel_offset + src_y * row_size
            for x in range(width):
                idx = data[row + x]
                indices.append(idx)
                pixels.append(bmp_palette[idx])
        return pixels, indices

    if bitcount not in (24, 32):
        raise ValueError("only 8-bit indexed, 24-bit and 32-bit BMP are supported")

    bytes_per_pixel = bitcount // 8
    for y in range(height):
        src_y = y if top_down else (height - 1 - y)
        row = pixel_offset + src_y * row_size
        for x in range(width):
            px = row + x * bytes_per_pixel
            b = data[px + 0]
            g = data[px + 1]
            r = data[px + 2]
            pixels.append((r, g, b))
    return pixels, None


def output_namespace(name: str) -> str:
    ident = re.sub(r"[^a-zA-Z0-9_]", "_", name)
    ident = re.sub(r"_+", "_", ident).strip("_")
    if not ident:
        ident = "Generated"
    if ident[0].isdigit():
        ident = "G_" + ident
    return ident


def palette_index(rgb: tuple[int, int, int], palette: list[tuple[int, int, int]]) -> int:
    best_idx = 0
    best_dist = 1 << 62
    r0, g0, b0 = rgb
    for idx, cand in enumerate(palette):
        dr = r0 - cand[0]
        dg = g0 - cand[1]
        db = b0 - cand[2]
        dist = dr * dr * 3 + dg * dg * 4 + db * db * 2
        if dist < best_dist:
            best_dist = dist
            best_idx = idx
    return best_idx


def load_assets(textures_dir: Path, palette: list[tuple[int, int, int]]) -> list[tuple[str, list[int]]]:
    textures_dir.mkdir(parents=True, exist_ok=True)
    assets: list[tuple[str, list[int]]] = []
    for bmp_path in sorted(textures_dir.glob("*.bmp")):
        name = bmp_path.stem
        pixels, source_indices = read_bmp(bmp_path)
        if source_indices is not None:
            indexed = source_indices
        else:
            indexed = [palette_index(px, palette) for px in pixels]
        assets.append((name, indexed))
    return assets


def build_blob(assets: list[tuple[str, list[int]]]) -> bytes:
    blob = bytearray()
    blob.extend(struct.pack("<HH", BLOB_MAGIC, len(assets)))
    for name, indices in assets:
        name_bytes = name.encode("ascii") + b"\0"
        blob.extend(struct.pack("<H", len(name_bytes)))
        blob.extend(name_bytes)
        blob.extend(struct.pack("<H", len(indices)))
        blob.extend(bytes(indices))
    return bytes(blob)


def bytes_lines(data: bytes, width: int = 16) -> list[str]:
    lines: list[str] = []
    for offset in range(0, len(data), width):
        chunk = data[offset:offset + width]
        lines.append("  " + ", ".join(f"0x{value:02X}" for value in chunk) + ",")
    return lines


def write_generated(project_dir: Path, output_base: str, palette: list[tuple[int, int, int]], assets: list[tuple[str, list[int]]]) -> None:
    namespace = output_namespace(output_base)
    header_path = project_dir / f"{output_base}.h"
    cpp_path = project_dir / f"{output_base}.cpp"

    header = f"""#pragma once

#include "Textures.h"

namespace {namespace} {{

extern const uint8_t PALETTE_RGB[Textures::PALETTE_SIZE * 3];
extern const uint8_t BLOB[];
extern const unsigned int BLOB_SIZE;

}}
"""
    write_text_if_changed(header_path, header)

    blob = build_blob(assets)
    lines: list[str] = [
        f'#include "{output_base}.h"',
        "",
        f"namespace {namespace} {{",
        "",
        "const uint8_t PALETTE_RGB[Textures::PALETTE_SIZE * 3] = {",
    ]

    palette_bytes = bytearray()
    for r, g, b in palette:
        palette_bytes.extend((r, g, b))
    lines.extend(bytes_lines(bytes(palette_bytes), 12))
    lines.extend([
        "};",
        "",
        "const uint8_t BLOB[] = {",
    ])
    lines.extend(bytes_lines(blob))
    lines.extend([
        "};",
        f"const unsigned int BLOB_SIZE = {len(blob)}u;",
        "",
        "}",
    ])
    write_text_if_changed(cpp_path, "\n".join(lines) + "\n")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--project-dir", default=".")
    parser.add_argument("--src-dir", default="textures")
    parser.add_argument("--out-base", default="TexturesGenerated")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    project_dir = Path(args.project_dir).resolve()
    textures_dir = project_dir / args.src_dir
    palette_file = textures_dir / "wolf-128.gpl"
    palette = read_palette_gpl(palette_file)
    assets = load_assets(textures_dir, palette)
    write_generated(project_dir, args.out_base, palette, assets)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
