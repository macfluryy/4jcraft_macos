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
  '! -name "SurvivalMode.cpp"',
  '! -name "CreativeMode.cpp"',
  '! -name "GameMode.cpp"',
  '! -name "DemoMode.cpp"',
]

# GET IT ALLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
# TODO: make this process more portable using a python script :3
minecraft_sources = run_command(
  'sh', '-c',
  'find "'
    + meson.current_source_dir()
    + '" \\( -name "*.cpp" -o -name "*.c" \\) '
    + ' '.join(exclude_sources),
  check : true,
).stdout().strip().split('\n')

occlusion_mode = get_option('occlusion_culling')
if occlusion_mode == 'off'
  global_cpp_defs += ['-DOCCLUSION_MODE_NONE']
elif occlusion_mode == 'frustum'
  global_cpp_defs += ['-DOCCLUSION_MODE_FRUSTUM']
elif occlusion_mode == 'bfs'
  global_cpp_defs += ['-DOCCLUSION_MODE_BFS', '-DUSE_OCCLUSION_CULLING']
elif occlusion_mode == 'hardware'
  global_cpp_defs += ['-DOCCLUSION_MODE_HARDWARE', '-DUSE_OCCLUSION_CULLING']
endif

if get_option('ui_backend') == 'java'
  global_cpp_defs += '-DENABLE_JAVA_GUIS'
endif

lib_minecraft = static_library('minecraft',
  minecraft_sources,
  dependencies : [
    miniaudio_dep, # TODO: remove once a SoundEngine facade is present
    render_dep,
    input_dep,
    profile_dep,
    storage_dep,
    nbt_dep,
    java_dep,
    assets_localisation_dep,
    platform_dep,
    console_helpers_dep,
  ],
  include_directories : include_directories('..'),
  cpp_args : global_cpp_args + global_cpp_defs,
)

zlib_dep = dependency('zlib')
crypto_dep = dependency('libcrypto')  # for MD5 in Hasher.cpp on Linux

minecraft_dep = declare_dependency(
  link_with : lib_minecraft,
  dependencies : [crypto_dep, zlib_dep],
)
