/**
 * Measure simple vertex processing rate via:
 *  - immediate mode
 *  - vertex arrays
 *  - VBO vertex arrays
 *  - glDrawElements
 *  - VBO glDrawElements
 *  - glDrawRangeElements
 *  - VBO glDrawRangeElements
 */
#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "glutils.h"
#include "glfwutils.h"
#include "myutils.h"


// settings
static const int WinWidth = 500;
static const int WinHeight = 500;
static GLFWwindow* window;

#define MAX_VERTS (100 * 100)

/** glVertex2/3/4 size */
#define VERT_SIZE 4

static GLuint VAO;
static GLuint VertexBO, ElementBO;
static GLuint program;
static GLint vPos_location;

static unsigned NumVerts = MAX_VERTS;
static unsigned VertBytes = VERT_SIZE * sizeof(float);
static float *VertexData = NULL;
static float *VertexData2 = NULL;

static unsigned NumElements = MAX_VERTS;
static GLuint *Elements = NULL;

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


static float* GenerateVertexData( float N )
{
    float x = -N;
    float y = -N;
    float dx = 2 * N / 100;
    float dy = 2 * N / 100;

    float* data = (float *) malloc(NumVerts * VertBytes);
    for (int i = 0; i < NumVerts; i++) {
        data[i * VERT_SIZE + 0] = x;
        data[i * VERT_SIZE + 1] = y;
        data[i * VERT_SIZE + 2] = 0.0;
        data[i * VERT_SIZE + 3] = 1.0;
        x += dx;
        if (x > N) {
            x = -N;
            y += dy;
        }
    }
    return data;
}

/**
 * Load VertexData buffer with a 2-D grid of points in the range [-1,1]^2.
 */
static void InitializeVertexData()
{
    VertexData = GenerateVertexData( 0.8 );
    VertexData2 = GenerateVertexData( 0.4 );

    Elements = (GLuint *) malloc(NumVerts * sizeof(GLuint));
    for (int i = 0; i < NumVerts; i++) {
        Elements[i] = NumVerts - i - 1;
    }
}

void PerfInit()
{
    // build and compile our shader program
    // ------------------------------------
#if !IS_GlLegacy
    program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );
    glUseProgram(program);
#endif

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    InitializeVertexData();

#if IS_GlLegacy
    /* setup VertexBO */
    glGenBuffers(1, &VertexBO);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBO);
    glBufferData(GL_ARRAY_BUFFER, NumVerts * VertBytes, VertexData, GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);

    /* setup ElementBO */
    glGenBuffers(1, &ElementBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    NumElements * sizeof(GLuint), Elements, GL_STATIC_DRAW);
#else
    /*
     * 关于VAO:
     * 1. DrawArraysMem()测试里的glVertexAttribPointer需要工作在Client模式
     *   a) OpenGL core profile 不支持该模式
     *   b) OpenGL ES 支持该模式，方法是：
     *        glBindVertexArray( 0 );             //注意, OpenGL core profile 不支持绑定0到VAO
     *        glBindBuffer( GL_ARRAY_BUFFER, 0 );
     *      并且，OpenGL ES的默认VAO是0
     */
    #if IS_GlEs
    VAO = 0;
    #else
    glGenVertexArrays(1, &VAO);
    #endif
    glBindVertexArray(VAO);

    /* setup VertexBO */
    glGenBuffers(1, &VertexBO);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBO);
    glBufferData(GL_ARRAY_BUFFER, NumVerts * VertBytes, VertexData, GL_STATIC_DRAW);

    vPos_location = glGetAttribLocation(program, "vPos");
    printf("Attrib location: vPos=%d\n", vPos_location);
    glEnableVertexAttribArray(vPos_location);

    /* setup ElementBO */
    glGenBuffers(1, &ElementBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, NumElements * sizeof(GLuint), Elements, GL_STATIC_DRAW);
#endif

    // misc GL state
    // ------------------------------------------------------------------

}

#if IS_GlLegacy
static void DrawImmediate(unsigned count)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for (int i = 0; i < count; i++) {
        glBegin(GL_POINTS);

        for (int j = 0; j < NumVerts; j++) {
#if VERT_SIZE == 4
            glVertex4fv(VertexData + j * 4);
#elif VERT_SIZE == 3
            glVertex3fv(VertexData + j * 3);
#elif VERT_SIZE == 2
         glVertex2fv(VertexData + j * 2);
#else
         abort();
#endif
        }

        glEnd();
    }

    glFinish();
    glfwSwapBuffers(window);
}
#endif

static void DrawArraysMem(unsigned count)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Client模式读取VertexData2，以便与非Client模式做区别
#if IS_GlLegacy
    glVertexPointer(VERT_SIZE, GL_FLOAT, VertBytes, VertexData2);
#else
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE, VertBytes, VertexData2);
#endif

    for (int i = 0; i < count; i++) {
        glDrawArrays(GL_POINTS, 0, NumVerts);
    }

    glFinish();
    glfwSwapBuffers(window);
}

static void DrawArraysVBO(unsigned count)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBO);

#if IS_GlLegacy
    glVertexPointer(VERT_SIZE, GL_FLOAT, VertBytes, (void *) 0);
#else
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE, VertBytes, (void*) 0);
#endif

    for (int i = 0; i < count; i++) {
        glDrawArrays(GL_POINTS, 0, NumVerts);
    }

    glFinish();
    glfwSwapBuffers(window);
}

static void DrawElementsMem(unsigned count)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Client模式读取VertexData2，以便与非Client模式做区别
#if IS_GlLegacy
    glVertexPointer(VERT_SIZE, GL_FLOAT, VertBytes, VertexData2);
#else
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE, VertBytes, VertexData2);
#endif

    for (int i = 0; i < count; i++) {
        glDrawElements(GL_POINTS, NumVerts, GL_UNSIGNED_INT, Elements);
    }

    glFinish();
    glfwSwapBuffers(window);
}

static void DrawElementsBO(unsigned count)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBO);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBO);

#if IS_GlLegacy
    glVertexPointer(VERT_SIZE, GL_FLOAT, VertBytes, (void *) 0);
#else
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE, VertBytes, (void*) 0);
#endif

    for (int i = 0; i < count; i++) {
        glDrawElements(GL_POINTS, NumVerts, GL_UNSIGNED_INT, (void *) 0);
    }

    glFinish();
    glfwSwapBuffers(window);
}

static void DrawRangeElementsMem(unsigned count)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Client模式读取VertexData2，以便与非Client模式做区别
#if IS_GlLegacy
    glVertexPointer(VERT_SIZE, GL_FLOAT, VertBytes, VertexData2);
#else
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE, VertBytes, VertexData2);
#endif

    for (int i = 0; i < count; i++) {
        glDrawRangeElements(GL_POINTS, 0, NumVerts - 1,
                            NumVerts, GL_UNSIGNED_INT, Elements);
    }

    glFinish();
    glfwSwapBuffers(window);
}

static void DrawRangeElementsBO(unsigned count)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBO);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBO);

#if IS_GlLegacy
    glVertexPointer(VERT_SIZE, GL_FLOAT, VertBytes, (void *) 0);
#else
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE, VertBytes, (void*) 0);
#endif

    for (int i = 0; i < count; i++) {
        glDrawRangeElements(GL_POINTS, 0, NumVerts - 1,
                            NumVerts, GL_UNSIGNED_INT, (void *) 0);
    }

    glFinish();
    glfwSwapBuffers(window);
}

static void PerfDraw()
{
    double rate;
    printf("Vertex rate (%d x Vertex%df)\n", NumVerts, VERT_SIZE);

#if IS_GlLegacy
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rate = PerfMeasureRate(DrawImmediate);
    rate *= NumVerts;
    printf("  Immediate mode: %s verts/sec\n", PerfHumanFloat(rate));
#endif

#if IS_GlLegacy || IS_GlEs
    // OpenGL Core profile 不支持让VBO工作在client模式
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rate = PerfMeasureRate(DrawArraysMem);
    rate *= NumVerts;
    printf("  glDrawArrays: %s verts/sec\n", PerfHumanFloat(rate));
#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rate = PerfMeasureRate(DrawArraysVBO);
    rate *= NumVerts;
    printf("  VBO glDrawArrays: %s verts/sec\n", PerfHumanFloat(rate));

#if IS_GlLegacy || IS_GlEs
    // OpenGL Core profile 不支持让VBO工作在client模式
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rate = PerfMeasureRate(DrawElementsMem);
    rate *= NumVerts;
    printf("  glDrawElements: %s verts/sec\n", PerfHumanFloat(rate));
#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rate = PerfMeasureRate(DrawElementsBO);
    rate *= NumVerts;
    printf("  VBO glDrawElements: %s verts/sec\n", PerfHumanFloat(rate));

#if IS_GlLegacy || IS_GlEs
    // OpenGL Core profile 不支持让VBO工作在client模式
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rate = PerfMeasureRate(DrawRangeElementsMem);
    rate *= NumVerts;
    printf("  glDrawRangeElements: %s verts/sec\n", PerfHumanFloat(rate));
#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rate = PerfMeasureRate(DrawRangeElementsBO);
    rate *= NumVerts;
    printf("  VBO glDrawRangeElements: %s verts/sec\n", PerfHumanFloat(rate));

    glErrorCheck();
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
        PerfDraw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
