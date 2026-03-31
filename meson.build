project(
  '4jcraft',
  ['cpp', 'c'],
  version: '0.1.0',
  meson_version: '>= 1.7',
  default_options: [
    'cpp_std=c++23',
    'warning_level=0',
    'buildtype=debug', # for now
    'unity=on', # merge source files per target
    'unity_size=8', # TODO: mess around with this
    'b_pch=true', # precompiled headers
  ],
)

pymod = import('python')
python = pymod.find_installation('python3', required: true)

cc = meson.get_compiler('cpp')

use_gles = get_option('renderer') == 'gles'

global_cpp_defs = [
  '-DSPLIT_SAVES',
  '-D_LARGE_WORLDS',
  '-D_EXTENDED_ACHIEVEMENTS',
  '-D_DEBUG_MENUS_ENABLED',
  '-D_DEBUG',
  '-DDEBUG',
]

if host_machine.system() == 'linux'
  global_cpp_defs += ['-Dlinux', '-D__linux', '-D__linux__']
endif

if use_gles
  global_cpp_defs += ['-DGLES']
endif

add_project_arguments(global_cpp_defs, language: ['cpp', 'c'])

global_cpp_args = [
  '-Wshift-count-overflow',
  '-pipe',
]
add_project_arguments(global_cpp_args, language: 'cpp')

sdl2_dep = dependency('sdl2')
thread_dep = dependency('threads')
dl_dep = cc.find_library('dl', required: true)

# GLES vs Desktop GL
if use_gles
  gl_dep = dependency('glesv2', required: true)
  glu_dep = dependency('', required: false)
else
  gl_dep = dependency('gl', required: true)
  glu_dep = dependency('glu', required: true)
endif

stb = subproject('stb').get_variable('stb_inc')
stb_dep = declare_dependency(include_directories: stb)

miniaudio_dep = dependency('miniaudio')

subdir('minecraft/java')
subdir('minecraft/nbt')
subdir('minecraft/platform')

subdir('minecraft/4J.Render')
subdir('minecraft/4J.Input')
subdir('minecraft/4J.Profile')
subdir('minecraft/4J.Storage')
subdir('minecraft/Minecraft.Assets')
subdir('minecraft/Minecraft.World')
subdir('minecraft/Minecraft.Client')

