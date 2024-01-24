#pragma once

#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "myutils.h"


const char* glErrorName( GLenum error );
const char* glShaderTypeName( GLenum shaderType );
const char* glslVersion( api_t api );

#define glErrorCheck() \
    {\
      GLenum error;\
      if( (error = glGetError()) != GL_NO_ERROR ){\
        printf("glGetError: %s, %s:%s:%d\n", glErrorName(error), __FILE__, __func__, __LINE__);\
        exit(EXIT_FAILURE);\
      }\
    }

GLuint CreateShaderFromSource( GLenum type, const char *shaderSource );
GLuint CreateProgramFromShader( GLuint vertShader, GLuint fragShader );
GLuint CreateProgramFromSource( const char *vertShaderSource, const char *fragShaderSource );