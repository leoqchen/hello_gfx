#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <ctype.h>
#include "glutils.h"
#include "glfwutils.h"
#include "myutils.h"


// settings
const int WinWidth = 200;
const int WinHeight = 200;

static GLuint VAO;
static GLuint VBO;
static GLuint program;

struct vertex
{
    GLfloat x, y;
};

static const struct vertex vertices[4] = {
    { -1.0, -1.0 },
    {  1.0, -1.0 },
    {  1.0,  1.0 },
    { -1.0,  1.0 }
};


#if IS_GlEs
const char *vertexShaderSource =
    "#version 320 es\n"
    "layout (location = 0) in vec2 vPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4( vPos.x, vPos.y, 0.0f, 1.0f );\n"
    "}\n\0";

const char *fragmentShaderSource =
    "#version 320 es\n"
    "precision mediump float;\n"
    "layout (location = 0) out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "   outColor = vec4( 1.0f, 0.0f, 0.0f, 1.0f );\n"
    "}\n\0";
#else
const char *vertexShaderSource =
    "#version 400\n"
    "layout (location = 0) in vec2 vPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4( vPos.x, vPos.y, 0.0f, 1.0f );\n"
    "}\n\0";

const char *fragmentShaderSource =
    "#version 400\n"
    "layout (location = 0) out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "   outColor = vec4( 1.0f, 0.0f, 0.0f, 1.0f );\n"
    "}\n\0";
#endif

static void PerfInit(void)
{
    // build and compile our shader program
    // ------------------------------------
#if !IS_GlLegacy
    program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );
    glUseProgram(program);
#endif

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
#if IS_GlLegacy
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexPointer(2, GL_FLOAT, sizeof(struct vertex), (void *)0);
    glEnableClientState(GL_VERTEX_ARRAY);
#else
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLint vPos_location = glGetAttribLocation(program, "vPos");
    printf("Attrib location: vPos=%d\n", vPos_location);
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct vertex), (void*) 0);
    glEnableVertexAttribArray(vPos_location);
#endif

    // misc GL state
    // ------------------------------------------------------------------
    //glAlphaFunc(GL_ALWAYS, 0.0);
    glDepthFunc(GL_LESS);
}

static void DrawNoStateChange(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        glDrawArrays(GL_POINTS, 0, 4);
    }
    glFinish();
}


static void DrawNopStateChange(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        //glDisable(GL_ALPHA_TEST);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_POINTS, 0, 4);
    }
    glFinish();
}


static void DrawStateChange(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        if (i & 1){
            //glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_DEPTH_TEST);
        }else{
            //glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_DEPTH_TEST);
        }
        glDrawArrays(GL_POINTS, 0, 4);
    }
    glFinish();
}

static void PerfDraw()
{
    double rate0, rate1, rate2, overhead;

    rate0 = PerfMeasureRate(DrawNoStateChange);
    printf("   Draw only: %s draws/second\n", PerfHumanFloat(rate0));

    rate1 = PerfMeasureRate(DrawNopStateChange);
    overhead = 1000.0 * (1.0 / rate1 - 1.0 / rate0);
    printf("   Draw w/ nop state change: %s draws/sec (overhead: %f ms/draw)\n", PerfHumanFloat(rate1), overhead);

    rate2 = PerfMeasureRate(DrawStateChange);
    overhead = 1000.0 * (1.0 / rate2 - 1.0 / rate0);
    printf("   Draw w/ state change: %s draws/sec (overhead: %f ms/draw)\n", PerfHumanFloat(rate2), overhead);

    exit(0);
}

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

    // init
    // -----------
    PerfInit();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        PerfDraw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
