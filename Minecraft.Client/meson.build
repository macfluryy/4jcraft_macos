# all sources except ./Platform/*
client_sources = run_command(
  'sh', '-c',
  'find "'
    + meson.current_source_dir()
    + '" \\( -name "*.cpp" -o -name "*.c" \\) ! -path "*/Platform/*"',
  check : true,
).stdout().strip().split('\n')

# all sources in ./Platform (top-level files only)
platform_sources = run_command(
  'sh', '-c',
  'find "'
    + meson.current_source_dir() / 'Platform'
    + '" -maxdepth 1 \\( -name "*.cpp" -o -name "*.c" \\)',
  check : true,
).stdout().strip().split('\n')

# some platform-specific sources that are for some stupid reason in Common
exclude_platform_common_sources = [
  ' ! -path "*/Network/Sony/*"',
  ' ! -path "*/XUI/*"',
  # we use system zlib instead, since this one is old as hell and isn't configured for linux correctly
  ' ! -path "*/zlib/*"',
]

# all sources in in ./Platform/Common
platform_sources += run_command(
  'sh', '-c',
  'find "'
    + meson.current_source_dir() / 'Platform/Common'
    + '" \\( -name "*.cpp" -o -name "*.c" \\)'
    + ' '.join(exclude_platform_common_sources),
  check : true,
).stdout().strip().split('\n')

# linux-specific files (everything in Platform/Linux)
linux_objects = []
if host_machine.system() == 'linux'
  platform_sources += run_command(
    'sh', '-c',
    'find "'
      + meson.current_source_dir() / 'Platform/Linux'
      + '" \\( -name "*.cpp" -o -name "*.c" \\) ',
      check : true,
  ).stdout().strip().split('\n')

  _linux_objects_str = run_command(
    'sh', '-c',
    'find "'
      + meson.current_source_dir() / 'Platform/Linux'
      + '" -name "*.o"',
    check : true,
  ).stdout().strip()
  if _linux_objects_str != ''
    linux_objects += _linux_objects_str.split('\n')
  endif
endif

client = executable('Minecraft.Client',
  client_sources + platform_sources + localisation[1],
  objects : linux_objects,
  include_directories : include_directories('Platform', 'Platform/Linux/Iggy/include'),
  dependencies : [
    render_dep,
    input_dep,
    profile_dep,
    storage_dep,
    assets_localisation_dep,
    world_dep,
    gl_dep,
    glu_dep,
    thread_dep,
    thread_dep,
    dependency('zlib'),
  ],
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-DUNICODE', '-D_UNICODE',
    '-include', meson.current_source_dir() / 'Platform/stdafx.h',
  ],
  c_args   : global_cpp_defs + ['-DUNICODE', '-D_UNICODE'],
  install  : true,
  install_dir : ''
)

# To support actually running the client from the build folder, we need to
# copy the generated assets from Minecraft.Assets into the working directory
# of the client.
custom_target('copy_assets_to_client',
  input: [client, media_archive],
  output: 'assets.stamp', # using a stamp file to avoid copying assets every time
  command : [
    python, meson.project_source_root() / 'scripts/copy_assets_to_client.py',
    meson.project_source_root(),
    meson.project_build_root(),
    meson.current_build_dir(),
    '@INPUT1@',
    '@OUTPUT@',
  ],
  build_by_default: true,
)
