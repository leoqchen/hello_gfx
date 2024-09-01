#-----------------------------------------------------------------------------------------
#  CMake settings
#-----------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.4...3.20 FATAL_ERROR)
project(hello_gfx)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/ext/cmake/modules/")
find_package(OpenGL REQUIRED)
find_package(OpenGLES 3.2 REQUIRED)
find_package(X11 REQUIRED)
find_package(EGL REQUIRED)
find_package(GLFW3 3.3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

set(CMAKE_VERBOSE_MAKEFILE ON)

# set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

message("PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
message("PROJECT_BINARY_DIR = ${PROJECT_BINARY_DIR}")
message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message("CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
message("CMAKE_ARCHIVE_OUTPUT_DIRECTORY = ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message("CMAKE_LIBRARY_OUTPUT_DIRECTORY = ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message("CMAKE_RUNTIME_OUTPUT_DIRECTORY = ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")


#-----------------------------------------------------------------------------------------
#  global settings
#-----------------------------------------------------------------------------------------
set(STD -std=gnu++2a)
set(CMAKE_CXX_FLAGS ${STD}) #only for c++
add_compile_options(-g -ggdb3 -fPIC -pthread -Wall )#-Werror

include_directories(
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/ext"
  "${PROJECT_SOURCE_DIR}/ext/glad/include"
  "${PROJECT_SOURCE_DIR}/ext/stb"
  "${PROJECT_SOURCE_DIR}/src/utils"
)

configure_file(src/utils/project_config.h.in project_config.h)
include_directories(${PROJECT_BINARY_DIR}/configuration)

set(IS_GlLegacy -DIS_GlLegacy=1)
set(IS_Gl -DIS_Gl=1)
set(IS_GlEs -DIS_GlEs=1)
set(IS_ColorAttachRenderbuffer -DIS_ColorAttachRenderbuffer=1)
set(IS_ColorAttachTexture -DIS_ColorAttachTexture=1)
set(IS_Cpu -DIS_Cpu=1)


link_libraries(
  dl
)

#-----------------------------------------------------------------------------------------
#  libraries
#-----------------------------------------------------------------------------------------
add_subdirectory(
  ${PROJECT_SOURCE_DIR}/ext
)

add_subdirectory(
  ${PROJECT_SOURCE_DIR}/src/utils
)


#-----------------------------------------------------------------------------------------
#  OpenGL samples
#-----------------------------------------------------------------------------------------
add_subdirectory(
  ${PROJECT_SOURCE_DIR}/src/gl
)


#-----------------------------------------------------------------------------------------
#  targets
#-----------------------------------------------------------------------------------------
#add_executable(
#  xxx
#  xxx.c
#)
#target_compile_options(
#  xxx
#  PUBLIC
#)
#target_link_libraries(
#  xxx
#)

