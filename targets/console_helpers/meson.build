console_helpers_sources = files(
    'src/C4JThread.cpp',
    'src/StringHelpers.cpp',
    'src/compression.cpp',
)

simdutf_dep = dependency('simdutf', 
  fallback: ['simdutf', 'simdutf_dep'], 
  default_options: ['utf8=true', 'utf16=true', 'utf32=true']
)

lib_console_helpers = static_library('console_helpers',
  console_helpers_sources,
  dependencies: [simdutf_dep, dependency('zlib')],
  include_directories : include_directories('include', '..'),
  cpp_args : global_cpp_args + global_cpp_defs,
)

console_helpers_dep = declare_dependency(
  link_with : lib_console_helpers,
  include_directories : include_directories('include'),
)
