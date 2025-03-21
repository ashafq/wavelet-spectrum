project('wavelet-spectrum', 'c',
  version : '1.0.0',
  default_options : ['warning_level=3'])

cmake_mod = import('cmake')
raylib_proj = cmake_mod.subproject('raylib')
raylib_dep = raylib_proj.dependency('raylib')

libdsp = static_library('libdsp',
  sources : ['libdsp/haar.c', 'libdsp/spectrum.c'],
  install : true)

libdsp_dep = declare_dependency(
  link_with : libdsp,
  include_directories : include_directories('libdsp'))


# Link with system math library (-lm)
cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required : true)

dep = [
  raylib_dep,
  libdsp_dep,
  math_dep,
]

executable('spectrum',
           'src/main.c',
           install : true,
           dependencies: dep)
