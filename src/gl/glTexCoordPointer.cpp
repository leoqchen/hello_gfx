/*
 * 等价性测试： 顶点/纹理坐标 上传
 * glVertexPointer + glTexCoordPointer
 * VertexArray + VertexBuffer + glVertexAttribPointer + glEnableVertexAttribArray
 *
 */

#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <ctype.h>
#include "glUtils.h"
#include "glfwUtils.h"
#include "myUtils.h"

// settings
static const int WinWidth = 800;
static const int WinHeight = 600;


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
    "   v_texCoord = vTexCoord;\n"
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

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    // glfw: initialize and configure
    // ------------------------------
    GLFWwindow* window = glfwInit_CreateWindow( api, WinWidth, WinHeight );

    // build and compile our shader program
    // ------------------------------------
#if !IS_GlLegacy
    const GLuint program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );
    GLint samplerLoc = glGetUniformLocation( program, "s_texture" );
#endif

    // set up texture data and configure texture attributes
    // ------------------------------------------------------------------
    // Create a 2x2 texture image
    GLuint textureId;
    static const GLubyte pixels[4 * 3] = {
        255,   0,   0, // Red
        0, 255,   0, // Green
        0,   0, 255, // Blue
        255, 255,   0  // Yellow
    };
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glGenTextures( 1, &textureId );
    glBindTexture( GL_TEXTURE_2D, textureId );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    static const GLfloat vVertices[] = {
        -0.5f,  0.5f, 0.0f,  // Position 0
        0.0f,  0.0f,        // TexCoord 0
        -0.5f, -0.5f, 0.0f,  // Position 1
        0.0f,  1.0f,        // TexCoord 1
        0.5f, -0.5f, 0.0f,  // Position 2
        1.0f,  1.0f,        // TexCoord 2
        0.5f,  0.5f, 0.0f,  // Position 3
        1.0f,  0.0f         // TexCoord 3
    };
    static const GLushort indices[] = {
        0, 1, 2, 0, 2, 3
    };
#if IS_GlLegacy
    // legacy OpenGL
    glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), vVertices );
    glTexCoordPointer( 2, GL_FLOAT, 5 * sizeof(GLfloat), &vVertices[3] );
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#else
    // modern OpenGL
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
#endif

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT );

#if IS_GlLegacy
        // legacy OpenGL
        glBindTexture( GL_TEXTURE_2D, textureId );
        glEnable(GL_TEXTURE_2D);

        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
#else
        // modern OpenGL
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, textureId );

        glUseProgram(program);

        // set the sampler texture unit to 0
        glUniform1i( samplerLoc, 0 );

        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
#endif

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteTextures( 1, &textureId );
#if !IS_GlLegacy
    glDeleteProgram(program);
#endif

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
