input_sources = files(
  '4J_Input.cpp',
  'INP_ForceFeedback.cpp',
  'INP_Keyboard.cpp',
  'INP_Main.cpp',
  'INP_StringCheck.cpp',
  'stdafx.cpp',
)

lib_input = static_library('4J_Input',
  input_sources,
  include_directories : include_directories('.'),
  cpp_args : global_cpp_args + global_cpp_defs + [
    '-include', meson.current_source_dir() / 'stdafx.h',
  ],
)

input_dep = declare_dependency(
  link_with : lib_input,
  include_directories : include_directories('.'),
)
