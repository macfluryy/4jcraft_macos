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

  # Standalone property tests, built into their own test executable below
  # (see view_distance_util_test). Must not be compiled into lib_minecraft.
  '! -name "ViewDistanceUtilTest.cpp"',
  # (see login_packet_view_distance_test). Must not be compiled into
  # lib_minecraft (it ships its own main()).
  '! -name "LoginPacketViewDistanceTest.cpp"',
  # (see client_information_packet_test). Must not be compiled into
  # lib_minecraft (it ships its own main()).
  '! -name "ClientInformationPacketTest.cpp"',
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

# macOS native frameworks for clipboard operations (Cocoa Pasteboard API)
macos_frameworks_dep = []
macos_mm_sources = []
if host_machine.system() == 'darwin'
  macos_frameworks_dep = [dependency('appleframeworks', modules: ['Cocoa'])]
  macos_mm_sources = ['client/gui/ClipboardMac.mm']
endif

lib_minecraft = static_library('minecraft',
  minecraft_sources + macos_mm_sources,
  dependencies : [
    miniaudio_dep, # TODO: remove once a SoundEngine facade is present
    render_dep,
    input_dep,
    profile_dep,
    storage_dep,
    glm_dep,
    nbt_dep,
    java_dep,
    assets_localisation_dep,
    platform_dep,
    util_dep,
    dependency('zlib'),
  ] + macos_compat_dep + macos_frameworks_dep,
  include_directories : include_directories('..'),
  cpp_args : global_cpp_args + global_cpp_defs,
)

zlib_dep = dependency('zlib')
crypto_dep = dependency('libcrypto')  # for MD5 in Hasher.cpp on Linux

minecraft_dep = declare_dependency(
  link_with : lib_minecraft,
  dependencies : [crypto_dep, zlib_dep],
)

# --- Property tests for pure view-distance helpers (task 1.2) -------------
# Built as a small standalone executable that compiles ViewDistanceUtil.cpp
# directly (rather than linking the whole lib_minecraft, which would drag in
# heavy graphics/network deps). ViewDistanceUtil.cpp only needs the lightweight
# PlayerChunkMap.h MIN/MAX constants and ChunkPos/JavaIntHash headers.
view_distance_util_test = executable(
  'view_distance_util_test',
  [
    'world/level/tests/ViewDistanceUtilTest.cpp',
    'world/level/ViewDistanceUtil.cpp',
  ],
  include_directories : include_directories('..'),
  dependencies : [java_dep],
  cpp_args : global_cpp_args + global_cpp_defs,
  build_by_default : false,
)

test('view_distance_util', view_distance_util_test)

# --- Property test for LoginPacket serverViewDistance round-trip (task 3.2) -
# Built as a small standalone executable. The real LoginPacket couples to the
# `app` global (MacGame, defined in the app target) plus LevelType/Player, so
# compiling LoginPacket.cpp here would create unresolved symbols and drag in
# the client stack. Instead this test reproduces LoginPacket::write()'s exact
# wire field sequence using the lightweight java stream classes (java_dep) and
# compiles ViewDistanceUtil.cpp directly for clampViewDistance. The live
# end-to-end login round-trip is covered by the task 10 integration checklist.
login_packet_view_distance_test = executable(
  'login_packet_view_distance_test',
  [
    'network/packet/tests/LoginPacketViewDistanceTest.cpp',
    'world/level/ViewDistanceUtil.cpp',
  ],
  include_directories : include_directories('..'),
  dependencies : [java_dep],
  cpp_args : global_cpp_args + global_cpp_defs,
  build_by_default : false,
)

test('login_packet_view_distance', login_packet_view_distance_test)

# --- Property test for ClientInformationPacket viewDistance round-trip ------
# (task 6.2). Built as a small standalone executable. The real
# ClientInformationPacket derives from Packet, whose Packet.cpp staticCtor()
# references every registered packet type plus the `app` global, so compiling
# ClientInformationPacket.cpp here would create unresolved symbols and drag in
# the client/network stack. Instead this test reproduces
# ClientInformationPacket::write()'s exact single-int wire sequence using the
# lightweight java stream classes (java_dep). The live end-to-end packet
# round-trip is covered by the task 10 integration checklist.
client_information_packet_test = executable(
  'client_information_packet_test',
  [
    'network/packet/tests/ClientInformationPacketTest.cpp',
  ],
  include_directories : include_directories('..'),
  dependencies : [java_dep],
  cpp_args : global_cpp_args + global_cpp_defs,
  build_by_default : false,
)

test('client_information_packet', client_information_packet_test)
