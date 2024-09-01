set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/utils)

# myUtils
add_library(
  myUtils
  STATIC
  myUtils.cpp
)

# x11 utils
add_library(
  x11Utils
  STATIC
  x11Utils.cpp
)

# gl utils
add_library(
  glUtils_gl
  STATIC
  glUtils.cpp
  SGI_rgb.cpp
)
add_library(
  glUtils_gles2
  STATIC
  glUtils.cpp
  SGI_rgb.cpp
)
target_compile_options(
  glUtils_gles2
  PUBLIC
  ${IS_GlEs}
)

# glfw utils
add_library(
  glfwUtils_gl
  STATIC
  glfwUtils.cpp
)
add_library(
  glfwUtils_gles2
  STATIC
  glfwUtils.cpp
)
target_compile_options(
  glfwUtils_gles2
  PUBLIC
  ${IS_GlEs}
)

# EGL utils
add_library(
  eglUtils_gl
  STATIC
  eglUtils.cpp
)
add_library(
  eglUtils_gles2
  STATIC
  eglUtils.cpp
)
target_compile_options(
  eglUtils_gles2
  PUBLIC
  ${IS_GlEs}
)


#-----------------------------------------------------------------------------------------
#  targets
#-----------------------------------------------------------------------------------------
add_executable(
  stbTest
  stbTest.cpp
)

