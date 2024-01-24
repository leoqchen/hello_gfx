/*
 * 分别用 Legacy OpenGL、Modern OpenGL 这两种不同写法实现顶点上传、颜色上传
 *
 * glVertexPointer 等价于 glVertexAttribPointer( index = 0 )
 * glColorPointer 无法用 glVertexAttribPointer()等价实现
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
const unsigned int WinWidth = 800;
const unsigned int WinHeight = 600;

typedef struct Vertex{
    vec2 pos;
    vec3 col;
} Vertex;


#if IS_GlEs
const char *vertexShaderSource =
    "#version 320 es\n"
    "layout (location = 0) in vec2 vPos;\n"
    "layout (location = 1) in vec3 vCol;\n"
    "out vec3 Color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(vPos.x, vPos.y, 0.0, 1.0);\n"
    "   Color = vCol;\n"
    "}\n\0";

const char *fragmentShaderSource =
    "#version 320 es\n"
    "precision mediump float;\n"
    "in vec3 Color;\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4( Color.r, Color.g, Color.b, 1.0f );\n"
    "}\n\0";
#else
const char *vertexShaderSource =
    "#version 400\n"
    "layout (location = 0) in vec2 vPos;\n"
    "layout (location = 1) in vec3 vCol;\n"
    "out vec3 Color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(vPos.x, vPos.y, 0.0, 1.0);\n"
    "   Color = vCol;\n"
    "}\n\0";

const char *fragmentShaderSource =
    "#version 400\n"
    "in vec3 Color;\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4( Color.r, Color.g, Color.b, 1.0f );\n"
    "}\n\0";
#endif

int main( int argc, const char* argv[] )
{
#if IS_GlEs
    api_t api = {.api = API_GLES, .major = 3, .minor = 2};
#elif IS_GlLegacy
    api_t api = {.api = API_GLLegacy, .major = 3, .minor = 0};
#else
    api_t api = {.api = API_GL, .major = 3, .minor = 3};
#endif
    if( argc >= 2 && isdigit(argv[1][0]) )
        api.major = argv[1][0] - '0';
    if( argc >= 3 && isdigit(argv[2][0]) )
        api.minor = argv[2][0] - '0';
    printf("command line: major = %d, minor = %d\n", api.major, api.minor);

    // glfw: initialize and configure
    // ------------------------------
    GLFWwindow* window = glfwInit_CreateWindow( api, WinWidth, WinHeight );

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
    while (!glfwWindowShouldClose(window))
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

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
#if !IS_GlLegacy
    glDeleteVertexArrays( 1, &vertex_array );
    glDeleteBuffers( 1, &vertex_buffer );
    glDeleteProgram(program);
#endif

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}