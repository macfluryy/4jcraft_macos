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
  '! -name LevelConflictException.cpp',
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

simdutf_dep = dependency('simdutf', 
  fallback: ['simdutf', 'simdutf_dep'], 
  default_options: ['utf8=true', 'utf16=true', 'utf32=true']
)

lib_world = static_library('Minecraft.World',
  world_sources,
  dependencies : [
    assets_localisation_dep,
    render_dep,
    input_dep,
    profile_dep,
    storage_dep,
    simdutf_dep,
  ],
  include_directories : include_directories('Header Files', '..'),
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-include', 'Header Files/stdafx.h',
  ],
)

zlib_dep = dependency('zlib')
crypto_dep = dependency('libcrypto')  # for MD5 in Hasher.cpp on Linux

world_dep = declare_dependency(
  link_with : lib_world,
  dependencies : [crypto_dep, zlib_dep],
  include_directories : include_directories('x64headers'),
)
