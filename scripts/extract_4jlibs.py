#!/usr/bin/env python3

from __future__ import annotations

import argparse
import site
import shutil
import subprocess
import sys
from pathlib import Path


PATHS_TO_KEEP = [
    "4J.Common",
    "4J.Input",
    "4J.Profile",
    "4J.Render",
    "4J.Storage",
    "4jlibs",
]


def run(cmd: list[str], cwd: Path | None = None) -> None:
    subprocess.run(cmd, cwd=cwd, check=True)


def require_clean_destination(dest: Path) -> None:
    if dest.exists():
        raise SystemExit(f"Destination already exists: {dest}")


def resolve_filter_repo_command() -> list[str]:
    script_name = "git-filter-repo.exe" if sys.platform == "win32" else "git-filter-repo"

    path_candidates = [
        shutil.which("git-filter-repo"),
        shutil.which(script_name),
    ]

    user_script_dirs = [
        Path(site.USER_BASE) / "Scripts",
        Path(site.getusersitepackages()).parent / "Scripts",
    ]

    for script_dir in user_script_dirs:
        user_scripts = script_dir / script_name
        path_candidates.append(str(user_scripts) if user_scripts.exists() else None)

    for candidate in path_candidates:
        if candidate:
            return [candidate]

    raise SystemExit(
        "git-filter-repo is required. Install it first and rerun this script."
    )


def clone_source(source: Path, dest: Path) -> None:
    run(["git", "clone", "--no-local", str(source), str(dest)])


def filter_history(dest: Path) -> None:
    cmd = resolve_filter_repo_command() + ["--force"]
    for path in PATHS_TO_KEEP:
        cmd.extend(["--path", path])
    run(cmd, cwd=dest)


def rewrite_staged_layout(dest: Path) -> None:
    staged_root = dest / "4jlibs"
    meson_src = staged_root / "meson.build"
    builddef_src = staged_root / "builddef"

    if not meson_src.exists() or not builddef_src.exists():
        raise SystemExit("Filtered tree is missing the staged 4jlibs Meson files.")

    shutil.move(str(meson_src), str(dest / "meson.build"))
    shutil.move(str(builddef_src), str(dest / "builddef"))

    try:
        staged_root.rmdir()
    except OSError:
        pass

    builddef_meson = dest / "builddef" / "meson.build"
    builddef_smoke = dest / "builddef" / "header_smoke.cpp"

    builddef_meson.write_text(
        builddef_meson.read_text(encoding="utf-8").replace("../../", "../"),
        encoding="utf-8",
    )
    builddef_smoke.write_text(
        builddef_smoke.read_text(encoding="utf-8").replace("../../", "../"),
        encoding="utf-8",
    )


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Create a filtered 4jlibs bootstrap repo from the current 4jcraft tree."
        )
    )
    parser.add_argument(
        "destination",
        type=Path,
        help="Path to create the filtered 4jlibs clone in",
    )
    parser.add_argument(
        "--source",
        type=Path,
        default=Path.cwd(),
        help="Source 4jcraft repo to clone from (default: current working directory)",
    )
    args = parser.parse_args()

    source = args.source.resolve()
    destination = args.destination.resolve()

    require_clean_destination(destination)
    clone_source(source, destination)
    filter_history(destination)
    rewrite_staged_layout(destination)

    print(f"Created filtered 4jlibs bootstrap repo at: {destination}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
