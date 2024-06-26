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
#include <stdio.h>
#include <string.h>
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"


// settings
static const int WinWidth = 500;
static const int WinHeight = 500;


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
    "#version 330\n"
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
    "#version 330\n"
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
    eglx_SwapBuffers();
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
    eglx_SwapBuffers();
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
    eglx_SwapBuffers();
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
    eglx_SwapBuffers();
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
    eglx_SwapBuffers();
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
    eglx_SwapBuffers();
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
    eglx_SwapBuffers();
}

static void PerfDraw( int mode )
{
    double rate;
    printf("Vertex rate (%d x Vertex%df)\n", NumVerts, VERT_SIZE);

#if IS_GlLegacy
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rate = PerfMeasureRate(DrawImmediate, eglx_PollEvents );
    rate *= NumVerts;
    printf("  Immediate mode: %s verts/sec\n", PerfHumanFloat(rate));
#endif

#if IS_GlLegacy || IS_GlEs
    if( mode == -1 || mode == 0 ) {
        // OpenGL Core profile 不支持让VBO工作在client模式
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rate = PerfMeasureRate(DrawArraysMem, eglx_PollEvents );
        rate *= NumVerts;
        printf("  glDrawArrays: %s verts/sec\n", PerfHumanFloat(rate));
    }
#endif

    if( mode == -1 || mode == 1 ) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rate = PerfMeasureRate(DrawArraysVBO, eglx_PollEvents );
        rate *= NumVerts;
        printf("  VBO glDrawArrays: %s verts/sec\n", PerfHumanFloat(rate));
    }

#if IS_GlLegacy || IS_GlEs
    if( mode == -1 || mode == 2 ) {
        // OpenGL Core profile 不支持让VBO工作在client模式
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rate = PerfMeasureRate(DrawElementsMem, eglx_PollEvents );
        rate *= NumVerts;
        printf("  glDrawElements: %s verts/sec\n", PerfHumanFloat(rate));
    }
#endif

    if( mode == -1 || mode == 3 ) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rate = PerfMeasureRate(DrawElementsBO, eglx_PollEvents );
        rate *= NumVerts;
        printf("  VBO glDrawElements: %s verts/sec\n", PerfHumanFloat(rate));
    }

#if IS_GlLegacy || IS_GlEs
    if( mode == -1 || mode == 4 ) {
        // OpenGL Core profile 不支持让VBO工作在client模式
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rate = PerfMeasureRate(DrawRangeElementsMem, eglx_PollEvents );
        rate *= NumVerts;
        printf("  glDrawRangeElements: %s verts/sec\n", PerfHumanFloat(rate));
    }
#endif

    if( mode == -1 || mode == 5 ) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rate = PerfMeasureRate(DrawRangeElementsBO, eglx_PollEvents );
        rate *= NumVerts;
        printf("  VBO glDrawRangeElements: %s verts/sec\n", PerfHumanFloat(rate));
    }

    glErrorCheck();
    exit(0);
}

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    int __mode = integerFromArgs("--mode", argc, argv, NULL );

    // initialize and configure
    // ------------------------------
    eglx_CreateWindow( api, WinWidth, WinHeight );

    // init
    // -----------
    PerfInit();

    // render loop
    // -----------
    while (!eglx_ShouldClose())
    {
        // render
        // ------
        PerfDraw( __mode );

        // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        eglx_SwapBuffers();
        eglx_PollEvents();
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------


    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
