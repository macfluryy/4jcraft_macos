# <img src=".github-assets/logo.jpg" alt="Logo" width="50" height="50"> 4JCraft



> Ported version of Minecraft Legacy Console edition to other systems

---

4JCraft is a modified version of the Minecraft Console Legacy Edition aimed on porting old Minecraft to different platforms (such as Linux, Android, Emscripten, etc.)

Join our community:
* Discord (Not currently available): https://discord.gg/zFCwRWkkUg
* Steam: https://steamcommunity.com/groups/4JCraft

## Planned platforms to be supported:
- Linux (~85%)
- Emscripten (not started)
- PS3
- macOS (not started)
- iOS (not started)
- Android (not started)
---

## Building (Linux)

### Dependencies

Install the following packages before building (Debian/Ubuntu names shown):

```bash
sudo apt install \
  build-essential cmake \
  libglfw3-dev libgl-dev libglu1-mesa-dev \
  libopenal-dev libvorbis-dev \
  libpthread-stubs0-dev
```

On Arch/Manjaro:

```bash
sudo pacman -S base-devel gcc pkgconf cmake glfw-x11 mesa openal libvorbis glu
```

If you are on wayland, swap glfw-x11 to glfw-wayland, but its doesn't matter cuz xwayland got yourself covered

### Configure & Build

```bash
# 1. Configure (only needed once, or after CMakeLists changes)
cmake .

# 2. Build (use -jN with your core count)
make -j$(nproc)
```

The binary is output to:

```
Linux/Debug/Minecraft.Client
```

### Clean

```bash
make clean
```

To fully reset the CMake configuration (removes cache + generated Makefiles):

```bash
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile
```
then re-run:
```
cmake . && make -j$(nproc)
```
---

### we also got meson say hi meson hii
[Meson](https://mesonbuild.com/) is bettr pls use over cmake

#### Install Meson

```bash
# Debian/Ubuntu
sudo apt install meson ninja-build

# Arch/Manjaro
sudo pacman -S meson ninja
```

#### Configure & Build

```bash
# 1. Configure that bih
meson setup build_meson

# 2. Build
ninja -C build_meson
```

The binary is output to:

```
build_meson/Minecraft.Client
```

#### Clean

Remove the build directory to do a full clean:

(CAREFUL SPONGEBOB CAREFUL)
```bash
rm -rf build_meson
```

Then re-run `meson setup build_meson` to reconfigure.

#### if u want release which u probably don't need right now
```bash
# Release build
meson setup build_release --buildtype=release
ninja -C build_release
```

---

## Running

(this is temp)
```bash
./Linux/Debug/Minecraft.Client
```

(todo refactor)
(todo refactor more)