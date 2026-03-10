FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

ENV DEBIAN_FRONTEND=noninteractive

# Dependencies
RUN apt-get update \
    && apt-get install -y software-properties-common \
    && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && apt-get update \
    && sudo apt-get install -y \
        build-essential \
        python3 \
        ninja-build \
        meson \
        libsdl2-dev \
        libgl-dev \
        libglu1-mesa-dev \
        libpthread-stubs0-dev
    # Clean up lol
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

ENV DEBIAN_FRONTEND=dialog