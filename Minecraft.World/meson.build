# sources that shouldn't be compiled for whatever reason
exclude_sources = [
  '! -name DurangoStats.cpp', # Durango-specific

  # Incomplete/Unused
  '! -name SkyIslandDimension.cpp',
  '! -name MemoryChunkStorage.cpp',
  '! -name MemoryLevelStorage.cpp',
  '! -name MemoryLevelStorageSource.cpp',
  '! -name NbtSlotFile.cpp',
  '! -name ZonedChunkStorage.cpp',
  '! -name ZoneFile.cpp',
  '! -name ZoneIo.cpp',
]

# GET IT ALLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
# TODO: make this process more portable using a python script :3
world_sources = run_command(
  'sh', '-c',
  'find "'
    + meson.current_source_dir()
    + '" \\( -name "*.cpp" -o -name "*.c" \\) '
    + ' '.join(exclude_sources),
  check : true,
).stdout().strip().split('\n')

lib_world = static_library('Minecraft.World',
  world_sources,
  include_directories : include_directories('Build', 'Build/x64headers'),
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-include', meson.current_source_dir() / 'Build/stdafx.h',
  ],
)

# dep_zlib = dependency('zlib') # using in-tree zlib for the time being
dep_crypto = dependency('libcrypto')  # for MD5 in Hasher.cpp on Linux

world_dep = declare_dependency(
  link_with : lib_world,
  dependencies : [dep_crypto],
  include_directories : include_directories('Build/x64headers'),
)
