#!/bin/bash

meson setup build -Db_sanitize=address,leak,undefined -Dcpp_args=-fno-sanitize-recover=all -Dc_args=-fno-sanitize-recover=all && \
meson compile -C build && \
cd build/Minecraft.Client/ && \
gdb -tui ./Minecraft.Client && \
cd ../..
