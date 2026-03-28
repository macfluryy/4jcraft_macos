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

global_cpp_defs = [
  '-DSPLIT_SAVES',
  '-D_LARGE_WORLDS',
  '-D_EXTENDED_ACHIEVEMENTS',
  '-D_DEBUG_MENUS_ENABLED',
  '-D_DEBUG',
  '-DDEBUG',
]

if host_machine.system() == 'linux'
  global_cpp_defs += [
    '-Dlinux',
    '-D__linux',
    '-D__linux__',
  ]
endif

add_project_arguments(global_cpp_defs, language: ['cpp', 'c'])

global_cpp_args = [
  '-fpermissive',
  '-Wshift-count-overflow',
  '-pipe',
]
add_project_arguments(global_cpp_args, language: 'cpp')

sdl2_dep = dependency('sdl2')
thread_dep = dependency('threads')
dl_dep = cc.find_library('dl', required: true)

if '-DGLES' in global_cpp_defs
  gl_dep = dependency('glesv2')
  glu_dep = dependency('', required: false)
else
  gl_dep = dependency('gl')
  glu_dep = dependency('glu')
endif

stb = subproject('stb').get_variable('stb_inc')
stb_dep = declare_dependency(include_directories: stb)

miniaudio_dep = dependency('miniaudio')

render_dep = dependency('4j-render', fallback: ['4jlibs', 'render_dep'])
input_dep = dependency('4j-input', fallback: ['4jlibs', 'input_dep'])
profile_dep = dependency('4j-profile', fallback: ['4jlibs', 'profile_dep'])
storage_dep = dependency('4j-storage', fallback: ['4jlibs', 'storage_dep'])

all_deps = [
  gl_dep,
  glu_dep,
  sdl2_dep,
  thread_dep,
  dl_dep,
  stb_dep,
  miniaudio_dep,
  render_dep,
  input_dep,
  profile_dep,
  storage_dep,
]

subdir('Minecraft.Assets')
subdir('Minecraft.World')
subdir('Minecraft.Client')