# <img src=".github-assets/logo.jpg" alt="Logo" width="50" height="50"> 4JCraft



> Ported version of Minecraft Legacy Console edition to other systems

---

4JCraft is a modified version of the Minecraft Console Legacy Edition aimed on porting old Minecraft to different platforms (such as Linux, Android, Emscripten, etc.) and refactoring the codebase to improve organization and use modern C++ features.

Join our community:
* Discord: https://discord.gg/zFCwRWkkUg
* Steam: https://steamcommunity.com/groups/4JCraft

## Scope & Platform Support

At the moment, we're aiming to support the following platforms:

- Linux (~85%, *rough estimate*)
- Emscripten (not started)
- macOS (not started)
- iOS (not started)
- Android (not started)

> All efforts are focused towards a native Linux port, OpenGL rendering pipeline, and modernizing the existing LCE codebase/tooling to make future platform ports easier.
> 
> `Windows64` and other platforms originally supported by LCE are currently unsupported, since the original Visual Studio tooling has been stripped from this repository and replaced with our own.

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

If you don't want to deal with installing dependencies, you can use the included devcontainer. Open the project in VS Code with the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension and it will set everything up for you - GCC 15, Meson, Ninja, lld, and all the libraries.

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

## whoever downloaded Common pls refetch & pull this there are updates
