# 4JCraft — Minecraft Console Edition (Apple Silicon Port)

Unofficial macOS/Apple Silicon port of **Minecraft: Xbox Edition 1.6** (4J Studios build).  
Tested on **MacBook Air 13" M4 (2025), 16 GB RAM, 512 GB SSD**.

---

## Requirements

| Tool | Version | Install |
|------|---------|---------|
| Xcode Command Line Tools | latest | `xcode-select --install` |
| Homebrew | latest | [brew.sh](https://brew.sh) |
| Python 3 | ≥ 3.9 | bundled with macOS / Homebrew |
| Meson | ≥ 1.1.0 | `brew install meson` |
| Ninja | latest | `brew install ninja` |
| SDL2 | latest | `brew install sdl2` |
| GLM | latest | `brew install glm` |
| pkg-config | latest | `brew install pkg-config` |

> **Note:** You do **not** need `llvm`, `cmake`, or any Java runtime.  
> The archive extraction step uses a built-in Python script — no GNU binutils required.

---

## Build

```bash
# 1. Clone the repository
git clone https://github.com/macfluryy/4jcraft_macos.git
cd 4jcraft_macos

# 2. Create a Python virtual environment (used by Meson/Ninja scripts)
python3 -m venv .venv
source .venv/bin/activate
pip install meson ninja

# 3. Configure the build
#    -Dui_backend=java: Required for macOS (uses Mach-O compatible UI)
#    -Dclassic_panorama=false: Use modern main menu background
#    -Drenderer=gl3: Use OpenGL 3.3 Core (optimal for macOS Metal translation)
#    -Denable_vsync=false: Disable VSync to prevent 60->30 FPS drops
#    -Denable_frame_profiler=false: Disable debug performance overlay
#    -Docclusion_culling=hardware: Use GPU for better performance
#    -Drelease_build=true: Maximum optimization (-O3)
meson setup build \
  -Dui_backend=java \
  -Dclassic_panorama=false \
  -Drenderer=gl3 \
  -Denable_vsync=false \
  -Denable_frame_profiler=false \
  -Docclusion_culling=hardware \
  -Drelease_build=true

# 4. Compile
meson compile -C build
```

Build output: `build/targets/app/Minecraft.Client`

---

## Advanced Build Options

You can customize the build by passing `-Doption=value` to `meson setup`.

### Occlusion Culling (`-Docclusion_culling`)
Controls how the game hides non-visible geometry (like blocks behind walls) to save performance:
- `off`: Disables all culling. Draws everything (debug only).
- `frustum`: (Default) Only draws objects inside the camera's field of view.
- `bfs`: Experimental connectivity-based culling (Breadth-First Search).
- `hardware`: **(Recommended)** Uses GPU queries to hide obscured blocks. Provides the best FPS on macOS.

### Release Build (`-Drelease_build`)
- `false`: (Default) Debug mode. Includes internal menus, debug symbols, and slower code.
- `true`: **(Recommended for play)** Enables `-O3` maximum optimizations, removes debug overlays, and disables `assert()` calls for peak performance.

---

## Run

```bash
cd build/targets/app
./Minecraft.Client
```

The game window title shows real-time FPS and renderer info, e.g.:  
`Minecraft Console Edition | 60 FPS | GL 4.1 Metal | macOS arm64`

---

## Controls

| Key / Action | In-game function |
|---|---|
| `W A S D` | Move |
| Mouse | Look around |
| Left click | Break block |
| Right click | Place block / use item |
| `E` | Open inventory |
| `Esc` | Pause / back |
| `1–9` | Hotbar slots |
| `Tab` | Toggle map (if available) |
| `F1` | Hide HUD |
| `F3` | In-game info overlay |

---

## Troubleshooting

### Build fails with `ar: //: File exists`

macOS's built-in `ar` cannot extract ORBIS (PS4) archives.  
**This is already fixed** in this repo via a pure-Python extractor (`subprojects/shiggy/scripts/unpack_archive.py`).  
If you see this error, make sure you pulled the latest code:

```bash
git pull origin dev
meson setup --reconfigure build
meson compile -C build
```

### Build fails with `SDL2 not found`

```bash
brew install sdl2
```

### Game crashes on launch

Make sure you run the binary **from its directory** so it can find the asset files:

```bash
cd build/targets/app && ./Minecraft.Client
```

### No sound

miniaudio backend is included. If you get no audio, check System Settings → Privacy → Microphone.

---

## Tested Hardware

| Device | Result |
|---|---|
| MacBook Air 13" M4 (2025) 16 GB | ✅ 60 FPS, fully playable |

Community reports for M1/M2/M3 welcome — open an issue!

---

## License & Credits

- Original game: © Mojang / Microsoft  
- Console port: © 4J Studios  
- macOS port: community effort — see commit history  
- This repository contains no proprietary game assets
