/*
 * 等价性测试：
 * glOrtho 的等价实现
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
#include "glUtils.h"
#include "glfwUtils.h"
#include "myUtils.h"


// settings
static const int WinWidth = 800;
static const int WinHeight = 800;

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
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    // glfw: initialize and configure
    // ------------------------------
    GLFWwindow* window = glfw_CreateWindow(api, WinWidth, WinHeight);

#if !IS_GlLegacy
    // build and compile our shader program
    // ------------------------------------
    const GLuint program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );

    // set up uniform data
    // ------------------------------------------------------------------
    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    printf("Uniform location: MVP=%d\n", mvp_location);
#endif

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    static const float Z = 30.0f;
    static const Vertex vertices[] = {
        { { 0.25, 0.25, Z }, { 1, 0, 0 } },
        { { 0.75, 0.25, Z }, { 1, 1, 0 } },
        { { 0.75, 0.75, Z }, { 1, 0, 1 } },
        { { 0.25, 0.75, Z }, { 0, 1, 1 } },
    };
#if IS_GlLegacy
    // legacy OpenGL
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &vertices[0].pos);
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
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) offsetof(Vertex, pos));
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) offsetof(Vertex, col));
    glEnableVertexAttribArray(vpos_location);
    glEnableVertexAttribArray(vcol_location);
#endif

    // render loop
    // -----------
    int frame = 0;
    GLdouble zNear = (Z == 0.0f) ? -1.0f : -Z;
    GLdouble zFar = (Z == 0.0f) ? 1.0f : Z;
    GLdouble left = -1.0f;
    GLdouble right = 1.0f;
    GLdouble bottom = -1.0f;
    GLdouble top = 1.0f;

    glClearColor(0.0, 0.0, 0.0, 0.0);
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // set up mvp matrix
        // -------------------
        if( frame == 0 ){
            //只显示右上角
            left = 0.0f;
            bottom = 0.0f;
        }else if( frame == 90 ){
            //恢复默认
            left = -1.0f;
            bottom = -1.0f;
        }else if( frame == 180 ){
            frame = -1;
        }
#if IS_GlLegacy
        // legacy OpenGL
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(left, right, bottom, top, zNear, zFar);
#else
        // modern OpenGL
        mat4x4 mvp;
        mat4x4_identity(mvp);
        mat4x4_ortho(mvp, left, right, bottom, top, zNear, zFar);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
#endif
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
        frame++;
    }
    glErrorCheck();

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
