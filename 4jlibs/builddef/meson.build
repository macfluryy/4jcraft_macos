gl_dep = dependency('gl', required: false)
sdl2_dep = dependency('sdl2', required: false)
thread_dep = dependency('threads')

fourj_cpp_args = [
  '-fpermissive',
  '-Wshift-count-overflow',
  '-pipe',
]

fourj_cpp_defs = [
  '-DSPLIT_SAVES',
  '-D_LARGE_WORLDS',
  '-D_EXTENDED_ACHIEVEMENTS',
  '-D_DEBUG_MENUS_ENABLED',
  '-D_DEBUG',
  '-DDEBUG',
]

if host_machine.system() == 'linux'
  fourj_cpp_defs += [
    '-Dlinux',
    '-D__linux',
    '-D__linux__',
  ]
endif

render_inc = include_directories('../../4J.Render', '../../4J.Common')
render_sources = files('../../4J.Render/4J_Render.cpp', '../../4J.Render/stdafx.cpp')
lib_render = static_library(
  '4J_Render',
  render_sources,
  include_directories: render_inc,
  dependencies: [sdl2_dep, gl_dep, thread_dep],
  cpp_args: fourj_cpp_args + fourj_cpp_defs + [
    '-include', meson.current_source_dir() / '../../4J.Render/stdafx.h',
  ],
)
render_dep = declare_dependency(
  link_with: lib_render,
  include_directories: render_inc,
  dependencies: [sdl2_dep, gl_dep, thread_dep],
)

input_inc = include_directories('../../4J.Input', '../../4J.Common')
input_sources = files(
  '../../4J.Input/4J_Input.cpp',
  '../../4J.Input/INP_ForceFeedback.cpp',
  '../../4J.Input/INP_Keyboard.cpp',
  '../../4J.Input/INP_Main.cpp',
  '../../4J.Input/INP_StringCheck.cpp',
  '../../4J.Input/stdafx.cpp',
)
lib_input = static_library(
  '4J_Input',
  input_sources,
  include_directories: input_inc,
  dependencies: [sdl2_dep],
  cpp_args: fourj_cpp_args + fourj_cpp_defs + [
    '-include', meson.current_source_dir() / '../../4J.Input/stdafx.h',
  ],
)
input_dep = declare_dependency(
  link_with: lib_input,
  include_directories: input_inc,
  dependencies: [sdl2_dep],
)

profile_inc = include_directories('../../4J.Profile', '../../4J.Common')
profile_sources = files(
  '../../4J.Profile/4J_Profile.cpp',
  '../../4J.Profile/PRO_AwardManager.cpp',
  '../../4J.Profile/PRO_Data.cpp',
  '../../4J.Profile/PRO_Main.cpp',
  '../../4J.Profile/PRO_RichPresence.cpp',
  '../../4J.Profile/PRO_Sys.cpp',
  '../../4J.Profile/stdafx.cpp',
)
lib_profile = static_library(
  '4J_Profile',
  profile_sources,
  include_directories: profile_inc,
  cpp_args: fourj_cpp_args + fourj_cpp_defs + [
    '-include', meson.current_source_dir() / '../../4J.Profile/stdafx.h',
  ],
)
profile_dep = declare_dependency(
  link_with: lib_profile,
  include_directories: profile_inc,
)

storage_inc = include_directories('../../4J.Storage', '../../4J.Common')
storage_sources = files(
  '../../4J.Storage/4J_Storage.cpp',
  '../../4J.Storage/stdafx.cpp',
  '../../4J.Storage/STO_DLC.cpp',
  '../../4J.Storage/STO_Main.cpp',
  '../../4J.Storage/STO_SaveGame.cpp',
)
lib_storage = static_library(
  '4J_Storage',
  storage_sources,
  include_directories: storage_inc,
  cpp_args: fourj_cpp_args + fourj_cpp_defs + [
    '-include', meson.current_source_dir() / '../../4J.Storage/stdafx.h',
  ],
)
storage_dep = declare_dependency(
  link_with: lib_storage,
  include_directories: storage_inc,
)

meson.override_dependency('4j-render', render_dep)
meson.override_dependency('4j-input', input_dep)
meson.override_dependency('4j-profile', profile_dep)
meson.override_dependency('4j-storage', storage_dep)

if meson.project_name() == '4jlibs'
  header_smoke = executable(
    '4j-header-smoke',
    'header_smoke.cpp',
    build_by_default: false,
  )
  test('4j-header-smoke', header_smoke)
endif
