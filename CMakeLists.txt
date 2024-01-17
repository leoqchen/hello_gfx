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

set(GLES -DGLES=1)
set(GL_LEGACY -DGL_LEGACY=1)

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
  triangle_gl
  triangle_gles

  hello_triangle_gl
  hello_triangle_gles

  glClear_gl
  glClear_gles

  glVertexPointer_gl_legacy
  glVertexPointer_gl
  glVertexPointer_gles

  glTexCoordPointer_gl_legacy
  glTexCoordPointer_gl
  glTexCoordPointer_gles
)

foreach(target ${targets})
  if(${target} MATCHES "_gl$")
    string(REGEX REPLACE "_gl$" "" filename ${target})
    set(compile_option "")
  elseif(${target} MATCHES "_gl_legacy$")
    string(REGEX REPLACE "_gl_legacy$" "" filename ${target})
    set(compile_option ${GL_LEGACY})
  elseif(${target} MATCHES "_gles$")
    string(REGEX REPLACE "_gles$" "" filename ${target})
    set(compile_option ${GLES})
  endif()

  add_executable(${target} src/gl/${filename}.cpp)
  target_compile_options(${target} PUBLIC ${compile_option})
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
