#!/usr/bin/env python3

import os
import shutil
import sys
from pathlib import Path

project_source_root = Path(sys.argv[1])
build_root_dir = Path(sys.argv[2])
client_build_dir = Path(sys.argv[3])
media_archive = Path(sys.argv[4])
output_stamp = Path(sys.argv[5])

# At runtime, the client expects a `Common/` folder from the Minecraft.Assets source and a compiled
# media archive (LinuxMedia.arc) inside of said folder at its current working directory to launch.
#
# `meson install` also handles this, but installs it to system folders, which can be annoying for
# testing. Since we want a way to run it straight from `/build` when debugging, we do this instead.
dest_common = Path(client_build_dir / "Common")

# clear out any old assets
if dest_common.exists():
    shutil.rmtree(dest_common)

# copy `Minecraft.Assets/Common` into the build directory for the client.
shutil.copytree(
    project_source_root / "Minecraft.Assets" / "Common",
    dest_common,
)

# copy the media archive to `Common/Media` inside the folder we just copied.
shutil.copy(media_archive, client_build_dir / "Common" / "Media")

# modify the stamp so this only happens when client or media_archive targets are changed
output_stamp.touch()