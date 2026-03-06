# get all those files
_client_build_raw = run_command(
  'sh', '-c',
  'find "' + meson.current_source_dir() / 'Build' + '" -name "*.cpp" ! -path "*/redist64/*"',
  check : true,
).stdout().strip().split('\n')

# linux files
_linux_cpp_sources = files(
  'Platform/Linux/Linux_App.cpp',
  'Platform/Linux/linux_game_stubs.cpp',
  'Platform/Linux/LinuxGL.cpp',
  'Platform/Linux/Linux_Minecraft.cpp',
  'Platform/Linux/Linux_UIController.cpp',
)

_linux_c_sources = files(
  'Platform/Linux/Iggy/gdraw/gdraw_glfw.c',
)

executable('Minecraft.Client',
  _client_build_raw + _linux_cpp_sources + _linux_c_sources,
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
