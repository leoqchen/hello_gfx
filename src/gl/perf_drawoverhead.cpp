/**
 * Measure drawing overhead
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
static const int WinWidth = 200;
static const int WinHeight = 200;
static GLFWwindow* window;

static GLuint VAO;
static GLuint VBO;
static GLuint program;

struct vertex
{
    GLfloat x, y;
};

static const struct vertex vertices[] = {
    { -0.5, -0.5 },
    {  0.5, -0.5 },
    {  0.0,  0.5 },
};


const char *vertexShaderSource =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 400\n"
#endif
    "layout (location = 0) in vec2 vPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4( vPos.x, vPos.y, 0.0f, 1.0f );\n"
#if IS_GlEs
    "   gl_PointSize = 1.0;\n" // make IMG gpu happy
#endif
    "}\n\0";

const char *fragmentShaderSource =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 400\n"
#endif
    "layout (location = 0) out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "   outColor = vec4( 1.0f, 1.0f, 1.0f, 1.0f );\n"
    "}\n\0";

static void PerfInit()
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
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    glFinish();
}


static void DrawNopStateChange(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        //glDisable(GL_ALPHA_TEST);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, 3);
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
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    glFinish();
}

static void PerfDraw(int mode)
{
    double rate0, rate1, rate2, overhead;

    if( mode == -1 || mode == 0 ) {
        rate0 = PerfMeasureRate(DrawNoStateChange, glfwPollEvents );
        printf("   Draw only: %s draws/second\n", PerfHumanFloat(rate0));
        glfwSwapBuffers(window);
    }

    if( mode == -1 || mode == 1 ) {
        rate1 = PerfMeasureRate(DrawNopStateChange, glfwPollEvents );
        overhead = 1000.0 * (1.0 / rate1 - 1.0 / rate0);
        printf("   Draw w/ nop state change: %s draws/sec (overhead: %f ms/draw)\n", PerfHumanFloat(rate1), overhead);
        glfwSwapBuffers(window);
    }

    if( mode == -1 || mode == 2 ) {
        rate2 = PerfMeasureRate(DrawStateChange, glfwPollEvents );
        overhead = 1000.0 * (1.0 / rate2 - 1.0 / rate0);
        printf("   Draw w/ state change: %s draws/sec (overhead: %f ms/draw)\n", PerfHumanFloat(rate2), overhead);
        glfwSwapBuffers(window);
    }

    glErrorCheck();
    exit(0);
}

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    int __mode = integerFromArgs("--mode", argc, argv, NULL );

    // glfw: initialize and configure
    // ------------------------------
    window = glfwInit_CreateWindow( api, WinWidth, WinHeight );

    // init
    // -----------
    PerfInit();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        PerfDraw( __mode );

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
