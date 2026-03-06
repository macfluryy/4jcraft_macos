render_sources = files(
  '4J_Render.cpp',
  'RendererCbuff.cpp',
  'RendererCore.cpp',
  'RendererMatrix.cpp',
  'RendererState.cpp',
  'RendererTexture.cpp',
  'RendererVertex.cpp',
  'stdafx.cpp',
)

lib_render = static_library('4J_Render',
  render_sources,
  include_directories : include_directories('.'),
  dependencies : [png_dep, glfw_dep, gl_dep, thread_dep],
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-include', meson.current_source_dir() / 'stdafx.h',
  ],
)

render_dep = declare_dependency(
  link_with : lib_render,
  include_directories : include_directories('.'),
)
