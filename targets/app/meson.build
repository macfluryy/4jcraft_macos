exclude_sources = [
  ' ! -path "*/common/*"',
  ' ! -path "*/linux/*"',
  ' ! -path "*/windows/*"',
]

# all sources except common/, linux/, windows/
client_sources = run_command(
  'sh',
  '-c', 'find "'
  + meson.current_source_dir()
  + '" \\( -name "*.cpp" -o -name "*.c" \\)'
  + ' '.join(exclude_sources),
  check: true,
).stdout().strip().split('\n')

exclude_platform_common_sources = [
  ' ! -name "UIScene_InGameSaveManagementMenu.cpp"',
  # Standalone JavaEdition property tests carry their own main() and are built
  # into their own small test executables below; keep them OUT of the client
  # source glob (mirrors view_distance_util_test in targets/minecraft).
  ' ! -path "*/JavaEdition/tests/*"',
]

# all sources in common/
platform_sources = run_command(
  'sh',
  '-c', 'find "'
  + meson.current_source_dir() / 'common'
  + '" \\( -name "*.cpp" -o -name "*.c" \\)'
  + ' '.join(exclude_platform_common_sources),
  check: true,
).stdout().strip().split('\n')

# linux/macOS shared desktop-backend files
if host_machine.system() in ['linux', 'darwin']
  platform_sources += run_command(
    'sh',
    '-c', 'find "'
    + meson.current_source_dir() / 'mac'
    + '" \\( -name "*.cpp" -o -name "*.c" \\) ',
    check: true,
  ).stdout().strip().split('\n')
endif

client_dependencies = [
  java_dep,
  nbt_dep,
  render_dep,
  input_dep,
  profile_dep,
  storage_dep,
  assets_localisation_dep,
  platform_dep,
  minecraft_dep,
  gl_dep,
  glu_dep,
  thread_dep,
  dl_dep,
  dependency('zlib'),
  miniaudio_dep,
  stb_dep,
  util_dep,
] + macos_compat_dep

if get_option('ui_backend') == 'shiggy'
  shiggy_dep = dependency(
    'shiggy',
    fallback: ['shiggy', 'shiggy_dep'],
  )
  client_dependencies += shiggy_dep
endif

platform_services_src = files('../platform/PlatformServices.cpp')

client = executable(
  'Minecraft.Client',
  client_sources + platform_sources + platform_services_src + localisation[1],
  include_directories: include_directories('include', '..'),
  dependencies: client_dependencies,
  cpp_args: global_cpp_args
  + global_cpp_defs
  + [
    '-DUNICODE',
    '-D_UNICODE',
  ],
  c_args: global_cpp_defs + ['-DUNICODE', '-D_UNICODE'],
  install: true,
  install_dir: '',
)

custom_target(
  'copy_assets_to_client',
  input: [client, media_archive],
  output: 'assets.stamp',
  command: [
    python,
    meson.project_source_root() / 'scripts/copy_assets_to_client.py',
    meson.project_source_root(),
    meson.project_build_root(),
    meson.current_build_dir(),
    '@INPUT1@',
    '@OUTPUT@',
  ],
  build_by_default: true,
)

# --- Standalone property tests for the pure JavaEdition module (Phase 1) -----
# Each test carries its own main() and a tiny deterministic property harness
# (fixed-seed std::mt19937, >=100 iterations), modelled on view_distance_util_test
# in targets/minecraft/meson.build. They compile only the lightweight, pure
# JavaEdition translation units directly (no graphics/network/app globals), so
# they do NOT link the whole client. include_directories('include','..') makes
# the `app/common/src/JavaEdition/...` include paths resolve.

java_address_parser_test = executable(
  'java_address_parser_test',
  [
    'common/src/JavaEdition/tests/JavaAddressParserTest.cpp',
    'common/src/JavaEdition/JavaAddressParser.cpp',
  ],
  include_directories: include_directories('include', '..'),
  cpp_args: global_cpp_args + global_cpp_defs,
  build_by_default: false,
)

test('java_address_parser', java_address_parser_test)

java_server_list_test = executable(
  'java_server_list_test',
  [
    'common/src/JavaEdition/tests/JavaServerListTest.cpp',
    'common/src/JavaEdition/JavaServerList.cpp',
  ],
  include_directories: include_directories('include', '..'),
  cpp_args: global_cpp_args + global_cpp_defs,
  build_by_default: false,
)

test('java_server_list', java_server_list_test)

java_packet_buffer_test = executable(
  'java_packet_buffer_test',
  [
    'common/src/JavaEdition/tests/JavaPacketBufferTest.cpp',
    'common/src/JavaEdition/JavaPacketBuffer.cpp',
  ],
  include_directories: include_directories('include', '..'),
  cpp_args: global_cpp_args + global_cpp_defs,
  build_by_default: false,
)

test('java_packet_buffer', java_packet_buffer_test)

status_json_extractor_test = executable(
  'status_json_extractor_test',
  [
    'common/src/JavaEdition/tests/StatusJsonExtractorTest.cpp',
    'common/src/JavaEdition/StatusJsonExtractor.cpp',
  ],
  include_directories: include_directories('include', '..'),
  cpp_args: global_cpp_args + global_cpp_defs,
  build_by_default: false,
)

test('status_json_extractor', status_json_extractor_test)

# JavaServerListStore performs real file I/O through the project File / stream
# stack, so this test links java_dep (File + FileInputStream / DataOutputStream
# / BufferedReader) and util_dep (StringHelpers include path + symbols). The
# test runs entirely against unique temp working directories and provides its
# own PlatformFileIO stub, so it pulls in no graphics/SDL/app globals.
java_server_list_store_test = executable(
  'java_server_list_store_test',
  [
    'common/src/JavaEdition/tests/JavaServerListStoreTest.cpp',
    'common/src/JavaEdition/JavaServerListStore.cpp',
    'common/src/JavaEdition/JavaServerList.cpp',
  ],
  include_directories: include_directories('include', '..'),
  dependencies: [java_dep, util_dep],
  cpp_args: global_cpp_args + global_cpp_defs,
  build_by_default: false,
)

test('java_server_list_store', java_server_list_store_test)
