/*
 * glfw, glad, OpenGL, OpenGLES glue codes
 */
#pragma once

#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "myUtils.h"

GLFWwindow* glfwInit_CreateWindow( api_t api, int width, int height );
