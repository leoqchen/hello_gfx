/*
 * 功能测试:
 * texture copy
 *   method 1: glCopyTexImage2D from FBO source texture to destination texture
 *   method 2: glBlitFramebuffer from source FBO texture to destination FBO texture
 *   method 3: render source texture to destination FBO texture
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

// settings
static const int WinWidth = 800;
static const int WinHeight = 600;

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    const char* __file = stringFromArgs("--file", argc, argv );
    const char *imgFile = (__file) ? __file : PROJECT_SOURCE_DIR "data/tree2.rgba";
    int imgWidth, imgHeight, imgChannels;
    GLenum imgFormat;
    GLubyte *imgSrc = imageFromFile( imgFile, &imgWidth, &imgHeight, &imgFormat, &imgChannels );

    stbi_flip_vertically_on_write( 1 );
    stbi_write_png("/tmp/src.png", imgWidth, imgHeight, imgChannels, imgSrc, imgWidth * imgChannels );
    printf("dump to /tmp/src.png\n");

    // initialize and configure
    // ------------------------------
    eglx_CreateWindow( api, WinWidth, WinHeight );

    // build and compile our shader program
    // ------------------------------------

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // texture upload
    // ------------------------------------
    GLuint texSrc;
    glGenTextures( 1, &texSrc );
    glBindTexture( GL_TEXTURE_2D, texSrc );
    glTexImage2D( GL_TEXTURE_2D, 0, imgFormat,imgWidth, imgHeight,
                   0, imgFormat, GL_UNSIGNED_BYTE, imgSrc );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    GLint defaultFramebuffer = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &defaultFramebuffer );

    // method 1: glCopyTexImage2D from FBO source texture to destination texture
    // -------------------------------------------------------------------------
    {
        // setup source
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

        // copy
        glBindTexture( GL_TEXTURE_2D, texDst );
        glReadBuffer( GL_COLOR_ATTACHMENT0 );
        glCopyTexImage2D(GL_TEXTURE_2D, 0, imgFormat, 0, 0, imgWidth, imgHeight, 0 );

        // compare dst, src
        GLubyte* imgDst = (GLubyte*) malloc( imgWidth * imgHeight * imgChannels );
        glBindTexture( GL_TEXTURE_2D, texDst );
        glGetTexImage( GL_TEXTURE_2D, 0, imgFormat, GL_UNSIGNED_BYTE, imgDst );

        stbi_write_png("/tmp/dst1.png", imgWidth, imgHeight, imgChannels, imgDst, imgWidth * imgChannels );
        printf("dump to /tmp/dst1.png\n");
        if( memcmp(imgDst, imgSrc, imgWidth * imgHeight * imgChannels) == 0 ){
            printf("src and dst1 are the same\n\n");
        }else{
            printf("src and dst1 are diff !!!\n\n");
        }
        glErrorCheck();
    }

    // method 2: glBlitFramebuffer from source FBO texture to destination FBO texture
    // -------------------------------------------------------------------------
    {
        // setup source
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
        glTexImage2D( GL_TEXTURE_2D, 0, imgFormat,imgWidth, imgHeight,
                      0, imgFormat, GL_UNSIGNED_BYTE, NULL );

        GLuint fboDst;
        glGenFramebuffers( 1, &fboDst );
        glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboDst );
        glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texDst, 0 );
        stat = glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER );
        if( stat != GL_FRAMEBUFFER_COMPLETE ){
            printf("%s::%d: Error: incomplete FBO!: 0x%X, %s\n", __func__, __LINE__, stat, framebufferStatusName(stat));
            exit(1);
        }

        // copy
        glBlitFramebuffer( 0, 0, imgWidth, imgHeight,
                           0, 0, imgWidth, imgHeight,
                           GL_COLOR_BUFFER_BIT, GL_NEAREST );

        // compare dst, src
        GLubyte* imgDst = (GLubyte*) malloc( imgWidth * imgHeight * imgChannels );
        glBindTexture( GL_TEXTURE_2D, texDst );
        glGetTexImage( GL_TEXTURE_2D, 0, imgFormat, GL_UNSIGNED_BYTE, imgDst );

        stbi_write_png("/tmp/dst2.png", imgWidth, imgHeight, imgChannels, imgDst, imgWidth * imgChannels );
        printf("dump to /tmp/dst2.png\n");
        if( memcmp(imgDst, imgSrc, imgWidth * imgHeight * imgChannels) == 0 ){
            printf("src and dst2 are the same\n\n");
        }else{
            printf("src and dst2 are diff !!!\n\n");
        }
        glErrorCheck();
    }

    // method 3: render source texture to destination FBO texture
    // -------------------------------------------------------------------------
    {
        // setup source
        const char *vertexShaderSource =
#if IS_GlEs
            "#version 320 es\n"
#else
            "#version 400\n"
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
            "#version 400\n"
            #endif
            "in vec2 v_texCoord;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "uniform sampler2D s_texture;\n"
            "void main()\n"
            "{\n"
            "   outColor = texture( s_texture, v_texCoord );\n"
            "}\n\0";

        const GLuint program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );
        GLint samplerLoc = glGetUniformLocation( program, "s_texture" );

        static const GLfloat vVertices[] = {
            -1.0f,  1.0f, 0.0f,  // Position 0
            0.0f,  0.0f,        // TexCoord 0
            -1.0f, -1.0f, 0.0f,  // Position 1
            0.0f,  1.0f,        // TexCoord 1
            1.0f, -1.0f, 0.0f,  // Position 2
            1.0f,  1.0f,        // TexCoord 2
            1.0f,  1.0f, 0.0f,  // Position 3
            1.0f,  0.0f         // TexCoord 3
        };
        static const GLushort indices[] = {
            0, 1, 2, 0, 2, 3
        };
        GLuint vertex_array;
        glGenVertexArrays(1, &vertex_array);
        glBindVertexArray(vertex_array);

        GLuint vertex_buffer;
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);

        const GLint vPos_location = glGetAttribLocation(program, "vPos");
        const GLint vTexCoord_location = glGetAttribLocation(program, "vTexCoord");
        glVertexAttribPointer(vPos_location, 3, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 5, (void*)0);
        glVertexAttribPointer(vTexCoord_location, 2, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 5, (void *) (sizeof(GLfloat) * 3));
        glEnableVertexAttribArray(vPos_location);
        glEnableVertexAttribArray(vTexCoord_location);

        // setup dst
        GLuint texDst;
        glGenTextures( 1, &texDst );
        glBindTexture( GL_TEXTURE_2D, texDst );
        glTexImage2D( GL_TEXTURE_2D, 0, imgFormat,imgWidth, imgHeight,
                      0, imgFormat, GL_UNSIGNED_BYTE, NULL );

        GLuint fboDst;
        glGenFramebuffers( 1, &fboDst );
        glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboDst );
        glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texDst, 0 );
        GLenum stat = glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER );
        if( stat != GL_FRAMEBUFFER_COMPLETE ){
            printf("%s::%d: Error: incomplete FBO!: 0x%X, %s\n", __func__, __LINE__, stat, framebufferStatusName(stat));
            exit(1);
        }

        // copy by shader
        glClear(GL_COLOR_BUFFER_BIT );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, texSrc );

        glUseProgram(program);
        glUniform1i( samplerLoc, 0 );

        glViewport( 0, 0, imgWidth, imgHeight );
        glDrawBuffer( GL_COLOR_ATTACHMENT0 );
        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

        // compare dst, src
        GLubyte* imgDst = (GLubyte*) malloc( imgWidth * imgHeight * imgChannels );
        glBindTexture( GL_TEXTURE_2D, texDst );
        glGetTexImage( GL_TEXTURE_2D, 0, imgFormat, GL_UNSIGNED_BYTE, imgDst );

        stbi_write_png("/tmp/dst3.png", imgWidth, imgHeight, imgChannels, imgDst, imgWidth * imgChannels );
        printf("dump to /tmp/dst3.png\n");
        if( memcmp(imgDst, imgSrc, imgWidth * imgHeight * imgChannels) == 0 ){
            printf("src and dst3 are the same\n\n");
        }else{
            printf("src and dst3 are diff !!!\n\n");
        }
        glErrorCheck();
    }

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
    glDeleteTextures( 1, &texSrc );
    free(imgSrc);

    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
