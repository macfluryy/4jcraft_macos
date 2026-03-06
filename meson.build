project('4jcraft-chucklegrounds', ['cpp', 'c'],
  version : '0.1.0',
  default_options : [
    'cpp_std=gnu++11',
    'warning_level=0',
    'buildtype=debug',
  ],
)

cc = meson.get_compiler('cpp')

# system deps
gl_dep     = dependency('gl')
glu_dep    = dependency('glu')
glfw_dep   = dependency('glfw3')
png_dep    = dependency('libpng')
thread_dep = dependency('threads')
dl_dep     = cc.find_library('dl')

# compile flags (chagne ts juicey)
global_cpp_args = [
  '-fpermissive',
  '-Wtemplate-body',
  '-Wshift-count-overflow',
]

# global ccp defs type shi
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

subdir('4J.Render')
subdir('4J.Input')
subdir('4J.Profile')
subdir('4J.Storage')
subdir('Minecraft.World')
subdir('Minecraft.Client')
