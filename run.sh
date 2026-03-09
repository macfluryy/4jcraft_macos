#!/bin/bash

meson compile -C build && \
cd build/Minecraft.Client/ && \
./Minecraft.Client && \
cd ../..
