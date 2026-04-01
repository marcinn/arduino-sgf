#!/usr/bin/env python3
from __future__ import annotations

import argparse
import wave
from pathlib import Path


def write_text_if_changed(path: Path, content: str) -> None:
    if path.exists():
        existing = path.read_text(encoding="ascii")
        if existing == content:
            return
    path.write_text(content, encoding="ascii")


def output_namespace(name: str) -> str:
    cleaned = "".join(ch if ch.isalnum() or ch == "_" else "_" for ch in name)
    cleaned = cleaned.strip("_") or "Generated"
    if cleaned[0].isdigit():
        cleaned = "G_" + cleaned
    return cleaned


def parse_manifest(path: Path, source_dir_name: str) -> dict[str, dict[str, object]]:
    if not path.exists():
        default_manifest = f"""# code [root=Hz]
# Supported codes list. The generator reads files from {source_dir_name}/<code>.wav.
# If {source_dir_name}/<code>.wav exists, it will be used.
"""
        write_text_if_changed(path, default_manifest)
        return {}

    entries: dict[str, dict[str, object]] = {}
    for line_no, raw_line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        code = parts[0]
        root_hz = 1.0
        for token in parts[1:]:
            if token.startswith("root="):
                root_hz = float(token.split("=", 1)[1])
                continue
            raise ValueError(f"{path.name}:{line_no}: unsupported token {token!r}")
        entries[code] = {"root_hz": root_hz}
    return entries


def load_wav_as_int8(path: Path) -> tuple[list[int], int]:
    with wave.open(str(path), "rb") as wav:
        channels = wav.getnchannels()
        sample_width = wav.getsampwidth()
        sample_rate = wav.getframerate()
        frame_count = wav.getnframes()
        raw = wav.readframes(frame_count)

    if channels not in (1, 2):
        raise ValueError(f"{path.name}: only mono or stereo WAV is supported")
    if sample_width not in (1, 2):
        raise ValueError(f"{path.name}: only 8-bit or 16-bit PCM WAV is supported")

    data: list[int] = []
    if sample_width == 1:
        step = channels
        for i in range(0, len(raw), step):
            if channels == 1:
                sample = raw[i] - 128
            else:
                left = raw[i] - 128
                right = raw[i + 1] - 128
                sample = int(round((left + right) / 2.0))
            data.append(sample)
    else:
        step = 2 * channels
        for i in range(0, len(raw), step):
            if channels == 1:
                sample16 = int.from_bytes(raw[i:i + 2], byteorder="little", signed=True)
            else:
                left = int.from_bytes(raw[i:i + 2], byteorder="little", signed=True)
                right = int.from_bytes(raw[i + 2:i + 4], byteorder="little", signed=True)
                sample16 = int(round((left + right) / 2.0))
            data.append(max(-128, min(127, sample16 >> 8)))
    return data, sample_rate


def collect_entries(samples_dir: Path, metadata: dict[str, dict[str, object]]) -> list[dict[str, object]]:
    samples_dir.mkdir(parents=True, exist_ok=True)
    entries: list[dict[str, object]] = []
    for wav_path in sorted(samples_dir.glob("*.wav")):
        code = wav_path.stem
        meta = metadata.get(code, {})
        entries.append({
            "code": code,
            "path": wav_path,
            "root_hz": float(meta.get("root_hz", 1.0)),
        })
    return entries


def generate(project_dir: Path, output_base: str, entries: list[dict[str, object]]) -> None:
    namespace = output_namespace(output_base)
    header_path = project_dir / f"{output_base}.h"
    cpp_path = project_dir / f"{output_base}.cpp"

    header = f"""#pragma once

#include <stdint.h>

#include "SGF/Synth.h"

namespace {namespace} {{

struct Entry {{
  const char* code;
  const SGFAudio::AudioSample* sample;
}};

extern const Entry ENTRIES[];
extern const unsigned int ENTRY_COUNT;

}}
"""
    write_text_if_changed(header_path, header)

    lines: list[str] = [
        f'#include "{output_base}.h"',
        "",
        f"namespace {namespace} {{",
        "",
    ]
    entry_lines: list[str] = []

    for entry in entries:
        code = str(entry["code"])
        path = Path(entry["path"])
        root_hz = float(entry["root_hz"])
        pcm, sample_rate = load_wav_as_int8(path)
        ident = output_namespace(code)
        lines.append(f"static const int8_t pcm_{ident}[] = {{")
        row: list[str] = []
        for i, value in enumerate(pcm):
            row.append(f"{value}")
            if len(row) == 16 or i == len(pcm) - 1:
                lines.append("  " + ", ".join(row) + ",")
                row = []
        lines.append("};")
        lines.append(
            f"static const SGFAudio::AudioSample sample_{ident}{{"
            f" pcm_{ident}, {len(pcm)}u, {sample_rate}u, {root_hz}f, false, 0u, 0u }};"
        )
        lines.append("")
        entry_lines.append(f'  {{"{code}", &sample_{ident}}},')

    lines.append("const Entry ENTRIES[] = {")
    if entry_lines:
        lines.extend(entry_lines)
    lines.append("};")
    lines.append(f"const unsigned int ENTRY_COUNT = {len(entries)}u;")
    lines.append("")
    lines.append("}")
    write_text_if_changed(cpp_path, "\n".join(lines) + "\n")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--project-dir", default=".")
    parser.add_argument("--src-dir", default="samples")
    parser.add_argument("--out-base", default="SamplesGenerated")
    parser.add_argument("--manifest", default="samples.txt")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    project_dir = Path(args.project_dir).resolve()
    samples_dir = project_dir / args.src_dir
    manifest_path = project_dir / args.manifest
    metadata = parse_manifest(manifest_path, args.src_dir)
    entries = collect_entries(samples_dir, metadata)
    generate(project_dir, args.out_base, entries)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
