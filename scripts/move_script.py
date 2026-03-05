#!/usr/bin/env python3
"""
move_and_fix_includes.py

Moves C/C++ files/folders and updates every #include "..." and CMake path
that breaks as a result. Config lives in move_config.json next to this script.

Usage:
  python move_and_fix_includes.py
  python move_and_fix_includes.py --dry-run
  python move_and_fix_includes.py --config path/to/other.json

Move formats:
  { "from": "Foo.cpp", "to": "Util/" }
  { "from": "Foo.*", "to": "Util/" }
  { "from": "Foo.**", "to": "Util/" }
  { "from": "*Packet.*", "to": "Network/Packets/" }
  { "from": "OldName.*", "to": "Util/", "rename": "NewName.*" }
  { "from": "mydir/", "to": "Build/", "folder": "move" }
  { "from": "mydir/", "to": "Build/", "folder": "contents" }
"""

import argparse
import fnmatch
import json
import os
import re
import shutil
from pathlib import Path


INCLUDE_RE      = re.compile(r'(#\s*include\s*")([^"]+)(")')
CMAKE_STRING_RE = re.compile(r'(")([^"\n]+)(")')

SOURCE_EXTS = {".c", ".cpp", ".cc", ".cxx", ".h", ".hpp", ".hh", ".hxx", ".inl"}
CMAKE_NAMES = {"CMakeLists.txt"}
CMAKE_EXTS  = {".cmake"}


def load_config(config_path: Path) -> dict:
    if not config_path.exists():
        print(f"ERROR: config file not found: {config_path}")
        raise SystemExit(1)

    with config_path.open(encoding="utf-8") as f:
        try:
            cfg = json.load(f)
        except json.JSONDecodeError as e:
            print(f"ERROR: bad JSON in {config_path}: {e}")
            raise SystemExit(1)

    cfg.setdefault("project_root", ".")
    cfg.setdefault("moves", [])
    cfg.setdefault("include_search_paths", [])
    cfg.setdefault("extra_scan_paths", [])
    return cfg


def expand_dir_globs(raw: list[str], root: Path) -> list[Path]:
    """
    Resolves a list of path strings into real directories, expanding globs.
    Splits each entry at the first glob character, uses Path.glob() on the
    fixed prefix, and keeps only directories from the results.
    Deduplicates while preserving order.
    """
    result = []

    for entry in raw:
        if "*" not in entry and "?" not in entry:
            p = Path(entry)
            result.append((p if p.is_absolute() else root / p).resolve())
            continue

        parts = Path(entry).parts
        base_parts, pattern_parts, hit_glob = [], [], False
        for part in parts:
            if not hit_glob and "*" not in part and "?" not in part:
                base_parts.append(part)
            else:
                hit_glob = True
                pattern_parts.append(part)

        base = Path(*base_parts) if base_parts else Path("")
        base = base if base.is_absolute() else (root / base).resolve()
        pattern = str(Path(*pattern_parts)) if pattern_parts else "*"

        for match in sorted(base.glob(pattern)):
            if match.is_dir():
                result.append(match.resolve())

    seen, deduped = set(), []
    for p in result:
        if p not in seen:
            seen.add(p)
            deduped.append(p)
    return deduped


def expand_entry(entry: dict, root: Path) -> list[tuple[Path, Path]]:
    """
    Turns one entry from the moves list into concrete (src, dst) path pairs.

    Folder mode resolves the source as a directory. "move" keeps the folder
    intact at the destination; "contents" flattens its files into dst directly.

    Wildcard mode matches files in the source directory by name pattern:
      "Foo.**" matches by name prefix (any number of dot-separated extensions)
      "Foo.*"  matches by stem only (single extension)
      anything else is passed through fnmatch for general glob matching

    The optional "rename" field swaps the destination stem. The matched
    file's extension(s) are kept as-is.
    """
    src_rel     = entry["from"]
    dst_rel     = entry["to"]
    rename_to   = entry.get("rename")
    folder_mode = entry.get("folder")

    # "/" and "./" both mean project root — avoids resolving to filesystem root
    if dst_rel in ("/", "./"):
        dst_rel = ""

    if folder_mode:
        src_dir = (root / src_rel.rstrip("/")).resolve()
        dst_dir = (root / dst_rel.rstrip("/")).resolve()

        if not src_dir.exists() or not src_dir.is_dir():
            print(f"  WARNING: folder not found, skipping: {src_dir}")
            return []

        if folder_mode == "move":
            folder_name = rename_to.rstrip("/") if rename_to else src_dir.name
            return [(src_dir, dst_dir / folder_name)]

        if folder_mode == "contents":
            pairs = []
            for f in sorted(src_dir.rglob("*")):
                if f.is_file():
                    pairs.append((f.resolve(), (dst_dir / f.relative_to(src_dir)).resolve()))
            if not pairs:
                print(f"  WARNING: folder is empty, skipping: {src_dir}")
            return pairs

        print(f"  WARNING: unknown folder mode '{folder_mode}', skipping")
        return []

    src_name = Path(src_rel).name
    src_dir  = Path(src_rel).parent

    if "*" in src_name or "?" in src_name:
        parent = (root / src_dir).resolve()
        if not parent.exists():
            print(f"  WARNING: directory not found for '{src_rel}', skipping")
            return []

        if src_name.endswith(".**"):
            stem    = src_name[:-3]
            matches = sorted(f for f in parent.iterdir()
                             if f.name.startswith(stem + ".") and f.is_file())
        elif src_name.endswith(".*") and "*" not in src_name[:-2]:
            stem    = src_name[:-2]
            matches = sorted(f for f in parent.iterdir()
                             if f.stem == stem and f.is_file())
        else:
            stem    = None
            matches = sorted(f for f in parent.iterdir()
                             if fnmatch.fnmatch(f.name, src_name) and f.is_file())

        if not matches:
            print(f"  WARNING: no files matched '{src_rel}', skipping")
            return []

        rename_stem = None
        if rename_to:
            rename_base = (
                rename_to[:-3] if rename_to.endswith(".**") else
                rename_to[:-2] if rename_to.endswith(".*") else
                rename_to
            )
            rename_stem = Path(rename_base).name

        pairs = []
        for match in matches:
            if rename_stem:
                suffix   = match.name[len(stem):] if (stem and src_name.endswith(".**")) else match.suffix
                new_name = rename_stem + suffix
            else:
                new_name = match.name

            dst_abs = (root / dst_rel.rstrip("/") / new_name).resolve()
            pairs.append((match.resolve(), dst_abs))
            print(f"  Wildcard matched: {match.relative_to(root)}  ->  {dst_abs.relative_to(root)}")

        return pairs

    src_abs = (root / src_rel).resolve()
    dst_abs = (root / dst_rel).resolve()

    if dst_rel.endswith("/") or (dst_abs.exists() and dst_abs.is_dir()):
        name    = (rename_to if rename_to and ".*" not in rename_to else None) or src_abs.name
        dst_abs = dst_abs / name
    elif rename_to and ".*" not in rename_to:
        dst_abs = dst_abs.parent / rename_to

    return [(src_abs, dst_abs)]


def collect_source_files(root: Path) -> list[Path]:
    files = []
    for ext in SOURCE_EXTS:
        files.extend(root.rglob(f"*{ext}"))
    return files


def collect_cmake_files(root: Path) -> list[Path]:
    return [
        f for f in root.rglob("*")
        if f.is_file() and (f.name in CMAKE_NAMES or f.suffix in CMAKE_EXTS)
    ]


def find_file(inc_str: str, relative_to: Path, search_paths: list[Path]) -> Path | None:
    """
    Resolves an include string to an absolute path by checking relative to the
    including file first, then each directory in search_paths in order.
    """
    candidate = (relative_to / inc_str).resolve()
    if candidate.exists():
        return candidate
    for sp in search_paths:
        candidate = (sp / inc_str).resolve()
        if candidate.exists():
            return candidate
    return None


def rel_path(target: Path, relative_to: Path) -> str:
    try:
        return os.path.relpath(target, start=relative_to).replace("\\", "/")
    except ValueError:
        return str(target)


def print_change(filepath: Path, old_val: str, new_val: str, label: str, root: Path, dry_run: bool):
    outside = not filepath.is_relative_to(root)
    display = filepath if outside else filepath.relative_to(root)
    tag = "[DRY RUN] " if dry_run else ""
    tag += "[OUTSIDE-ROOT] " if outside else ""
    print(f"  {tag}{label} in {display}\n    - \"{old_val}\"\n    + \"{new_val}\"")


def rewrite_includes(
        filepath: Path,
        old_loc: Path | None,
        new_loc: Path | None,
        moved_map: dict[Path, Path],
        search_paths: list[Path],
        root: Path,
        dry_run: bool,
) -> int:
    """
    Rewrites #include "..." lines in a source file.

    old_loc / new_loc are the file's old and new absolute paths — only set
    when the file itself is being moved. When set, any include that can't be
    resolved via moved_map is recalculated relative to new_loc so it still
    points at the same file from the new location.

    Returns the number of lines changed.
    """
    try:
        text = filepath.read_text(encoding="utf-8", errors="replace")
    except OSError as e:
        print(f"  ⚠  Could not read {filepath}: {e}")
        return 0

    lines, new_lines, changes = text.splitlines(keepends=True), [], 0

    for line in lines:
        m = INCLUDE_RE.search(line)
        if not m:
            new_lines.append(line)
            continue

        inc_str  = m.group(2)
        base_dir = old_loc.parent if old_loc else filepath.parent
        resolved = find_file(inc_str, base_dir, search_paths)
        new_inc  = inc_str

        if resolved in moved_map:
            ref     = new_loc.parent if new_loc else filepath.parent
            new_inc = rel_path(moved_map[resolved], ref)
        elif old_loc and new_loc and resolved:
            new_inc = rel_path(resolved, new_loc.parent)

        if new_inc != inc_str:
            new_lines.append(line[:m.start(2)] + new_inc + line[m.end(2):])
            changes += 1
            print_change(filepath, inc_str, new_inc, "Updated include", root, dry_run)
        else:
            new_lines.append(line)

    if changes and not dry_run:
        filepath.write_text("".join(new_lines), encoding="utf-8")

    return changes


def rewrite_cmake_paths(
        filepath: Path,
        moved_map: dict[Path, Path],
        search_paths: list[Path],
        root: Path,
        dry_run: bool,
        old_loc: Path | None = None,
        new_loc: Path | None = None,
) -> int:
    """
    Scans a CMake file for quoted strings that resolve to files being moved
    and updates them to the new relative path. Skips comment lines and any
    string that doesn't look like a file path (no dot or slash, or has spaces).

    old_loc / new_loc work the same as in rewrite_includes — set when this
    CMake file is itself being moved, so paths are calculated from new_loc.

    Returns the number of values changed.
    """
    try:
        text = filepath.read_text(encoding="utf-8", errors="replace")
    except OSError as e:
        print(f"  ⚠  Could not read {filepath}: {e}")
        return 0

    lines, new_lines, changes = text.splitlines(keepends=True), [], 0

    for line in lines:
        if line.lstrip().startswith("#"):
            new_lines.append(line)
            continue

        new_line = line
        for m in list(CMAKE_STRING_RE.finditer(line)):
            val = m.group(2)
            if " " in val or ("." not in val and "/" not in val):
                continue
            base_dir = old_loc.parent if old_loc else filepath.parent
            resolved = find_file(val, base_dir, search_paths)
            if resolved not in moved_map and not (old_loc and new_loc and resolved):
                continue
            ref = new_loc.parent if new_loc else filepath.parent
            if resolved in moved_map:
                new_val = rel_path(moved_map[resolved], ref)
            else:
                new_val = rel_path(resolved, ref)
            if new_val == val:
                continue
            new_line = new_line[:m.start(2)] + new_val + new_line[m.end(2):]
            changes += 1
            print_change(filepath, val, new_val, "Updated CMake path", root, dry_run)

        new_lines.append(new_line)

    if changes and not dry_run:
        filepath.write_text("".join(new_lines), encoding="utf-8")

    return changes


def main(config_path: Path, dry_run: bool) -> None:
    cfg          = load_config(config_path)
    root         = Path(cfg["project_root"]).resolve()
    search_paths = expand_dir_globs(cfg["include_search_paths"], root)
    extra_dirs   = expand_dir_globs(cfg["extra_scan_paths"], root)

    if not root.exists():
        print(f"ERROR: project_root does not exist: {root}")
        raise SystemExit(1)

    if not cfg["moves"]:
        print("No moves listed in config.")
        return

    print("\nResolving moves…")
    all_pairs: list[tuple[Path, Path]] = []
    for entry in cfg["moves"]:
        all_pairs.extend(expand_entry(entry, root))

    resolved:  list[tuple[Path, Path]] = []
    seen_srcs: set[Path] = set()

    for src, dst in all_pairs:
        if src in seen_srcs:
            rel = src.relative_to(root) if src.is_relative_to(root) else src
            print(f"  WARNING: duplicate move for {rel}, skipping duplicate")
            continue
        seen_srcs.add(src)

        if src.is_dir():
            rel_src = src.relative_to(root) if src.is_relative_to(root) else src
            rel_dst = dst.relative_to(root) if dst.is_relative_to(root) else dst
            resolved.append((src, dst))
            print(f"  Planned (folder): {rel_src}/  →  {rel_dst}/")
            continue

        if not src.exists():
            print(f"  WARNING: source not found, skipping: {src}")
            continue
        if dst.exists() and dst != src:
            print(f"  WARNING: destination already exists, skipping: {dst}")
            continue

        rel_src = src.relative_to(root) if src.is_relative_to(root) else src
        rel_dst = dst.relative_to(root) if dst.is_relative_to(root) else dst
        resolved.append((src, dst))
        print(f"  Planned: {rel_src}  →  {rel_dst}")

    if not resolved:
        print("Nothing to do.")
        return

    # build old->new map for every file being moved, including files inside moved folders
    moved_map: dict[Path, Path] = {}
    for src, dst in resolved:
        if src.is_dir():
            for f in src.rglob("*"):
                if f.is_file():
                    moved_map[f.resolve()] = (dst / f.relative_to(src)).resolve()
        else:
            moved_map[src] = dst

    source_files = collect_source_files(root)
    extra_files: list[Path] = []
    for d in extra_dirs:
        if not d.is_relative_to(root):
            extra_files.extend(collect_source_files(d))
    all_files = source_files + extra_files

    print(f"\nScanning {len(all_files)} source file(s)"
          f" ({len(extra_files)} outside project root)…\n")

    total_changes = 0
    moved_set     = set(moved_map.keys())

    # rewrite includes inside files that are being moved
    for src, dst in resolved:
        if src.is_dir():
            for f in src.rglob("*"):
                if f.is_file() and f.suffix in SOURCE_EXTS:
                    total_changes += rewrite_includes(
                        f, f, dst / f.relative_to(src), moved_map, search_paths, root, dry_run
                    )
        elif src.suffix in SOURCE_EXTS:
            total_changes += rewrite_includes(
                src, src, dst, moved_map, search_paths, root, dry_run
            )

    # rewrite includes in files that stay put but reference something being moved
    for f in all_files:
        if f.resolve() not in moved_set:
            total_changes += rewrite_includes(
                f, None, None, moved_map, search_paths, root, dry_run
            )

    cmake_files = collect_cmake_files(root)
    for d in extra_dirs:
        if not d.is_relative_to(root):
            cmake_files.extend(collect_cmake_files(d))
    print(f"\nScanning {len(cmake_files)} CMake file(s)…\n")

    # build a lookup so we can pass old/new loc when a cmake file is itself moving
    cmake_move_map = {src: dst for src, dst in resolved if not src.is_dir()}

    for f in cmake_files:
        old_loc = f if f in cmake_move_map else None
        new_loc = cmake_move_map[f] if old_loc else None
        total_changes += rewrite_cmake_paths(
            f, moved_map, search_paths, root, dry_run, old_loc, new_loc
        )

    print(f"\n{'[DRY RUN] ' if dry_run else ''}Moving files…")
    for src, dst in resolved:
        rel_src = src.relative_to(root) if src.is_relative_to(root) else src
        rel_dst = dst.relative_to(root) if dst.is_relative_to(root) else dst
        if dry_run:
            print(f"  [DRY RUN] {rel_src}  →  {rel_dst}")
        else:
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.move(str(src), str(dst))
            print(f"  {rel_src}  →  {rel_dst}")

    print(f"\n{'[DRY RUN] ' if dry_run else ''}Done. "
          f"{len(resolved)} item(s) moved, {total_changes} path(s) updated.")

    if dry_run:
        print("Re-run without --dry-run to apply changes.")


if __name__ == "__main__":
    script_dir = Path(__file__).parent
    parser = argparse.ArgumentParser(
        description="Move C/C++ files/folders and update #include paths."
    )
    parser.add_argument(
        "--config",
        type=Path,
        default=script_dir / "move_config.json",
        help="Path to JSON config (default: move_config.json next to this script)",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Preview changes without touching the disk.",
    )
    args = parser.parse_args()
    main(args.config, args.dry_run)