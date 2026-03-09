project('4jcraft-chucklegrounds', ['cpp', 'c'],
  version : '0.1.0',
  default_options : [
    'buildtype=debug', # for now
    'unity=on', # merge source files per target
    'unity_size=8', # TODO: mess around with this
    'b_pch=true', # precompiled headers
  ],
)

pymod = import('python')
python = pymod.find_installation('python3', required: true)

cc = meson.get_compiler('cpp')
buildtype = get_option('buildtype')
#langs cant sadly be set before calling project
langs = ['cpp', 'c']

# system deps
gl_dep     = dependency('gl')
glu_dep    = dependency('glu')
glfw_dep   = dependency('glfw3')
png_dep    = dependency('libpng')
thread_dep = dependency('threads')
dl_dep     = cc.find_library('dl')

if buildtype.startswith('debug')
    strAsan = '-fsanitize=address'
    strLsan = '-fsanitize=leak'
    if cc.has_argument(strAsan)
        add_project_arguments(strAsan, language: langs)
        add_project_link_arguments(strAsan, language: langs)
    endif
    if cc.has_argument(strLsan)
        add_project_arguments(strLsan, language: langs)
        add_project_link_arguments(strLsan, language: langs)
    endif
endif

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
subdir('Minecraft.Assets')
subdir('Minecraft.World')
subdir('Minecraft.Client')
