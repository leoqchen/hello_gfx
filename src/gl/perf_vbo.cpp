/**
 * Measure VBO upload speed.
 * That is, measure glBufferData() and glBufferSubData().
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
#include <string.h>
#include "glUtils.h"
#include "glfwUtils.h"
#include "myUtils.h"


// settings
static const int WinWidth = 200;
static const int WinHeight = 200;
static GLFWwindow* window;

// Copy data out of a large array to avoid caching effects:
#define DATA_SIZE (16*1024*1024)

static GLuint VAO;
static GLuint VBO;
static GLuint program;
static GLsizei VBOSize = 0;
static GLsizei SubSize = 0;
static GLubyte *VBOData = NULL;  /* array[DATA_SIZE] */
static GLint vPos_location;

static const GLfloat Vertex0[2] = { 0.0, 0.0 };

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

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex0), (void *) 0);
    glEnableClientState(GL_VERTEX_ARRAY);
#else
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    vPos_location = glGetAttribLocation(program, "vPos");
    printf("Attrib location: vPos=%d\n", vPos_location);
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex0), (void*) 0);
    glEnableVertexAttribArray(vPos_location);
#endif

    // misc GL state
    // ------------------------------------------------------------------
}

static void UploadVBO(unsigned count)
{
    unsigned i;
    unsigned total = 0;
    unsigned src = 0;

    for (i = 0; i < count; i++) {
        glBufferData(GL_ARRAY_BUFFER, VBOSize, VBOData + src, GL_STREAM_DRAW);

        glDrawArrays(GL_POINTS, 0, 1);

        /* Throw in an occasional flush to work around a driver crash:
         */
        total += VBOSize;
        if (total >= 16*1024*1024) {
            glFlush();
            total = 0;
        }

        src += VBOSize;
        src %= DATA_SIZE;
    }
    glFinish();
}

static void UploadSubVBO(unsigned count)
{
    unsigned i;
    unsigned src = 0;

    for (i = 0; i < count; i++) {
        unsigned offset = (i * SubSize) % VBOSize;
        glBufferSubData(GL_ARRAY_BUFFER, offset, SubSize, VBOData + src);

        glDrawArrays(GL_POINTS, offset / sizeof(Vertex0), 1);

        src += SubSize;
        src %= DATA_SIZE;
    }
    glFinish();
}


/* Do multiple small SubData uploads, then call DrawArrays.  This may be a
 * fairer comparison to back-to-back BufferData calls:
 */
static void BatchUploadSubVBO(unsigned count)
{
    unsigned i = 0, j;
    unsigned period = VBOSize / SubSize;
    unsigned src = 0;

    while (i < count) {
        for (j = 0; j < period && i < count; j++, i++) {
            unsigned offset = j * SubSize;
            glBufferSubData(GL_ARRAY_BUFFER, offset, SubSize, VBOData + src);
        }

        glDrawArrays(GL_POINTS, 0, 1);

        src += SubSize;
        src %= DATA_SIZE;
    }
    glFinish();
}


/**
 * Test the sequence:
 *    create/load VBO
 *    draw
 *    destroy VBO
 */
static void CreateDrawDestroyVBO(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        GLuint vbo;
        /* create/load */
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, VBOSize, VBOData, GL_STREAM_DRAW);

        /* draw */
#if IS_GlLegacy
        glVertexPointer(2, GL_FLOAT, sizeof(Vertex0), (void *) 0);
#else
        glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex0), (void*) 0);
#endif
        glDrawArrays(GL_POINTS, 0, 1);

        /* destroy */
        glDeleteBuffers(1, &vbo);
    }
    glFinish();
}


static const GLsizei Sizes[] = {
    64,
    1024,
    16*1024,
    256*1024,
    1024*1024,
    16*1024*1024,
    0 /* end of list */
};

static void PerfDraw()
{
    double rate, mbPerSec;
    int i, sz;

    /* Load VBOData buffer with duplicated Vertex0.
     */
    VBOData = (GLubyte*) calloc(DATA_SIZE, 1);

    for (i = 0; i < DATA_SIZE / sizeof(Vertex0); i++) {
        memcpy(VBOData + i * sizeof(Vertex0),
               Vertex0,
               sizeof(Vertex0));
    }

    /* glBufferData()
     */
    for (sz = 0; Sizes[sz]; sz++) {
        SubSize = VBOSize = Sizes[sz];
        rate = PerfMeasureRate(UploadVBO, glfwPollEvents );
        mbPerSec = rate * VBOSize / (1024.0 * 1024.0);
        printf("  glBufferData(size = %d): %.1f MB/sec\n",
                    VBOSize, mbPerSec);
        glfwSwapBuffers(window);
    }
    printf("\n");

    /* glBufferSubData()
     */
    for (sz = 0; Sizes[sz]; sz++) {
        SubSize = VBOSize = Sizes[sz];
        rate = PerfMeasureRate(UploadSubVBO, glfwPollEvents );
        mbPerSec = rate * VBOSize / (1024.0 * 1024.0);
        printf("  glBufferSubData(size = %d): %.1f MB/sec\n",
                    VBOSize, mbPerSec);
        glfwSwapBuffers(window);
    }
    printf("\n");

    /* Batch upload
     */
    VBOSize = 1024 * 1024;
    glBufferData(GL_ARRAY_BUFFER, VBOSize, VBOData, GL_STREAM_DRAW);

    for (sz = 0; Sizes[sz] < VBOSize; sz++) {
        SubSize = Sizes[sz];
        rate = PerfMeasureRate(UploadSubVBO, glfwPollEvents );
        mbPerSec = rate * SubSize / (1024.0 * 1024.0);
        printf("  glBufferSubData(size = %d, VBOSize = %d): %.1f MB/sec\n",
                    SubSize, VBOSize, mbPerSec);
        glfwSwapBuffers(window);
    }
    printf("\n");

    //TODO:FIXME: IMG gpu hang
//    for (sz = 0; Sizes[sz] < VBOSize; sz++) {
//        SubSize = Sizes[sz];
//        rate = PerfMeasureRate(BatchUploadSubVBO, glfwPollEvents );
//        mbPerSec = rate * SubSize / (1024.0 * 1024.0);
//        printf("  glBufferSubData(size = %d, VBOSize = %d), batched: %.1f MB/sec\n",
//                    SubSize, VBOSize, mbPerSec);
//        glfwSwapBuffers(window);
//    }
//    printf("\n");

    /* Create/Draw/Destroy
     */
    for (sz = 0; Sizes[sz]; sz++) {
        SubSize = VBOSize = Sizes[sz];
        rate = PerfMeasureRate(CreateDrawDestroyVBO, glfwPollEvents );
        mbPerSec = rate * VBOSize / (1024.0 * 1024.0);
        printf("  VBO Create/Draw/Destroy(size = %d): %.1f draws/sec, %.1f MB/sec\n",
                    VBOSize, rate, mbPerSec);
        glfwSwapBuffers(window);
    }
    printf("\n");

    glErrorCheck();
    exit(0);
}

static void PerfDraw2( int mode, int Sizes_ )
{
    double rate, mbPerSec;
    int i, sz;

    /* Load VBOData buffer with duplicated Vertex0.
     */
    VBOData = (GLubyte*) calloc(DATA_SIZE, 1);

    for (i = 0; i < DATA_SIZE / sizeof(Vertex0); i++) {
        memcpy(VBOData + i * sizeof(Vertex0),
               Vertex0,
               sizeof(Vertex0));
    }

    /* glBufferData()
     */
    if( mode == 0 ){
        SubSize = VBOSize = Sizes_;
        rate = PerfMeasureRate(UploadVBO, glfwPollEvents );
        mbPerSec = rate * VBOSize / (1024.0 * 1024.0);
        printf("  glBufferData(size = %d): %.1f MB/sec\n",
               VBOSize, mbPerSec);
        glfwSwapBuffers(window);
        printf("\n");
    }

    /* glBufferSubData()
     */
    if( mode == 1 ){
        SubSize = VBOSize = Sizes_;
        glBufferData(GL_ARRAY_BUFFER, VBOSize, VBOData, GL_STREAM_DRAW);
        rate = PerfMeasureRate(UploadSubVBO, glfwPollEvents );
        mbPerSec = rate * VBOSize / (1024.0 * 1024.0);
        printf("  glBufferSubData(size = %d): %.1f MB/sec\n",
               VBOSize, mbPerSec);
        glfwSwapBuffers(window);
        printf("\n");
    }

    /* Batch upload
     */
    if( mode == 2 ){
        VBOSize = 1024 * 1024;
        glBufferData(GL_ARRAY_BUFFER, VBOSize, VBOData, GL_STREAM_DRAW);

        SubSize = Sizes_;
        rate = PerfMeasureRate(UploadSubVBO, glfwPollEvents );
        mbPerSec = rate * SubSize / (1024.0 * 1024.0);
        printf("  glBufferSubData(size = %d, VBOSize = %d): %.1f MB/sec\n",
               SubSize, VBOSize, mbPerSec);
        glfwSwapBuffers(window);
        printf("\n");
    }


    /* Create/Draw/Destroy
     */
    if( mode == 3 ){
        SubSize = VBOSize = Sizes_;
        rate = PerfMeasureRate(CreateDrawDestroyVBO, glfwPollEvents );
        mbPerSec = rate * VBOSize / (1024.0 * 1024.0);
        printf("  VBO Create/Draw/Destroy(size = %d): %.1f draws/sec, %.1f MB/sec\n",
               VBOSize, rate, mbPerSec);
        glfwSwapBuffers(window);
        printf("\n");
    }

    glErrorCheck();
    exit(0);
}

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    int __mode = integerFromArgs("--mode", argc, argv, NULL );
    int __testcase = integerFromArgs( "--testcase", argc, argv, NULL );

    // glfw: initialize and configure
    // ------------------------------
    window = glfw_CreateWindow(api, WinWidth, WinHeight);

    // init
    // -----------
    PerfInit();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        if(  __mode != -1 && __testcase != -1 ){
            PerfDraw2( __mode, __testcase );
            exit(0);
        }

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
