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
add_compile_options(${O} -g -ggdb3 -fPIC -pthread -Wall -Werror)

include_directories(
  "${CMAKE_SOURCE_DIR}/ext"
  "${CMAKE_SOURCE_DIR}/src/utils"
)


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
add_executable(
  triangle-opengl
  src/gl/triangle-opengl.c
)
target_link_libraries(
  triangle-opengl
)
target_compile_options(
  triangle-opengl
  PUBLIC
)


add_executable(
  triangle-opengles
  src/gl/triangle-opengles.c
)
target_link_libraries(
  triangle-opengles
)
target_compile_options(
  triangle-opengles
  PUBLIC
)

#add_executable(
#  hello_triangle
#  src/gl/hello_triangle.cpp
#)
#target_link_libraries(
#  hello_triangle
#)
#target_compile_options(
#  hello_triangle
#  PUBLIC
#)
#
#
#add_executable(
#  glClear
#  src/gl/glClear.cpp
#)
#target_link_libraries(
#  glClear
#)
#target_compile_options(
#  glClear
#  PUBLIC
#)
#
#
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
#
#
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