storage_sources = files(
  '4J_Storage.cpp',
  'stdafx.cpp',
  'STO_DLC.cpp',
  'STO_Main.cpp',
  'STO_SaveGame.cpp',
)

lib_storage = static_library('4J_Storage',
  storage_sources,
  include_directories : include_directories('.'),
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-include', meson.current_source_dir() / 'stdafx.h',
  ],
)

storage_dep = declare_dependency(
  link_with : lib_storage,
  include_directories : include_directories('.'),
)
