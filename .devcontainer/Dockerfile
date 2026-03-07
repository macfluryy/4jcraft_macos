FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

ENV DEBIAN_FRONTEND=noninteractive

# Dependencies
RUN apt-get update \
    && apt-get install -y software-properties-common \
    && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && apt-get update \
    && apt-get install -y \
        build-essential \
        cmake \
        libglfw3-dev \
        libgl-dev \
        libglu1-mesa-dev \
        libopenal-dev \
        libvorbis-dev \
        libpng-dev \
        libpthread-stubs0-dev \
        clang \
        lld \
        meson \
        ninja-build \
        gcc-15 \
        g++-15 \
    # Clean up lol
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Set GCC 15 as default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-15 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-15 100 \
    && update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 100 \
    && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100

ENV DEBIAN_FRONTEND=dialog