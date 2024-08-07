/*
 * 等价性测试：
 * glRotate, glOrtho 的等价实现
 */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "linmath.h"
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"

typedef struct Vertex
{
    vec2 pos;
    vec3 col;
} Vertex;

static const Vertex vertices[3] =
{
    { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
    { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
    { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }
};

static const char* vertex_shader_text =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 330\n"
#endif
    "uniform mat4 MVP;\n"
    "layout (location = 0) in vec3 vCol;\n"
    "layout (location = 1) in vec2 vPos;\n"
    "out vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
    "    color = vCol;\n"
    "}\n";

static const char* fragment_shader_text =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 330\n"
#endif
    "in vec3 color;\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(color, 1.0);\n"
    "}\n";


int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    int __frame = integerFromArgs("--f", argc, argv, NULL );

    // initialize and configure
    // ------------------------------
    eglx_CreateWindow( api, 640, 480 );

#if !IS_GlLegacy
    // build and compile our shader program
    // ------------------------------------
    const GLuint program = CreateProgramFromSource( vertex_shader_text, fragment_shader_text );

    // set up uniform data
    // ------------------------------------------------------------------
    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    printf("Uniform location: MVP=%d\n", mvp_location);
#endif

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
#if IS_GlLegacy
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &vertices[0].pos);
    glColorPointer(3, GL_FLOAT, sizeof(Vertex), &vertices[0].col);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
#else
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
                          sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, col));
    glEnableVertexAttribArray(vpos_location);
    glEnableVertexAttribArray(vcol_location);
#endif


    // render loop
    // -----------
    int frame_count = 0;
    while (!eglx_ShouldClose())
    {
        int width, height;
        eglx_GetWindowSize( &width, &height);
        const float ratio = width / (float) height;

        glClear(GL_COLOR_BUFFER_BIT);

#if IS_GlLegacy
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef( DegreeFromRadian( (float)PerfGetSecond() ), 0.0, 0.0, 1.0 );
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ratio, ratio, -1.0, 1.0, 1.0, -1.0);
#else
        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float) PerfGetSecond());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);

        glBindVertexArray(vertex_array);
#endif

        glDrawArrays(GL_TRIANGLES, 0, 3);

        // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        eglx_SwapBuffers();
        eglx_PollEvents();

        frame_count++;
        if( __frame != -1 && frame_count >= __frame ){
            printf("frame count: %d\n", frame_count);
            break;
        }
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
#if !IS_GlLegacy
    glDeleteVertexArrays( 1, &vertex_array );
    glDeleteBuffers( 1, &vertex_buffer );
    glDeleteProgram( program );
#endif

    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    exit(EXIT_SUCCESS);
}

