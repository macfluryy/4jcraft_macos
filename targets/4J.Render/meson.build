render_sources = files(
  '4J_Render.cpp',
  'stdafx.cpp',
  'stubs.cpp',
)

lib_render = static_library(
  '4J_Render',
  render_sources,
  include_directories: include_directories('.'),
  dependencies: [sdl2_dep, stb_dep, gl_dep, thread_dep, dependency('glm'), stb_dep],
  cpp_args: global_cpp_args
  + [
    '-fpermissive',
    '-Wshift-count-overflow',
    '-pipe',
    '-include', meson.current_source_dir() / 'stdafx.h',
  ],
)

render_dep = declare_dependency(
  link_with: lib_render,
  include_directories: include_directories('.'),
)