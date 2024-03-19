/*
 * 行为测试：
 * 测试glUniform设置是否能被记录在program里，无需glDraw触发
 */
#include <stdio.h>
#include "linmath.h"
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"


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
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    static const float Z = 30.0f;

    // initialize and configure
    // ------------------------------
    eglx_CreateWindow( api, WinWidth, WinHeight );

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
    while (!eglx_ShouldClose())
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

        // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        eglx_SwapBuffers();
        eglx_PollEvents();
        frame++;
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays( 1, &vertex_array );
    glDeleteBuffers( 1, &vertex_buffer );
    glDeleteProgram(program1);
    glDeleteProgram(program2);

    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
