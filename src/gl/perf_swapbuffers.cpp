/**
 * Measure SwapBuffers.
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
#include <sched.h>
#include "glutils.h"
#include "glfwutils.h"
#include "myutils.h"


// settings
static int WinWidth = 100;
static int WinHeight = 100;
static GLFWwindow* window;

static GLuint VAO;
static GLuint VBO;
static GLuint program;

struct vertex
{
    GLfloat x, y;
};

static const struct vertex vertices[4] = {
    { -0.5, -0.5 },
    {  0.5, -0.5 },
    {  0.5,  0.5 },
    { -0.5,  0.5 }
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexPointer(2, GL_FLOAT, sizeof(struct vertex), (void *) 0);
    glEnableClientState(GL_VERTEX_ARRAY);
#else
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLint vPos_location = glGetAttribLocation(program, "vPos");
    printf("Attrib location: vPos=%d\n", vPos_location);
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*) 0);
    glEnableVertexAttribArray(vPos_location);
#endif

    // misc GL state
    // ------------------------------------------------------------------
}

static void SwapNaked(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        glfwSwapBuffers(window);
    }
}


static void SwapClear(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
    }
}

static void SwapClearPoint(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, 4);
        glfwSwapBuffers(window);
    }
}

static const struct {
    unsigned w;
    unsigned h;
} sizes[] = {
    { 320, 240 },
    { 640, 480 },
    { 1024, 768 },
    { 1200, 1024 },
    { 1600, 1200 }
};

static void PerfDraw()
{
    int w, h;
    glfwGetWindowSize( window, &w, &h );
    printf("Window %dx%d\n", w, h);

    double rate0;
    rate0 = PerfMeasureRate(SwapNaked);
    printf("   Swapbuffers      %dx%d: %s swaps/second, %s pixels/second\n",
           w, h,
           PerfHumanFloat(rate0),
           PerfHumanFloat(rate0 * w * h) );

    rate0 = PerfMeasureRate(SwapClear);
    printf("   Swap/Clear       %dx%d: %s swaps/second, %s pixels/second\n",
           w, h,
           PerfHumanFloat(rate0),
           PerfHumanFloat(rate0 * w * h) );


    rate0 = PerfMeasureRate(SwapClearPoint);
    printf("   Swap/Clear/Draw  %dx%d: %s swaps/second, %s pixels/second\n",
           w, h,
           PerfHumanFloat(rate0),
           PerfHumanFloat(rate0 * w * h) );
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
    WinWidth = sizes[0].w;
    WinHeight = sizes[0].h;
    window = glfwInit_CreateWindow( api, WinWidth, WinHeight );

    // init
    // -----------
    PerfInit();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        for( int i=0; i < (sizeof(sizes)/sizeof(sizes[0])); i++ ){
            // change window size
            // ------------------
            glfwSetWindowSize( window, sizes[i].w, sizes[i].h );

            //wait window size change take effect
            int n = 0;
            int w, h;
            while(1){
                sched_yield();

                n++;
                glfwGetWindowSize( window, &w, &h );
                if( (w == sizes[i].w && h == sizes[i].h) || (n >= 10000) )
                    break;
            }

            // render
            // ------
            PerfDraw();

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        exit( 0 );
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
