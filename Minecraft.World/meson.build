# GET IT ALLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
# Note: the files below are commented-out in CMakeLists.txt (missing headers or
# unfinished ports) and must be excluded from the build.
_world_sources_raw = run_command(
  'sh', '-c',
  'find "' + meson.current_source_dir() + '" -name "*.cpp"' +
  ' ! -name "DurangoStats.cpp"' +
  ' ! -name "MemoryChunkStorage.cpp"' +
  ' ! -name "MemoryLevelStorage.cpp"' +
  ' ! -name "MemoryLevelStorageSource.cpp"' +
  ' ! -name "NbtSlotFile.cpp"' +
  ' ! -name "SkyIslandDimension.cpp"' +
  ' ! -name "ZonedChunkStorage.cpp"' +
  ' ! -name "ZoneFile.cpp"' +
  ' ! -name "ZoneIo.cpp"',
  check : true,
).stdout().strip().split('\n')

lib_world = static_library('Minecraft.World',
  _world_sources_raw,
  include_directories : include_directories('Build', 'Build/x64headers'),
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-include', meson.current_source_dir() / 'Build/stdafx.h',
  ],
)

world_dep = declare_dependency(
  link_with : lib_world,
  include_directories : include_directories('Build/x64headers'),
)
