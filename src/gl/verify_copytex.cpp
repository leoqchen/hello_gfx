/*
 * Verify glCopyTex[Sub]Image()
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

// settings
static const int WinWidth = 800;
static const int WinHeight = 600;

static int imgWidth, imgHeight, imgChannels;
static GLenum imgFormat;
static GLubyte *imgSrc;
static GLubyte *imgSrc2;

static GLuint program;

static void test( int texSize, int repeat, int mode, int dump )
{
    // resize source image
    imgSrc2 = stbir_resize_uint8_linear( imgSrc, imgWidth, imgHeight, 0, NULL, texSize, texSize, 0, (stbir_pixel_layout)imgChannels );

    char filename[64];
    sprintf( filename, "/tmp/%d_src.jpg", texSize );
    stbi_flip_vertically_on_write( 1 );
    stbi_write_jpg(filename, texSize, texSize, imgChannels, imgSrc2, 90 );
    printf("dump to %s\n", filename);

    // setup source
    GLuint texSrc;
    glGenTextures( 1, &texSrc );
    glBindTexture( GL_TEXTURE_2D, texSrc );
    glTexImage2D( GL_TEXTURE_2D, 0, imgFormat, texSize, texSize,
                  0, imgFormat, GL_UNSIGNED_BYTE, imgSrc2 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    GLuint fboSrc;
    glGenFramebuffers( 1, &fboSrc );
    glBindFramebuffer( GL_READ_FRAMEBUFFER, fboSrc );
    glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texSrc, 0 );
    GLenum stat = glCheckFramebufferStatus( GL_READ_FRAMEBUFFER );
    if( stat != GL_FRAMEBUFFER_COMPLETE ){
        printf("%s::%d: Error: incomplete FBO!: 0x%X, %s\n", __func__, __LINE__, stat, framebufferStatusName(stat));
        exit(1);
    }

    // setup dst
    GLuint texDst;
    glGenTextures( 1, &texDst );
    glBindTexture( GL_TEXTURE_2D, texDst );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //XXX 影响 tex->dirty
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLubyte* imgDst = (GLubyte*) malloc( texSize * texSize * imgChannels );

    // copy and verify
    double t = 0;
    for( int i=0; i < repeat; i++ ){
        // copy
        glBindTexture( GL_TEXTURE_2D, texDst );
        glReadBuffer( GL_COLOR_ATTACHMENT0 );
        if( mode != 0 ){
            /* setup empty dest texture */
            glTexImage2D(GL_TEXTURE_2D, 0, imgFormat,
                         texSize, texSize, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        }

        double t0 = PerfGetSecond();
        if( mode == 0 ) {
            glCopyTexImage2D(GL_TEXTURE_2D, 0, imgFormat, 0, 0, texSize, texSize, 0);
        }else{
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texSize, texSize);
        }
        t += PerfGetSecond() - t0;

        // verify by comparing dst, src
        glBindTexture( GL_TEXTURE_2D, texDst );
        glGetTexImage( GL_TEXTURE_2D, 0, imgFormat, GL_UNSIGNED_BYTE, imgDst );
        if( memcmp( imgSrc2, imgDst, texSize * texSize * imgChannels) != 0 ){
            printf("src2 and dst are diff !!!\n\n");
        }

        if( dump == 1 ) {
            sprintf(filename, "/tmp/%d_dst%02d.jpg", texSize, i);
            stbi_write_jpg(filename, texSize, texSize, imgChannels, imgDst, 90);
            printf("dump to %s\n", filename);
        }
    }

    double rate = (double)repeat / t;
    double mbPerSec = (double)(repeat * (texSize * texSize * imgChannels)) / (1024 * 1024) / t;
    printf("  glCopyTex%sImage(%d x %d): %.1f copies/sec, %.1f Mpixels/sec\n",
           (mode ? "Sub" : ""), texSize, texSize,
           rate, mbPerSec);

    glErrorCheck();
    free( imgDst );
    free( imgSrc2 );
}

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    const char* __file = stringFromArgs("--file", argc, argv );
    const char *imgFile = (__file) ? __file : PROJECT_SOURCE_DIR "data/tree2.rgba";
    imgSrc = imageFromFile( imgFile, &imgWidth, &imgHeight, &imgFormat, &imgChannels );

    stbi_flip_vertically_on_write( 1 );
    stbi_write_jpg("/tmp/src.jpg", imgWidth, imgHeight, imgChannels, imgSrc, 90 );
    printf("dump to /tmp/src.jpg\n");

    int __testcase = integerFromArgs( "--testcase", argc, argv, NULL );
    int __repeate = integerFromArgs("--repeat", argc, argv, NULL );
    int __mode = integerFromArgs( "--mode", argc, argv, NULL );
    int __dump = integerFromArgs( "--dump", argc, argv, NULL );

    // initialize and configure
    // ------------------------------
    eglx_CreateWindow( api, WinWidth, WinHeight );

    // build and compile our shader program
    // ------------------------------------
    const char *vertexShaderSource =
#if IS_GlEs
        "#version 320 es\n"
#else
        "#version 330\n"
        #endif
        "layout (location = 0) in vec3 vPos;\n"
        "layout (location = 1) in vec2 vTexCoord;\n"
        "out vec2 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4( vPos.x, vPos.y, vPos.z, 1.0f );\n"
        "   v_texCoord = vec2( vTexCoord.x, 1.0 - vTexCoord.y );\n" // flip vertically
        "}\n\0";

    const char *fragmentShaderSource =
#if IS_GlEs
        "#version 320 es\n"
    "precision mediump float;\n"
#else
        "#version 330\n"
        #endif
        "in vec2 v_texCoord;\n"
        "layout (location = 0) out vec4 outColor;\n"
        "uniform sampler2D s_texture;\n"
        "void main()\n"
        "{\n"
        "   outColor = texture( s_texture, v_texCoord );\n"
        "}\n\0";

    program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );
    glUseProgram( program ); //XXX 影响 tex->dirty

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    if( __testcase != -1 || __repeate != -1 || __mode != -1 ){
        test(__testcase,__repeate, __mode, __dump);
    }else {
        test(16, 10000, 0, 0);
        test(64, 10000, 0, 0);
        test(256, 10000, 0, 0);
        test(1024, 10000, 0, 0);
        test(4096, 10000, 0, 0);
    }
    exit( 0 );

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
    free(imgSrc);

    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
