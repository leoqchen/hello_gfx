/*
 * 测试glUniform设置是否能被记录在program里，无需glDraw触发
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
const unsigned int WinHeight = 800;

typedef struct Vertex{
    vec3 pos;
    vec3 col;
} Vertex;


const char *vertexShaderSource =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 400\n"
#endif
    "uniform mat4 MVP;\n"
    "layout (location = 0) in vec3 vPos;\n"
    "layout (location = 1) in vec3 vCol;\n"
    "out vec3 Color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = MVP * vec4(vPos.x, vPos.y, vPos.z, 1.0);\n"
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

    static const float Z = 30.0f;

    // glfw: initialize and configure
    // ------------------------------
    GLFWwindow* window = glfwInit_CreateWindow( api, WinWidth, WinHeight );

    // build and compile our shader program
    // ------------------------------------
    const GLuint program1 = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );
    const GLuint program2 = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );

    // set up uniform data
    // ------------------------------------------------------------------
    GLdouble zNear = (Z == 0.0f) ? -1.0f : -Z;
    GLdouble zFar = (Z == 0.0f) ? 1.0f : Z;
    mat4x4 mvp;

    mat4x4_identity(mvp);
    mat4x4_ortho(mvp, -1.0f, 1.0f, -1.0f, 1.0f, zNear, zFar); //默认
    glUseProgram(program1);
    glUniformMatrix4fv(glGetUniformLocation(program1, "MVP"), 1, GL_FALSE, (const GLfloat*) &mvp);

    mat4x4_identity(mvp);
    mat4x4_ortho(mvp, 0.0f, 1.0f, 0.0f, 1.0f, zNear, zFar); //只显示右上角
    glUseProgram(program2);
    glUniformMatrix4fv(glGetUniformLocation(program2, "MVP"), 1, GL_FALSE, (const GLfloat*) &mvp);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    static const Vertex vertices[] = {
        { { 0.25, 0.25, Z }, { 1, 0, 0 } },
        { { 0.75, 0.25, Z }, { 1, 1, 0 } },
        { { 0.75, 0.75, Z }, { 1, 0, 1 } },
        { { 0.25, 0.75, Z }, { 0, 1, 1 } },
    };
    // modern OpenGL
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) offsetof(Vertex, pos));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) offsetof(Vertex, col));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // render loop
    // -----------
    int frame = 0;
    glClearColor(0.0, 0.0, 0.0, 0.0);
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // set up mvp matrix
        // -------------------
        if( frame == 0 ){
            glUseProgram(program1);
        }else if( frame == 90 ){
            glUseProgram(program2);
        }else if( frame == 180 ){
            frame = -1;
        }
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
        frame++;
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays( 1, &vertex_array );
    glDeleteBuffers( 1, &vertex_buffer );
    glDeleteProgram(program1);
    glDeleteProgram(program2);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}