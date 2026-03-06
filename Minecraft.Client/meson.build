# sources that shouldn't be compiled for whatever reason
exclude_sources = [
  ' ! -path "*/Platform/*"',
  ' ! -path "*/Build/Common/Network/Sony/*"',
  ' ! -path "*/Build/Common/XUI/*"',
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
  client_sources + platform_sources,
  include_directories : include_directories('Build'),
  dependencies : [
    render_dep,
    input_dep,
    profile_dep,
    storage_dep,
    world_dep,
    gl_dep,
    glu_dep,
    glfw_dep,
    thread_dep,
    dl_dep,
  ],
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-DUNICODE', '-D_UNICODE',
    '-include', meson.current_source_dir() / 'Build/stdafx.h',
  ],
  c_args   : global_cpp_defs + ['-DUNICODE', '-D_UNICODE'],
  install  : false,
)
