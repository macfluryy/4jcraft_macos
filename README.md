# <img src=".github-assets/logo.jpg" alt="Logo" width="50" height="50"> 4JCraft



> Ported version of Minecraft Legacy Console edition to other systems

---

4JCraft is a modified version of the Minecraft Console Legacy Edition aimed on porting old Minecraft to different platforms (such as Linux, Android, Emscripten, etc.)

Join our community:
* Discord: https://discord.gg/zFCwRWkkUg
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
  libpng-dev libpthread-stubs0-dev
```

On Arch/Manjaro:

```bash
sudo pacman -S base-devel gcc pkgconf cmake glfw-x11 mesa openal libvorbis glu
```

If you are on wayland, you may swap `glfw-x11` to `glfw-wayland` for native wayland windowing instead of xwayland.

On Docker:

If you don't want to deal with installing dependencies, you can use the included devcontainer. Open the project in VS Code with the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension and it will set everything up for you — GCC 15, Meson, Ninja, lld, and all the libraries.

Alternatively, you can build and use the container manually:

```bash
docker build -t 4jcraft-dev .devcontainer/
docker run -it -v $(pwd):/workspaces/4jcraft -w /workspaces/4jcraft 4jcraft-dev bash
```

### Configure & Build

> [!IMPORTANT]
> If you are using GCC, then GCC 15 or newer is currently *required* to build this project. Ubuntu installations in particular may have older versions preinstalled, so verify your compiler version with `gcc --version`.

This project uses the [Meson](https://mesonbuild.com/) build system (with [Ninja](https://ninja-build.org/)).

#### Install Tooling

Follow [this Quickstart guide](https://mesonbuild.com/Quick-guide.html) for installing or building Meson and Ninja on your respective distro.

#### Configure & Build

```bash
# 1. Configure a build directory (we'll name it `build`)
meson setup build

# 2. Compile the project
meson compile -C build
```

> [!TIP]
>
> For the fastest compilation speeds, you may want to use the compilers and linkers provided by an [LLVM toolchain](https://llvm.org/) (`clang`/`lld`) over your system compiler and linker. To do this, install `clang` and `lld`, and configure your build using the `llvm_native.txt` nativescript in `/scripts`:
>
> ```bash
> meson setup --native-file ./scripts/llvm_native.txt build
> ```
>
> ...or if you've already configured a build directory:
>
> ```bash
> meson setup --native-file ./scripts/llvm_native.txt build --reconfigure
> ```

The binary is output to:

```
./build/Minecraft.Client/Minecraft.Client
```

#### Clean

To perform a clean compilation:

```bash
meson compile --clean -C build
```

...or to reconfigure an existing build directory:

```bash
meson setup build --reconfigure 
```

...or to hard reset the build directory:

```bash
rm -rf ./build
meson setup build
```

---

## Running

In order to run the compiled binary, you must obtain a copy of a Windows64 build `Common` folder containing compiled media assets. The `Common` folder should be placed at the current working directory when you run the `Minecraft.Client` executable. 

At this time, we don't have the necessary tools to build game resources without Visual Studio, so for the time being these can be obtained from the latest [MinecraftConsoles nightly](https://github.com/smartcmd/MinecraftConsoles/releases/tag/nightly).

1. Download `LCEWindows64.zip` and extract.
2. Copy the `Common` directory to your current working directory.
3. Run `Minecraft.Client`.