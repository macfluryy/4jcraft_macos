platform_inc = include_directories('.')

lib_platform_core = static_library('platform_core',
  files('C4JThread.cpp', 'ShutdownManager.cpp'),
  include_directories: [platform_inc, include_directories('..')],
  dependencies: [dependency('threads')],
  cpp_args: global_cpp_args + global_cpp_defs,
)

platform_dep = declare_dependency(
  link_with: lib_platform_core,
  include_directories: platform_inc,
)

# SDL2-based platform implementations (formerly 4J.* modules)
_sdl2 = dependency('sdl2')
_threads = dependency('threads')
_defs = []

if get_option('renderer') == 'gles'
  _gl = dependency('glesv2', required: true)
  _defs += ['-DGLES']
else
  _gl = dependency('gl', required: true)
endif

sdl2_sources = files(
  'sdl2/Input.cpp',
  'sdl2/Profile.cpp',
  'sdl2/Render.cpp',
  'sdl2/render_stubs.cpp',
  'sdl2/Storage.cpp',
)

lib_platform_sdl2 = static_library('platform_sdl2',
  sdl2_sources,
  include_directories: [platform_inc, include_directories('sdl2')],
  dependencies: [_sdl2, _gl, _threads, glm_dep, stb_dep],
  cpp_args: _defs + global_cpp_args + global_cpp_defs,
)

# Combined dependency: interfaces + SDL2 implementations
# Replaces the old render_dep, input_dep, profile_dep, storage_dep
render_dep = declare_dependency(
  link_with: lib_platform_sdl2,
  include_directories: [platform_inc, include_directories('sdl2')],
  dependencies: [_sdl2, _gl, _threads, glm_dep],
)
input_dep = render_dep
profile_dep = render_dep
storage_dep = render_dep
