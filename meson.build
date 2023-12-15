project('clang-extract', 'cpp',
  version : '0.1',
  default_options : [
    'cpp_std=c++20',
    'warning_level=2',
    'b_sanitizer=address'
  ]
)

add_project_arguments(
  [
    '-Wno-unused-parameter',
    '-Wuninitialized',
    '-Wmissing-field-initializers',
    '-g3'
  ], language: 'cpp'
)

compiler_name = meson.get_compiler('cpp').get_id()
# FIXME: We currently don't support g++, and get_id returns clang for clang++
assert(compiler_name == 'clang', compiler_name + ' found but only clang is supported')

elf_dep = dependency('libelf')

subdir('libcextract')

incdir = include_directories('libcextract')

executable('inline', 'Inline.cpp',
  include_directories : incdir,
  link_with : libcextract_static,
  dependencies : elf_dep
)

executable('clang-extract', 'Main.cpp',
  include_directories : incdir,
  link_with : libcextract_static,
  # FIXME: make it work using the dependecy constructor, instead of plain link
  # args
  link_args : ['-l:libclang-cpp.so.16', '-l:libLLVM.so.16'],
  dependencies : elf_dep
)
