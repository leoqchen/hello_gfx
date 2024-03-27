/*
 * 功能测试:
 * 直接显示纹理，不通过glDraw，而是通过glBlitFramebuffer
 * 步骤：
 *   1. 纹理上传，并绑定到FBO的Color Attachment
 *   2. 通过glBlitFramebuffer，把FBO的Color Attachment复制到Default Framebuffer，就能显示了
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"
#include "SGI_rgb.h"


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

    // build and compile our shader program
    // ------------------------------------

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // texture upload
    // ------------------------------------
    GLuint fboTex;
    glGenTextures( 1, &fboTex );
    glBindTexture( GL_TEXTURE_2D, fboTex );
    glTexImage2D( GL_TEXTURE_2D, 0, imgFormat,imgWidth, imgHeight,
                   0, imgFormat, GL_UNSIGNED_BYTE, image );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    // setup fbo
    // ------------------------------------
    GLint defaultFramebuffer = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &defaultFramebuffer );

    GLuint fbo;
    glGenFramebuffers( 1, &fbo );
    glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0 );
    GLenum stat = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( stat != GL_FRAMEBUFFER_COMPLETE ){
        printf("Error: incomplete FBO!: 0x%X, %s\n", stat, framebufferStatusName(stat));
        exit(1);
    }

    // copy FBO's color attachment to Default Framebuffer
    // ---------------------------------------------------
    glBindFramebuffer( GL_FRAMEBUFFER, defaultFramebuffer );
    glBindFramebuffer( GL_READ_FRAMEBUFFER, fbo );
    glReadBuffer ( GL_COLOR_ATTACHMENT0 );
    glBlitFramebuffer( 0, 0, imgWidth, imgHeight,
                       0, 0, WinWidth, WinHeight,
                       GL_COLOR_BUFFER_BIT, GL_LINEAR );

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
    glDeleteTextures( 1, &fboTex );
    glDeleteFramebuffers( 1, &fbo );
    free(image);

    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
