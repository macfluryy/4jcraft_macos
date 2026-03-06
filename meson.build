project('4jcraft-chucklegrounds', ['cpp', 'c'],
  version : '0.1.0',
  default_options : [
    'cpp_std=gnu++11',
    'warning_level=0',
    'buildtype=debug',
    'unity=on',         # merge source files per target → fewer compile units
    'unity_size=8',     # files per unity batch (tune up for faster full builds)
    'b_pch=true',       # precompiled headers
  ],
)

cc = meson.get_compiler('cpp')

# Use LLD for dramatically faster linking (ld.lld must be installed)
add_project_link_arguments('-fuse-ld=lld', language : ['c', 'cpp'])

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
  '-Wshift-count-overflow',
  '-pipe',             # use pipes instead of temp files between compiler stages
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
