# sources that shouldn't be compiled for whatever reason
exclude_sources = [
  ' ! -path "*/Platform/*"',
  ' ! -path "*/../Common/Network/Sony/*"',
  ' ! -path "*/../Common/XUI/*"',
  ' ! -path "*/../Common/zlib/*"',
]

# get all those files
client_sources = run_command(
  'sh', '-c',
  'find "'
    + meson.current_source_dir()
    + '" \\( -name "*.cpp" -o -name "*.c" \\) '
    + ' '.join(exclude_sources),
  check : true,
).stdout().strip().split('\n')

# Common/ sources (previously lived in Minecraft.Client/Build/Common/)
common_exclude = [
  ' ! -path "*/Network/Sony/*"',
  ' ! -path "*/XUI/*"',
  ' ! -path "*/zlib/*"',
  ' ! -path "*/Media/*"',
  ' ! -path "*/DummyTexturePack/*"',
]

common_sources = run_command(
  'sh', '-c',
  'find "'
    + meson.current_source_dir() / '../Common'
    + '" \\( -name "*.cpp" -o -name "*.c" \\) '
    + ' '.join(common_exclude),
  check : true,
).stdout().strip().split('\n')

platform_sources = []

# linux-specific files (everything in Platform/Linux)
if host_machine.system() == 'linux'
  platform_sources += run_command(
    'sh', '-c',
    'find "'
      + meson.current_source_dir() / 'Platform/Linux'
      + '" \\( -name "*.cpp" -o -name "*.c" \\) ',
    check : true,
  ).stdout().strip().split('\n')
endif

executable('Minecraft.Client',
  client_sources + platform_sources + common_sources,
  include_directories : include_directories('Build', '..'),
  dependencies : [
    render_dep,
    input_dep,
    profile_dep,
    storage_dep,
    world_dep,
    gl_dep,
    glu_dep,
    glfw_dep,
    thread_dep,
    dl_dep,
    dependency('zlib'),
  ],
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-DUNICODE', '-D_UNICODE',
    '-include', meson.current_source_dir() / 'Build/stdafx.h',
  ],
  c_args   : global_cpp_defs + ['-DUNICODE', '-D_UNICODE'],
  install  : false,
)

# This is shitty because we don't actually know what files we want to pack into an arc
# until we check the filter files (movies.txt media.txt etc....), we should get rid of
# the filters and use a dedicated asset dir that we can just entirely pack into an arc
# so we can have changes in our asset files cause arc rebuild instead of doing it every build
archive_target_dir = meson.current_source_dir() / 'Build/Common/Media'
custom_target('Minecraft.Client_Archive',
  output : 'MediaWindows64.arc',
  command : [
    'python3', meson.project_source_root() / 'scripts/pack_arc.py',
    archive_target_dir,
    '@OUTPUT@', 
    archive_target_dir / 'movies1080.txt',
    archive_target_dir / 'media.txt'
  ],
  install : true,
  install_dir : 'Common/Media',
  build_subdir : 'Common/Media',
  build_by_default : true,
  build_always_stale : true
)

custom_target('Minecraft.Client_Localisation',
  output : 'languages.loc',
  command : [
    'python3', meson.project_source_root() / 'scripts/pack_loc.py',
    meson.current_source_dir() / 'Platform/Windows64Media/loc',
    '@OUTPUT@'
  ],
  build_by_default : true,
  build_always_stale : true
)
