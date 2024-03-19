#pragma once

/*
 * glad include
 */
#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#include <GL/glu.h>
#endif
