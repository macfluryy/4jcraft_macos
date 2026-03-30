# isolate em
_sdl2 = dependency('sdl2')
_threads = dependency('threads')
_glm = dependency('glm')
_defs = []

if get_option('renderer') == 'gles'
  _gl = dependency('glesv2', required: true)
  _defs += ['-DGLES']
else
  _gl = dependency('gl', required: true)
endif

render_sources = files(
  '4J_Render.cpp',
  'stdafx.cpp',
  'stubs.cpp',
)

lib_render = static_library(
  '4J_Render',
  render_sources,
  include_directories: include_directories('.'),
  dependencies: [_sdl2, _gl, _threads, _glm],
  cpp_args: _defs
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