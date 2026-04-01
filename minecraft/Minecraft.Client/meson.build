exclude_sources = [
  ' ! -path "*/Common/*"', # added by platform_sources
  ' ! -path "*/Linux/*"', # added by platform_sources
  ' ! -path "*/Windows64/*"', # added by platform_sources
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

# some platform-specific sources that are for some stupid reason in Common
exclude_platform_common_sources = [
  ' ! -name "UIScene_InGameSaveManagementMenu.cpp"',
]

# all sources in in ./Platform/Common
platform_sources = run_command(
  'sh',
  '-c', 'find "'
  + meson.current_source_dir() / 'Common'
  + '" \\( -name "*.cpp" -o -name "*.c" \\)'
  + ' '.join(exclude_platform_common_sources),
  check: true,
).stdout().strip().split('\n')

# linux-specific files (everything in Platform/Linux)
if host_machine.system() == 'linux'
  platform_sources += run_command(
    'sh',
    '-c', 'find "'
    + meson.current_source_dir() / 'Linux'
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
  console_helpers_dep,
]

if get_option('enable_vsync')
  global_cpp_defs += ['-DENABLE_VSYNC']
endif

if get_option('classic_panorama')
    global_cpp_defs += '-DCLASSIC_PANORAMA'
endif

if get_option('enable_frame_profiler')
  global_cpp_defs += ['-DENABLE_FRAME_PROFILER']
endif

if get_option('ui_backend') == 'shiggy'
  shiggy_dep = dependency(
    'shiggy',
    fallback: ['shiggy', 'shiggy_dep'],
  )

  global_cpp_defs += ['-D_ENABLEIGGY']
  client_dependencies += shiggy_dep
endif

if get_option('ui_backend') == 'java'
  global_cpp_defs += '-DENABLE_JAVA_GUIS'
endif

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
