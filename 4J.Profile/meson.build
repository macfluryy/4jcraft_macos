profile_sources = files(
  '4J_Profile.cpp',
  'PRO_AwardManager.cpp',
  'PRO_Data.cpp',
  'PRO_Main.cpp',
  'PRO_RichPresence.cpp',
  'PRO_Sys.cpp',
  'stdafx.cpp',
)
# 4jprofile stink
lib_profile = static_library('4J_Profile',
  profile_sources,
  include_directories : include_directories('.'),
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-include', meson.current_source_dir() / 'stdafx.h',
  ],
)

profile_dep = declare_dependency(
  link_with : lib_profile,
  include_directories : include_directories('.'),
)
