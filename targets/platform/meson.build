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

sdl2_sources = [
  files('sdl2/Input.cpp',
        'sdl2/Profile.cpp',
        'sdl2/render_stubs.cpp',
        'sdl2/Storage.cpp'),
]

# Select renderer backend: native Metal .mm vs OpenGL .cpp
if get_option('renderer') == 'metal'
  sdl2_sources += files('sdl2/Render_Metal.mm')
else
  sdl2_sources += files('sdl2/Render.cpp')
endif

_platform_deps = [_sdl2, gl_dep, _threads, glm_dep, stb_dep]
if get_option('renderer') == 'metal'
  _platform_deps += metal_dep
endif

lib_platform_sdl2 = static_library('platform_sdl2',
  sdl2_sources,
  include_directories: macos_compat_inc + [platform_inc, include_directories('sdl2'), include_directories('..')],
  dependencies: _platform_deps,
  cpp_args: _defs + global_cpp_args + global_cpp_defs,
  objcpp_args: _defs + global_cpp_defs,
)

# Combined dependency: interfaces + SDL2 implementations
_render_dep_extras = [_sdl2, gl_dep, _threads, glm_dep]
if get_option('renderer') == 'metal'
  _render_dep_extras += metal_dep
endif
render_dep = declare_dependency(
  link_with: lib_platform_sdl2,
  include_directories: macos_compat_inc + [platform_inc, include_directories('sdl2')],
  dependencies: _render_dep_extras,
)
input_dep = render_dep
profile_dep = render_dep
storage_dep = render_dep
