#!/usr/bin/env python3

import argparse
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


AR_MAGIC = b"!<arch>\n"
AR_HEADER_SIZE = 60


def parse_decimal(field: bytes, label: str) -> int:
    try:
        return int(field.decode("ascii").strip() or "0")
    except ValueError as exc:
        raise ValueError(f"Invalid ar {label}: {field!r}") from exc


def member_name(raw_name: bytes, long_names: bytes | None) -> str:
    name = raw_name.decode("utf-8", errors="replace").strip()

    if name in {"/", "//"}:
        return name

    if name.startswith("/") and name[1:].isdigit():
        if long_names is None:
            raise ValueError(f"Archive member {name} references a missing long-name table")

        offset = int(name[1:])
        end = long_names.find(b"/", offset)
        if end == -1:
            end = long_names.find(b"\n", offset)
        if end == -1:
            end = len(long_names)

        return long_names[offset:end].decode("utf-8", errors="replace").rstrip("/")

    return name.rstrip("/")


def safe_filename(name: str, index: int) -> str:
    base_name = Path(name).name or f"member_{index}.obj"
    safe = "".join(char if char.isalnum() or char in "._-" else "_" for char in base_name)
    return f"{index:04d}_{safe}"


def extract_object_members(archive_path: Path, output_dir: Path) -> list[Path]:
    data = archive_path.read_bytes()
    if not data.startswith(AR_MAGIC):
        raise ValueError(f"{archive_path} is not a standard ar archive")

    objects: list[Path] = []
    long_names: bytes | None = None
    offset = len(AR_MAGIC)
    index = 0

    while offset < len(data):
        if offset + AR_HEADER_SIZE > len(data):
            raise ValueError(f"Truncated ar header at byte {offset}")

        header = data[offset:offset + AR_HEADER_SIZE]
        offset += AR_HEADER_SIZE

        if header[58:60] != b"`\n":
            raise ValueError(f"Invalid ar header terminator at byte {offset - AR_HEADER_SIZE}")

        size = parse_decimal(header[48:58], "member size")
        payload = data[offset:offset + size]
        if len(payload) != size:
            raise ValueError(f"Truncated ar member at byte {offset}")

        name = member_name(header[0:16], long_names)

        if name == "//":
            long_names = payload
        elif name != "/":
            object_path = output_dir / safe_filename(name, index)
            object_path.write_bytes(payload)
            objects.append(object_path)
            index += 1

        offset += size
        if offset % 2:
            offset += 1

    if not objects:
        raise ValueError(f"No object members found in {archive_path}")

    return objects


def repack_archive(archive_path: Path, ar_command: str) -> None:
    ar_path = shutil.which(ar_command)
    if ar_path is None:
        raise FileNotFoundError(f"Could not find ar command: {ar_command}")

    archive_path = archive_path.resolve()
    with tempfile.TemporaryDirectory(prefix="repack-ar-") as tmp:
        tmp_path = Path(tmp)
        objects_dir = tmp_path / "objects"
        objects_dir.mkdir()

        objects = extract_object_members(archive_path, objects_dir)
        repacked = tmp_path / archive_path.name

        subprocess.run([ar_path, "rcs", str(repacked), *map(str, objects)], check=True)
        archive_path.write_bytes(repacked.read_bytes())


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Repack a standard ar archive with the host ar command."
    )
    parser.add_argument("archive", type=Path)
    parser.add_argument("--ar", default="ar", help="ar executable to use when writing")
    args = parser.parse_args()

    try:
        repack_archive(args.archive, args.ar)
    except Exception as exc:
        print(f"repack-ar-archive.py: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
