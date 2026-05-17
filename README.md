# 4JCraft — Minecraft Console Edition (Apple Silicon Port)

Unofficial macOS/Apple Silicon port of **Minecraft: Xbox Edition 1.6** (4J Studios build).  

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
| `F3` | In-game info overlay |

---

## World Types

When creating a new world, the **World Type** button on the Create World screen
cycles through four generators. Each one produces a fundamentally different
landscape:

| Type | Description |
|---|---|
| **Normal** | Default Minecraft Console Edition terrain. Balanced hills, plains, oceans, caves. Recommended starting point. |
| **Superflat** | Endless flat layers of grass / dirt / bedrock. No mountains, no caves, no biomes. Useful for redstone / building tests. |
| **Large Biomes** | Same generator as Normal, but biome regions are scaled up several times. You can walk for a long time without leaving a desert / forest / ocean. |
| **Amplified** | Heavily modified generator: gigantic mountain ranges that can reach the build height limit, deep canyons, frequent rare floating islands with stalactites, snow caps on high stone peaks, large cave entrances cut into mountain sides, tall waterfalls. Designed for cinematic exploration; survival is harder because flat ground is rare. **CPU-intensive** — first chunk batch can stutter on lower-spec machines. |

The selected world type is persisted in the level data and used for both
single-player and multiplayer sessions.

---

## Multiplayer (Experimental)

> ⚠️ **Experimental.** Direct-connect TCP multiplayer is implemented but
> incomplete. It is good enough for two people to share a world and walk
> around, but several systems are still missing or buggy (see *Known
> limitations* below). Use it for testing, not for serious play.

The multiplayer stack is a thin TCP transport layered on top of the original
4J Studios `ClientConnection` / `ServerConnection` packet code. There is **no
matchmaking, no Xbox Live auth, no Realms** — peers exchange data over a raw
TCP socket on **port 25565** (Minecraft's traditional port; chosen for
familiarity, this client is **not** compatible with Java / Bedrock servers).

### How it works

1. The host launches a world. As soon as the world is loaded, a TCP listener
   is started by `PlatformNetworkManagerStub` on port 25565.
2. The client opens a TCP socket to the host's IP:port and sends a
   `LoginPacket`. The server replies with `PreLoginPacket`, accepts the
   login, and starts streaming chunks via `BlockRegionUpdatePacket`.
3. From that point on the connection uses the same packet framing as the
   single-player code path — chat, block updates, entity moves, etc. all
   ride the same `Packet::readPacket` / `writePacket` helpers, just sent
   through the TCP socket instead of the in-process queue used by
   split-screen.
4. Both peers must be running **exactly the same build** of this client —
   `SharedConstants::NETWORK_PROTOCOL_VERSION` is checked for byte-equality
   on login.

### Hosting

Just create or load a world the normal way and host it as an online game from
the world options screen. The TCP listener attaches itself automatically.

To host an **offline** world (useful for quick testing) without going through
the online toggle, set the listener port via an environment variable before
launching:

```bash
cd build/targets/app
MC_LISTEN_PORT=25565 ./Minecraft.Client
```

Find the host's LAN IP with `ifconfig | grep "inet "` (or System Settings →
Network) and share it with the player who wants to join.

### Joining (in-game UI)

1. Click **Play Game** on the title screen.
2. On the world list, press the **Multiplayer (Direct Connect)** button.
3. Type the server address. Both forms are supported:
   - `192.168.1.50` (defaults to port 25565)
   - `192.168.1.50:25577` (custom port)
   - `[::1]:25565` (IPv6)
4. Press **Connect**.

The previously used IP is remembered between launches in `options.txt`
(`lastMpIp`).

### Joining (command line)

For automated tests or quick connect without going through the menus:

```bash
cd build/targets/app
MC_DIRECT_CONNECT=192.168.1.50:25565 ./Minecraft.Client
```

The client skips the title screen and connects directly. All `[TCP] ...`
lines printed to the console come from the multiplayer transport and are
useful when reporting issues.

### Known limitations

These are tracked in `targets/app/common/src/Network/MULTIPLAYER.md`:

- **Dark spawn ring.** Skylight is not recomputed on the client when full
  chunks arrive — chunks render dim until a block update repropagates light.
- **No HUD on the joining client.** The fullscreen progress UI scene is not
  closed automatically because direct-connect bypasses
  `StateChange_AnyToStarting`.
- **Other players and mobs are invisible.** `AddPlayerPacket` is dropped on
  the client because both processes share the same default XUID (no Xbox
  Live profile is loaded).
- **Position correction snaps you back.** The server's authoritative
  player position desynchronises from the client's, so the host's view of
  the joining player keeps teleporting.
- **No reconnect.** Disconnecting and rejoining inside the same process
  has not been tested and will likely require a relaunch.
- **Same build only.** No protocol-version negotiation; both peers must
  be built from the same commit.

### Not supported

Connecting to a **Java Edition** or **Bedrock Edition** server is **not
possible**. This client speaks the original 4J Studios console packet
format (length-prefixed, big-endian, no VarInts, packet IDs as plain
bytes, custom `LoginPacket` carrying XUIDs and 4J-specific fields). The
two protocols share nothing except the default port number. A protocol
bridge would be a project on the order of ViaVersion / Geyser and is
out of scope here.

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
