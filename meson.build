project(
  '4jcraft',
  ['cpp', 'c', 'objcpp'],
  version: '0.1.0',
  meson_version: '>= 1.3',
  default_options: [
    'cpp_std=c++23',
    'warning_level=2',
    'buildtype=debugoptimized',
    'unity=on',
    'unity_size=8',
    'b_pch=true',
  ],
)

pymod = import('python')
python = pymod.find_installation('python3', required: true)

cc = meson.get_compiler('cpp')

global_cpp_defs = [
  '-DSPLIT_SAVES',
  '-D_LARGE_WORLDS',
  '-D_EXTENDED_ACHIEVEMENTS',
  '-D_FORTIFY_SOURCE=2',
]

if not get_option('release_build')
  global_cpp_defs += [
    '-D_DEBUG_MENUS_ENABLED',
    '-D_DEBUG',
    '-DDEBUG',
  ]
else
  global_cpp_defs += ['-DNDEBUG']
  add_project_arguments(['-O3'], language: ['cpp', 'c'])
endif

if host_machine.system() == 'linux'
  global_cpp_defs += ['-Dlinux', '-D__linux', '-D__linux__']
endif

if host_machine.system() in ['linux', 'darwin']
  global_cpp_defs += ['-D_PLATFORM_DESKTOP']
endif

if get_option('renderer') == 'gles'
  global_cpp_defs += ['-DGLES']
  gl_dep = dependency('glesv2', required: true)
  glu_dep = dependency('', required: false)
else
  if host_machine.system() == 'darwin'
    gl_dep = dependency('appleframeworks', modules: ['OpenGL'])
    glu_dep = dependency('appleframeworks', modules: ['OpenGL'])
  else
    gl_dep = dependency('gl', required: true)
    glu_dep = dependency('glu', required: true)
  endif
endif

if get_option('enable_vsync')
  global_cpp_defs += ['-DENABLE_VSYNC']
endif

if get_option('classic_panorama')
    global_cpp_defs += '-DCLASSIC_PANORAMA'
endif

if get_option('enable_frame_profiler')
  global_cpp_defs += ['-DENABLE_FRAME_PROFILER']
endif

if get_option('ui_backend') == 'shiggy'
  global_cpp_defs += ['-D_ENABLEIGGY']
endif

if get_option('ui_backend') == 'java'
  global_cpp_defs += '-DENABLE_JAVA_GUIS'
endif

if get_option('srgb_output')
  global_cpp_defs += ['-DSRGB_OUTPUT']
endif

if get_option('srgb_output')
  global_cpp_defs += ['-DSRGB_OUTPUT']
endif

add_project_arguments(global_cpp_defs, language: ['cpp', 'c'])

global_cpp_args = [
  '-Wshift-count-overflow',
  '-pipe',
]
if host_machine.system() == 'darwin'
  global_cpp_args += [
    '-include', meson.source_root() / 'targets/platform/macos_compat/debugbreak.h',
  ]
endif
add_project_arguments(global_cpp_args, language: 'cpp')

sdl2_dep = dependency('sdl2')
thread_dep = dependency('threads')
if host_machine.system() == 'darwin'
  dl_dep = dependency('', required: false)
else
  dl_dep = cc.find_library('dl', required: true)
endif
glm_dep = dependency('glm')

stb = subproject('stb').get_variable('stb_inc')
stb_dep = declare_dependency(include_directories: stb)

miniaudio_dep = dependency('miniaudio')

macos_compat_dep = []
if host_machine.system() == 'darwin'
  macos_compat_dep = [declare_dependency(
    include_directories: include_directories('targets/platform/macos_compat')
  )]
endif

subdir('targets/util')
subdir('targets/java')
subdir('targets/nbt')
subdir('targets/platform')

subdir('targets/resources')
subdir('targets/minecraft')
subdir('targets/app')
