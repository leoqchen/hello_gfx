#pragma once
#include "glad.h"

#include <stdio.h>
#include <stdlib.h>
#include "myUtils.h"

#define glErrorCheck() \
    {\
      GLenum error;\
      if( (error = glGetError()) != GL_NO_ERROR ){\
        printf("glGetError: %s, %s:%s:%d\n", glErrorName(error), __func__, __FILE__, __LINE__);\
        exit(EXIT_FAILURE);\
      }\
    }

const char* glErrorName( GLenum error );
const char* glShaderTypeName( GLenum shaderType );
const char* framebufferStatusName( GLenum status );
const char* glContextProfileBitName( GLint profileBit );
const char* glContextFlagName( GLint flag );
const char* glFormatName( GLenum format );
const char* glslVersion( api_t api );

#if IS_GlLegacy
void MatrixPrint( GLenum pname, const char *file, int line );
#endif

GLuint CreateShaderFromSource( GLenum type, const char *shaderSource );
GLuint CreateProgramFromShader( GLuint vertShader, GLuint fragShader );
GLuint CreateProgramFromSource( const char *vertShaderSource, const char *fragShaderSource );

GLuint CreateTexture_FillWithCheckboard( GLsizei width, GLsizei height );

GLubyte* imageFromFile( const char *filename, GLsizei *width, GLsizei *height, GLenum *format, GLsizei *channels );

void ReadPixels_FromFboColorAttachment( void *dstData, GLuint texture, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type );
