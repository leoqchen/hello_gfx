/*
 * 行为测试：
 * 用多个Vertex Buffer来传输 顶点数据、颜色数据
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
#include "linmath.h"
#include "glutils.h"
#include "glfwutils.h"
#include "myutils.h"


// settings
static const int WinWidth = 800;
static const int WinHeight = 600;

//typedef struct Vertex{
//    vec2 pos;
//    vec3 col;
//} Vertex;


const char *vertexShaderSource =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 400\n"
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
    "#version 400\n"
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

    // glfw: initialize and configure
    // ------------------------------
    GLFWwindow* window = glfwInit_CreateWindow( api, WinWidth, WinHeight );

    // build and compile our shader program
    // ------------------------------------
    const GLuint program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
//    static const Vertex vertices[3] = {
//        { { -1, -1 }, { 1, 0, 0 } },
//        { {  1, -1 }, { 0, 1, 0 } },
//        { {  0,  1 }, { 0, 0, 1 } }
//    };
    static const vec2 vPos[3] = {
        { -1, -1 },
        {  1, -1 },
        {   0,  1 },
    };
    static const vec3 vCol[3] = {
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 },
    };
//    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].pos);
//    glColorPointer(3, GL_FLOAT, sizeof(Vertex), &vertices[0].col);
//    glEnableClientState(GL_VERTEX_ARRAY);
//    glEnableClientState(GL_COLOR_ARRAY);
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");
    printf("Attrib location: vPos=%d\n", vpos_location);
    printf("Attrib location: vCol=%d\n", vcol_location);

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    GLuint vertex_buffer[2];
    glGenBuffers( 2, vertex_buffer );

    glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(vPos), vPos, GL_STATIC_DRAW );
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vec2), 0);
    glEnableVertexAttribArray(vpos_location);

    glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer[1] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(vCol), vCol, GL_STATIC_DRAW );
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vec3), 0);
    glEnableVertexAttribArray(vcol_location);

    // render loop
    // -----------
    glClearColor(0.4, 0.4, 0.4, 0.0);
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays( 1, &vertex_array );
    glDeleteBuffers( 2, vertex_buffer );
    glDeleteProgram(program);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
