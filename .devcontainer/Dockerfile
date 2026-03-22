FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

ENV DEBIAN_FRONTEND=noninteractive

# Dependencies
RUN apt-get update \
    && apt-get install -y software-properties-common \
    && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && apt-get update \
    && apt-get install -y \
        build-essential \
        gcc-14 g++-14 \
        python3 \
        ninja-build \
        libsdl2-dev \
        libgl-dev \
        libglu1-mesa-dev \
        libpthread-stubs0-dev \
    # Set GCC 14 as the default compiler
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100 \
    # Clean up lol
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Use the latest version of Meson -> $(curl -s "https://api.github.com/repos/mesonbuild/meson/releases/latest" | grep -Po '"tag_name": "\K[0-9.]+')
RUN MESON_VERSION=1.10.2 \
    && wget -qO meson.tar.gz https://github.com/mesonbuild/meson/releases/latest/download/meson-${MESON_VERSION}.tar.gz \
    && mkdir /opt/meson \
    && tar xf meson.tar.gz --strip-components=1 -C /opt/meson \
    && mv /opt/meson/meson.py /opt/meson/meson \
    && ln -s /opt/meson/meson /usr/bin/meson \
    && rm -rf meson.tar.gz

ENV DEBIAN_FRONTEND=dialog
