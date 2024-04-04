/*
 * 等价性测试： 顶点/纹理坐标 上传
 * glVertexPointer + glTexCoordPointer
 * VertexArray + VertexBuffer + glVertexAttribPointer + glEnableVertexAttribArray
 *
 */
#include <stdio.h>
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"

// settings
static const int WinWidth = 800;
static const int WinHeight = 600;


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
    "   v_texCoord = vTexCoord;\n"
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

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    const char* __file = stringFromArgs("--file", argc, argv );
    const char *imgFile = (__file) ? __file : PROJECT_SOURCE_DIR "data/basemap.tga";
    int imgWidth, imgHeight;
    GLenum imgFormat;
    GLubyte *imgData = imageFromFile( imgFile, &imgWidth, &imgHeight, &imgFormat, NULL );

    // initialize and configure
    // ------------------------------
    eglx_CreateWindow( api, WinWidth, WinHeight );

    // build and compile our shader program
    // ------------------------------------
    const GLuint program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );
    GLint samplerLoc = glGetUniformLocation( program, "s_texture" );

    // set up texture data and configure texture attributes
    // ------------------------------------------------------------------
    GLuint textureId;
    glGenTextures( 1, &textureId );
    glBindTexture( GL_TEXTURE_2D, textureId );
    glTexImage2D( GL_TEXTURE_2D, 0, imgFormat, imgWidth, imgHeight, 0, imgFormat, GL_UNSIGNED_BYTE, imgData );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
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
    printf("Attrib location: vPos=%d\n", vPos_location);
    printf("Attrib location: vTexCoord=%d\n", vTexCoord_location);
    glVertexAttribPointer(vPos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat) * 5, (void*)0);
    glVertexAttribPointer(vTexCoord_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat) * 5, (void *) (sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(vPos_location);
    glEnableVertexAttribArray(vTexCoord_location);

    // render loop
    // -----------
    while (!eglx_ShouldClose())
    {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, textureId );

        glUseProgram(program);

        // set the sampler texture unit to 0
        glUniform1i( samplerLoc, 0 );

        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

        // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        eglx_SwapBuffers();
        eglx_PollEvents();
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteTextures( 1, &textureId );
    glDeleteProgram(program);
    free( imgData );

    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
