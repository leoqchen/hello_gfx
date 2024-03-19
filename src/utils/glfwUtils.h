/*
 * glad + GLFW glue codes
 */
#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "myUtils.h"

GLFWwindow* glfw_CreateWindow(api_t api, int width, int height );
