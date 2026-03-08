# sources that shouldn't be compiled for whatever reason
exclude_sources = [
  ' ! -path "*/Platform/*"',
  ' ! -path "*/../Common/Network/Sony/*"',
  ' ! -path "*/../Common/XUI/*"',
  ' ! -path "*/../Common/zlib/*"',
]

# get all those files
client_sources = run_command(
  'sh', '-c',
  'find "'
    + meson.current_source_dir()
    + '" \\( -name "*.cpp" -o -name "*.c" \\) '
    + ' '.join(exclude_sources),
  check : true,
).stdout().strip().split('\n')

# Common/ sources (previously lived in Minecraft.Client/Build/Common/)
common_exclude = [
  ' ! -path "*/Network/Sony/*"',
  ' ! -path "*/XUI/*"',
  ' ! -path "*/zlib/*"',
  ' ! -path "*/Media/*"',
  ' ! -path "*/DummyTexturePack/*"',
]

common_sources = run_command(
  'sh', '-c',
  'find "'
    + meson.current_source_dir() / '../Common'
    + '" \\( -name "*.cpp" -o -name "*.c" \\) '
    + ' '.join(common_exclude),
  check : true,
).stdout().strip().split('\n')

platform_sources = []

# linux-specific files (everything in Platform/Linux)
if host_machine.system() == 'linux'
  platform_sources += run_command(
    'sh', '-c',
    'find "'
      + meson.current_source_dir() / 'Platform/Linux'
      + '" \\( -name "*.cpp" -o -name "*.c" \\) ',
    check : true,
  ).stdout().strip().split('\n')
endif

executable('Minecraft.Client',
  client_sources + platform_sources + common_sources + localisation[1],
  include_directories : include_directories('Build'),
  dependencies : [
    render_dep,
    input_dep,
    profile_dep,
    storage_dep,
    assets_localisation_dep,
    world_dep,
    gl_dep,
    glu_dep,
    glfw_dep,
    thread_dep,
    dl_dep,
    dependency('zlib'),
  ],
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-DUNICODE', '-D_UNICODE',
    '-include', meson.current_source_dir() / 'Build/stdafx.h',
  ],
  c_args   : global_cpp_defs + ['-DUNICODE', '-D_UNICODE'],
  install  : true,
  install_dir : ''
)

