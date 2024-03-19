#pragma once
/*
 * Read an SGI .rgb image file and generate a mipmap texture set.
 * Much of this code was borrowed from SGI's tk OpenGL toolkit.
 */
#include "glad.h"


//GLboolean SGI_LoadRGBMipmaps( const char *imageFile, GLint intFormat );
//GLboolean SGI_LoadRGBMipmaps2( const char *imageFile, GLenum target, GLint intFormat, GLint *width, GLint *height );

GLubyte* SGI_LoadRGBImage( const char *imageFile, GLint *width, GLint *height, GLenum *format );

GLushort* SGI_LoadYUVImage( const char *imageFile, GLint *width, GLint *height );
