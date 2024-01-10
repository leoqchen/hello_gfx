cmake_minimum_required(VERSION 3.0)
project(hello_gfx)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/src/utils/cmake/modules/")
#find_package(GLFW3 REQUIRED)
find_package(glfw3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(O -O0)
set(STD -std=gnu++2a)
set(CMAKE_CXX_FLAGS ${STD}) #only for c++

add_compile_options(${O} -g -ggdb3 -fPIC -pthread -Wall -Werror)
link_libraries(glfw dl)


include_directories(
  src/utils
  src/utils/glad
)


#-----------------------------------------------------------------------------------------
#  libraries
#-----------------------------------------------------------------------------------------
add_library(
  utils
  STATIC
  src/utils/glad/glad.c
  src/utils/myutils.cpp
)


#-----------------------------------------------------------------------------------------
#  targets
#-----------------------------------------------------------------------------------------
add_executable(
  hello_triangle
  src/gl/hello_triangle.cpp
)
target_link_libraries(
  hello_triangle
  utils
)
target_compile_options(
  hello_triangle
  PUBLIC
  # xxx
)


add_executable(
  glClear
  src/gl/glClear.cpp
)
target_link_libraries(
  glClear
  utils
)
target_compile_options(
  glClear
  PUBLIC
  # xxx
)


add_executable(
  perf_genmipmap
  src/gl/perf_genmipmap.cpp
)
target_link_libraries(
  perf_genmipmap
  utils
)
target_compile_options(
  perf_genmipmap
  PUBLIC
  # xxx
)


add_executable(
  perf_copytex
  src/gl/perf_copytex.cpp
)
target_link_libraries(
  perf_copytex
  utils
)
target_compile_options(
  perf_copytex
  PUBLIC
  # xxx
  -Wno-error #TODO
)
