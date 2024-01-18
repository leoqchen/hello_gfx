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
  # ----------------------------------------------------
  # target_name \; sourcecode_name \; compile_options
  # ----------------------------------------------------

  "triangle_gl   \; src/gl/triangle.cpp \; "
  "triangle_gles \; src/gl/triangle.cpp \; ${GLES}"

  "hello_triangle_gl   \; src/gl/hello_triangle.cpp \; "
  "hello_triangle_gles \; src/gl/hello_triangle.cpp \; ${GLES}"

  "glClear_gl   \; src/gl/glClear.cpp \; "
  "glClear_gles \; src/gl/glClear.cpp \; ${GLES}"

  "glVertexPointer_gl_legacy \; src/gl/glVertexPointer.cpp \; ${GL_LEGACY}"
  "glVertexPointer_gl        \; src/gl/glVertexPointer.cpp \; "
  "glVertexPointer_gles      \; src/gl/glVertexPointer.cpp \; ${GLES}"

  "glTexCoordPointer_gl_legacy \; src/gl/glTexCoordPointer.cpp \; ${GL_LEGACY}"
  "glTexCoordPointer_gl        \; src/gl/glTexCoordPointer.cpp \; "
  "glTexCoordPointer_gles      \; src/gl/glTexCoordPointer.cpp \; ${GLES}"
)

foreach(target ${targets})
  list(GET target 0 targetName)
  list(GET target 1 targetSource)
  list(GET target 2 targetCompile)

  string(STRIP ${targetName} targetName)
  string(STRIP ${targetSource} targetSource)
  string(STRIP ${targetCompile} targetCompile)

  #message("target=${target}")
  message("targetName=${targetName}, targetSource=${targetSource}, targetCompile=${targetCompile}")

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
