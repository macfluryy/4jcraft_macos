#!/bin/bash

CC=clang CXX=clang++ meson compile -C build && \
cd build/Minecraft.Client/ && \
gdb -tui ./Minecraft.Client && \
cd ../..
