cmake_minimum_required(VERSION 3.4...3.20 FATAL_ERROR)
project(hello_gfx)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/ext/cmake/modules/")
find_package(GLFW3 3.3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

set(CMAKE_VERBOSE_MAKEFILE ON)


#-----------------------------------------------------------------------------------------
#  global settings
#-----------------------------------------------------------------------------------------
set(O -O0)
set(STD -std=gnu++2a)
set(CMAKE_CXX_FLAGS ${STD}) #only for c++
add_compile_options(${O} -g -ggdb3 -fPIC -pthread -Wall )#-Werror

include_directories(
  "${CMAKE_SOURCE_DIR}/ext"
  "${CMAKE_SOURCE_DIR}/src/utils"
)

set(IS_GlEs -DIS_GlEs=1)
set(IS_GlLegacy -DIS_GlLegacy=1)
set(IS_ColorAttachRenderbuffer -DIS_ColorAttachRenderbuffer=1)
set(IS_ColorAttachTexture -DIS_ColorAttachTexture=1)

#-----------------------------------------------------------------------------------------
#  libraries
#-----------------------------------------------------------------------------------------
add_library(
  utils
  STATIC
  src/utils/myutils.cpp
)

link_libraries(
  glfw
  dl
  utils
)


#-----------------------------------------------------------------------------------------
#  targets
#-----------------------------------------------------------------------------------------

#add_executable(
#  xxx
#  xxx.c
#)
#target_link_libraries(
#  xxx
#)
#target_compile_options(
#  xxx
#  PUBLIC
#)

set(targets
  #-----------------------------------------------------------------------------------------
  # target_name \; target_sourcecode \; target_compile_options \; target_link_libraries
  #-----------------------------------------------------------------------------------------

  "triangle_gl   \; src/gl/triangle.cpp"
  "triangle_gles \; src/gl/triangle.cpp"

  "hello_triangle_gl   \; src/gl/hello_triangle.cpp"
  "hello_triangle_gles \; src/gl/hello_triangle.cpp"

  "glClear_gl   \; src/gl/glClear.cpp"
  "glClear_gles \; src/gl/glClear.cpp"

  "glVertexPointer_gl_legacy \; src/gl/glVertexPointer.cpp"
  "glVertexPointer_gl        \; src/gl/glVertexPointer.cpp"
  "glVertexPointer_gles      \; src/gl/glVertexPointer.cpp"

  "glTexCoordPointer_gl_legacy \; src/gl/glTexCoordPointer.cpp"
  "glTexCoordPointer_gl        \; src/gl/glTexCoordPointer.cpp"
  "glTexCoordPointer_gles      \; src/gl/glTexCoordPointer.cpp"

  "fbo_ColorAttachRenderbuffer_gl   \; src/gl/fbo.cpp \; ${IS_ColorAttachRenderbuffer}"
  "fbo_ColorAttachTexture_gl        \; src/gl/fbo.cpp \; ${IS_ColorAttachTexture}"
  "fbo_ColorAttachRenderbuffer_gles \; src/gl/fbo.cpp \; ${IS_ColorAttachRenderbuffer}"
  "fbo_ColorAttachTexture_gles      \; src/gl/fbo.cpp \; ${IS_ColorAttachTexture}"
)

foreach(target ${targets})
  list(LENGTH target length)
  #message("length=${length}")

  list(GET target 0 targetName)
  string(STRIP ${targetName} targetName)

  list(GET target 1 targetSource)
  string(STRIP ${targetSource} targetSource)

  set(targetCompile "")
  if(length GREATER_EQUAL 3)
    list(GET target 2 targetCompile)
    string(STRIP ${targetCompile} targetCompile)
    string(REPLACE " " ";" targetCompile "${targetCompile}")
  endif()

  set(targetLink "")
  if(length GREATER_EQUAL 4)
    list(GET target 3 targetLink)
    string(STRIP ${targetLink} targetLink)
    string(REPLACE " " ";" targetLink "${targetLink}")
  endif()

  if(${targetName} MATCHES "_gl$")
    # do nothing
  elseif(${targetName} MATCHES "_gl_legacy$")
    list(APPEND targetCompile ${IS_GlLegacy})
  elseif(${targetName} MATCHES "_gles$")
    list(APPEND targetCompile ${IS_GlEs})
  endif()

  #message("target=${target}")
  message("targetName=${targetName}, targetSource=${targetSource}, targetCompile=${targetCompile}, targetLink=${targetLink}")

  add_executable(${targetName} ${targetSource})
  target_compile_options(${targetName} PUBLIC ${targetCompile})
endforeach(target)


#add_executable(
#  perf_genmipmap
#  src/gl/perf_genmipmap.cpp
#)
#target_link_libraries(
#  perf_genmipmap
#)
#target_compile_options(
#  perf_genmipmap
#  PUBLIC
#)


#add_executable(
#  perf_copytex
#  src/gl/perf_copytex.cpp
#)
#target_link_libraries(
#  perf_copytex
#)
#target_compile_options(
#  perf_copytex
#  PUBLIC
#)
