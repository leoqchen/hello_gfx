/*
 * 等价性测试： 顶点/颜色 上传
 * glVertexPointer + glColorPointer
 * VertexArray + VertexBuffer + glVertexAttribPointer + glEnableVertexAttribArray
 *
 */
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stddef.h>
#include "linmath.h"
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"


// settings
const unsigned int WinWidth = 800;
const unsigned int WinHeight = 600;

typedef struct Vertex{
    vec2 pos;
    vec3 col;
} Vertex;


const char *vertexShaderSource =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 330\n"
#endif
    "layout (location = 0) in vec2 vPos;\n"
    "layout (location = 1) in vec3 vCol;\n"
    "out vec3 Color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(vPos.x, vPos.y, 0.0, 1.0);\n"
    "   Color = vCol;\n"
    "}\n\0";

const char *fragmentShaderSource =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 330\n"
#endif
    "in vec3 Color;\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4( Color.r, Color.g, Color.b, 1.0f );\n"
    "}\n\0";

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    // initialize and configure
    // ------------------------------
    eglx_CreateWindow( api, WinWidth, WinHeight );

    // build and compile our shader program
    // ------------------------------------
#if !IS_GlLegacy
    const GLuint program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );
#endif

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    static const Vertex vertices[3] = {
        { { -1, -1 }, { 1, 0, 0 } },
        { {  1, -1 }, { 0, 1, 0 } },
        { {   0,  1 }, { 0, 0, 1 } }
    };
#if IS_GlLegacy
    // legacy OpenGL
    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].pos);
    glColorPointer(3, GL_FLOAT, sizeof(Vertex), &vertices[0].col);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
#else
    // modern OpenGL
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");
    printf("Attrib location: vPos=%d\n", vpos_location);
    printf("Attrib location: vCol=%d\n", vcol_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) offsetof(Vertex, pos));
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) offsetof(Vertex, col));
    glEnableVertexAttribArray(vpos_location);
    glEnableVertexAttribArray(vcol_location);
#endif

    // render loop
    // -----------
    glClearColor(0.4, 0.4, 0.4, 0.0);
    while (!eglx_ShouldClose())
    {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if IS_GlLegacy
        // legacy OpenGL
        glDrawArrays(GL_TRIANGLES, 0, 3);
#else
        // modern OpenGL
        glUseProgram(program);
        glDrawArrays(GL_TRIANGLES, 0, 3);
#endif

        // dump to disk
        // ------------
        static int i = 0;
        if( i == 0 ){
            i = 1;

            glFinish();
            GLubyte* pixels = (GLubyte*)malloc( WinWidth * WinHeight * 4 );
            glReadPixels(0, 0, WinWidth, WinHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

            // Write image Y-flipped because OpenGL
            stbi_flip_vertically_on_write( 1 );
            stbi_write_png("/tmp/glVertexPointer.png", WinWidth, WinHeight, 4, pixels, WinWidth*4);
            printf("dump to /tmp/glVertexPointer.png\n");

            stbi_write_jpg("/tmp/glVertexPointer.jpg", WinWidth, WinHeight, 4, pixels, 95);
            printf("dump to /tmp/glVertexPointer.jpg\n");

            free( pixels );
            //------------------

            if( 1 ){
                // glReadPixels by PBO
                // --------------------
                GLuint pbo;
                glGenBuffers( 1, &pbo );

                glBindBuffer( GL_PIXEL_PACK_BUFFER, pbo );
                glBufferData( GL_PIXEL_PACK_BUFFER, WinWidth * WinHeight *4, NULL, GL_DYNAMIC_READ );

                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glPixelStorei(GL_PACK_ROW_LENGTH, WinWidth);
                glReadPixels(0, 0, WinWidth, WinHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);

#if IS_GlEs
                GLubyte *pixels = (GLubyte*) glMapBufferRange( GL_PIXEL_PACK_BUFFER, 0, WinWidth * WinHeight * 4, GL_MAP_READ_BIT );
#else
                GLubyte *pixels = (GLubyte*) glMapBuffer( GL_PIXEL_PACK_BUFFER, GL_READ_ONLY );
#endif
                if( pixels ){
                    // Write image Y-flipped because OpenGL
                    stbi_flip_vertically_on_write( 1 );
                    stbi_write_png("/tmp/glVertexPointer_PBO.png", WinWidth, WinHeight, 4, pixels, WinWidth*4);
                    printf("dump to /tmp/glVertexPointer_PBO.png\n");

                    stbi_write_jpg("/tmp/glVertexPointer_PBO.jpg", WinWidth, WinHeight, 4, pixels, 95);
                    printf("dump to /tmp/glVertexPointer_PBO.jpg\n");

                    glUnmapBuffer( GL_PIXEL_PACK_BUFFER );
                }
            }
        }

        // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        eglx_SwapBuffers();
        eglx_PollEvents();
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
#if !IS_GlLegacy
    glDeleteVertexArrays( 1, &vertex_array );
    glDeleteBuffers( 1, &vertex_buffer );
    glDeleteProgram(program);
#endif

    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
