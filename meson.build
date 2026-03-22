project('4jcraft-chucklegrounds', ['cpp', 'c'],
  version : '0.1.0',
  meson_version: '>= 1.1',
  default_options : [
    'cpp_std=c++23',
    'warning_level=0',
    'buildtype=debug', # for now
    'unity=off', # merge source files per target
    # 'unity_size=8', # TODO: mess around with this
    'b_pch=true', # precompiled headers
  ],
)

pymod = import('python')
python = pymod.find_installation('python3', required: true)

cc = meson.get_compiler('cpp')

# system deps
gl_dep     = dependency('gl')
glu_dep    = dependency('glu')
sdl2_dep   = dependency('sdl2') # Yes.. i know sdl3 is out, but there's not point upgrading right now except when
                                # someone is gonna ask me "Hey juicey can you make it SDL3?" and i'd be like fuck you and still do it.
thread_dep = dependency('threads')

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

render_dep = dependency('4j-render', fallback: ['4jlibs', 'render_dep'])
input_dep = dependency('4j-input', fallback: ['4jlibs', 'input_dep'])
profile_dep = dependency('4j-profile', fallback: ['4jlibs', 'profile_dep'])
storage_dep = dependency('4j-storage', fallback: ['4jlibs', 'storage_dep'])

subdir('Minecraft.Assets')
subdir('Minecraft.World')
subdir('Minecraft.Client')
