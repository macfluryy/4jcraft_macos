simdutf_dep = dependency('simdutf',
  fallback: ['simdutf', 'simdutf_dep'],
  default_options: ['utf8=true', 'utf16=true', 'utf32=true']
)

lib_util = static_library('util',
  files('src/StringHelpers.cpp'),
  dependencies: [simdutf_dep],
  include_directories : include_directories('include', '..'),
  cpp_args : global_cpp_args + global_cpp_defs,
)

util_dep = declare_dependency(
  link_with : lib_util,
  include_directories : include_directories('include'),
)
