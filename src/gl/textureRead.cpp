/*
 * 功能测试:
 * CPU读取纹理，不通过glDraw，而是通过glBlitFramebuffer
 *   方法1：
 *     1. 纹理上传，并把纹理绑定到FBO的Color Attachment
 *     2. glReadBuffer指定Color Attachment后，通过glReadPixels读取FBO的Color Attachment
 *
 *   方法2：使用glGetTexImage （但是OpenGLES不支持）
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"
#include "SGI_rgb.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// settings
static const int WinWidth = 800;
static const int WinHeight = 600;

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    // initialize and configure
    // ------------------------------
    eglx_CreateWindow( api, WinWidth, WinHeight );

    // load image from file
    // ------------------------------------
    const char *TexFile = PROJECT_SOURCE_DIR  "data/tree2.rgba";
    GLint imgWidth, imgHeight;
    GLenum imgFormat;
    GLubyte *image = SGI_LoadRGBImage( TexFile, &imgWidth, &imgHeight, &imgFormat);
    printf("%s: width = %d, height = %d, format = %s\n", TexFile, imgWidth, imgHeight, glFormatName(imgFormat));
    if (!image) {
        printf("Couldn't read %s\n", TexFile);
        exit(0);
    }

    stbi_flip_vertically_on_write( 1 );
    stbi_write_png("/tmp/src.png", imgWidth, imgHeight, 4, image, imgWidth*4 );
    printf("dump to /tmp/src.png\n");

    // build and compile our shader program
    // ------------------------------------

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // texture upload
    // ------------------------------------
    GLuint srcTex;
    glGenTextures( 1, &srcTex );
    glBindTexture( GL_TEXTURE_2D, srcTex );
    glTexImage2D( GL_TEXTURE_2D, 0, imgFormat,imgWidth, imgHeight,
                   0, imgFormat, GL_UNSIGNED_BYTE, image );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    // CPU read texture by FBO color attachment
    // ------------------------------------
    GLint defaultFramebuffer = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &defaultFramebuffer );

    GLuint fbo;
    glGenFramebuffers( 1, &fbo );
    glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTex, 0 );
    GLenum stat = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( stat != GL_FRAMEBUFFER_COMPLETE ){
        printf("Error: incomplete FBO!: 0x%X, %s\n", stat, framebufferStatusName(stat));
        exit(1);
    }

    // CPU read texture
    GLubyte* imageDst = (GLubyte*) malloc( imgWidth * imgHeight * 4 );
    glReadBuffer ( GL_COLOR_ATTACHMENT0 );
    glReadPixels(0, 0, imgWidth, imgHeight, imgFormat, GL_UNSIGNED_BYTE, imageDst);

    stbi_write_png("/tmp/dst.png", imgWidth, imgHeight, 4, imageDst, imgWidth*4 );
    printf("dump to /tmp/dst.png\n");

    // compare result
    if( memcmp(imageDst, image, imgWidth * imgHeight * 4) == 0 ){
        printf("src and dst are the same\n");
    }else{
        printf("src and dst are diff !!!\n");
        exit(1);
    }

    // CPU read texture by glGetTexImage
    // ------------------------------------
#if IS_Gl
    GLubyte* imageDst2 = (GLubyte*) malloc( imgWidth * imgHeight * 4 );
    glBindTexture( GL_TEXTURE_2D, srcTex );
    glGetTexImage( GL_TEXTURE_2D, 0, imgFormat, GL_UNSIGNED_BYTE, imageDst2 );

    stbi_write_png("/tmp/dst2.png", imgWidth, imgHeight, 4, imageDst2, imgWidth*4 );
    printf("\ndump to /tmp/dst2.png\n");

    if( memcmp(imageDst2, image, imgWidth * imgHeight * 4) == 0 ){
        printf("src and dst2 are the same\n");
    }else{
        printf("src and dst2 are diff !!!\n");
        exit(1);
    }
#endif

    // render loop
    // -----------
    while (!eglx_ShouldClose())
    {
        // render
        // ------

        // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        eglx_SwapBuffers();
        eglx_PollEvents();
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteTextures( 1, &srcTex );
    glDeleteFramebuffers( 1, &fbo );
    free(image);
    free( imageDst );
#if IS_Gl
    free( imageDst2 );
#endif

    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
