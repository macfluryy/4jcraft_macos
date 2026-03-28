exclude_sources = [
  ' ! -path "*/Platform/*"', # added by platform_sources
  ' ! -name "SurvivalMode.cpp"',
  ' ! -name "CreativeMode.cpp"',
  ' ! -name "GameMode.cpp"',
  ' ! -name "DemoMode.cpp"',
]

# all sources except ./Platform/*
client_sources = run_command(
  'sh',
  '-c', 'find "'
  + meson.current_source_dir()
  + '" \\( -name "*.cpp" -o -name "*.c" \\)'
  + ' '.join(exclude_sources),
  check: true,
).stdout().strip().split('\n')

# all sources in ./Platform (top-level files only)
platform_sources = run_command(
  'sh',
  '-c', 'find "'
  + meson.current_source_dir() / 'Platform'
  + '" -maxdepth 1 \\( -name "*.cpp" -o -name "*.c" \\)',
  check: true,
).stdout().strip().split('\n')

# some platform-specific sources that are for some stupid reason in Common
exclude_platform_common_sources = [
  ' ! -path "*/Network/Sony/*"',
  ' ! -path "*/XUI/*"',
  # we use system zlib instead, since this one is old as hell and isn't configured for linux correctly
  ' ! -path "*/zlib/*"',
  ' ! -name "SonyLeaderboardManager.cpp"',
  ' ! -name "UIScene_InGameSaveManagementMenu.cpp"',
]

# all sources in in ./Platform/Common
platform_sources += run_command(
  'sh',
  '-c', 'find "'
  + meson.current_source_dir() / 'Platform/Common'
  + '" \\( -name "*.cpp" -o -name "*.c" \\)'
  + ' '.join(exclude_platform_common_sources),
  check: true,
).stdout().strip().split('\n')

# linux-specific files (everything in Platform/Linux)
if host_machine.system() == 'linux'
  platform_sources += run_command(
    'sh',
    '-c', 'find "'
    + meson.current_source_dir() / 'Platform/Linux'
    + '" \\( -name "*.cpp" -o -name "*.c" \\) ',
    check: true,
  ).stdout().strip().split('\n')
endif

client_dependencies = [
  render_dep,
  input_dep,
  profile_dep,
  storage_dep,
  assets_localisation_dep,
  world_dep,
  gl_dep,
  glu_dep,
  thread_dep,
  dl_dep,
  dependency('zlib'),
  miniaudio_dep,
]

if get_option('enable_vsync')
  global_cpp_defs += ['-DENABLE_VSYNC']
endif

if get_option('classic_panorama')
    global_cpp_defs += '-DCLASSIC_PANORAMA'
endif

if get_option('ui_backend') == 'shiggy'
    shiggy_dep = dependency(
      'shiggy',
      fallback : ['shiggy', 'shiggy_dep'],
    )

  global_cpp_defs += ['-D_ENABLEIGGY']
  client_dependencies += shiggy_dep
endif

if get_option('ui_backend') == 'java'
  global_cpp_defs += '-DENABLE_JAVA_GUIS'
endif

client = executable('Minecraft.Client',
  client_sources + platform_sources + localisation[1],
  include_directories: [include_directories('Platform', 'Platform/Linux/Iggy/include'), stb],
  dependencies: client_dependencies,
  cpp_args: global_cpp_args
  + global_cpp_defs
  + [
    '-DUNICODE',
    '-D_UNICODE',
    '-include', meson.current_source_dir() / 'Platform/stdafx.h',
  ],
  c_args: global_cpp_defs + ['-DUNICODE', '-D_UNICODE'],
  install: true,
  install_dir: '',
)

# To support actually running the client from the build folder, we need to
# copy the generated assets from Minecraft.Assets into the working directory
# of the client.
custom_target(
  'copy_assets_to_client',
  input: [client, media_archive],
  output: 'assets.stamp', # using a stamp file to avoid copying assets every time
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