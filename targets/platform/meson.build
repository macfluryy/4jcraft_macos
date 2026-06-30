platform_inc = include_directories('.')
_threads = dependency('threads')

# On macOS, prepend a shim directory so that GL/gl.h -> OpenGL/gl.h, etc.
macos_compat_inc = []
if host_machine.system() == 'darwin'
  macos_compat_inc = [include_directories('macos_compat')]
endif

lib_platform_core = static_library('platform_core',
  files('C4JThread.cpp', 'ShutdownManager.cpp'),
  include_directories: macos_compat_inc + [platform_inc, include_directories('..')],
  dependencies: [_threads],
  cpp_args: global_cpp_args + global_cpp_defs,
)

platform_dep = declare_dependency(
  link_with: lib_platform_core,
  include_directories: platform_inc,
)

# SDL2-based platform implementations (formerly 4J.* modules)
_sdl2 = dependency('sdl2')
_defs = []

sdl2_sources = files(
  'sdl2/Clipboard.cpp',
  'sdl2/Input.cpp',
  'sdl2/Profile.cpp',
  'sdl2/Render.cpp',
  'sdl2/render_stubs.cpp',
  'sdl2/Storage.cpp',
)

lib_platform_sdl2 = static_library('platform_sdl2',
  sdl2_sources,
  include_directories: macos_compat_inc + [platform_inc, include_directories('sdl2')],
  dependencies: [_sdl2, gl_dep, _threads, glm_dep, stb_dep],
  cpp_args: _defs + global_cpp_args + global_cpp_defs,
)

# Combined dependency: interfaces + SDL2 implementations
# Replaces the old render_dep, input_dep, profile_dep, storage_dep
render_dep = declare_dependency(
  link_with: lib_platform_sdl2,
  include_directories: macos_compat_inc + [platform_inc, include_directories('sdl2')],
  dependencies: [_sdl2, gl_dep, _threads, glm_dep],
)
input_dep = render_dep
profile_dep = render_dep
storage_dep = render_dep
