/**
 * Measure glReadPixels speed.
 */
#include <stdio.h>
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"


// settings
static const int WinWidth = 1000;
static const int WinHeight = 1000;


static GLuint VAO;
static GLuint VBO;
static GLuint program;
static const GLboolean DrawPoint = GL_TRUE;
static GLenum ReadFormat, ReadType;
static GLint ReadWidth, ReadHeight;
static GLvoid *ReadBuffer;

static GLuint PBO;
static int use_PBO = 0;

static const GLfloat vertices[2] = { 0.0, 0.0 };

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

    glVertexPointer(2, GL_FLOAT, sizeof(vertices), (void *) 0);
    glEnableClientState(GL_VERTEX_ARRAY);
#else
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLint vPos_location = glGetAttribLocation(program, "vPos");
    printf("Attrib location: vPos=%d\n", vPos_location);
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices), (void*) 0);
    glEnableVertexAttribArray(vPos_location);
#endif

    // misc GL state
    // ------------------------------------------------------------------
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // PBO
    // ------------------------------------------------------------------
    glGenBuffers( 1, &PBO );
    glBindBuffer( GL_PIXEL_PACK_BUFFER, PBO );
    glBufferData( GL_PIXEL_PACK_BUFFER, 1000 * 1000 *4, NULL, GL_STREAM_READ );
    glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
}

static void ReadPixels(unsigned count)
{
    if( use_PBO )
        glBindBuffer( GL_PIXEL_PACK_BUFFER, PBO );
    else
        glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

    unsigned i;
    for (i = 0; i < count; i++) {
        /* read from random pos */
        GLint x, y;

        x = WinWidth - ReadWidth;
        y = WinHeight - ReadHeight;
        if (x > 0)
            x = rand() % x;
        if (y > 0)
            y = rand() % y;

        if (DrawPoint)
            glDrawArrays(GL_POINTS, 0, 1);

        glReadPixels(x, y, ReadWidth, ReadHeight,
                     ReadFormat, ReadType, use_PBO ? 0 : ReadBuffer);
    }
    glFinish();
}


static const GLsizei Sizes[] = {
    10,
    100,
    500,
    1000,
    0
};

static const struct {
    GLenum format;
    GLenum type;
    const char *name;
    GLuint pixel_size;
} DstFormats[] = {
    { GL_RGBA, GL_UNSIGNED_BYTE,           "RGBA/ubyte", 4 },
//#if !IS_GlEs
//    // OpenGL ES not support
//    { GL_BGRA, GL_UNSIGNED_BYTE,           "BGRA/ubyte", 4 },
//    { GL_RGB, GL_UNSIGNED_SHORT_5_6_5,     "RGB/565", 2 },
//    { GL_LUMINANCE, GL_UNSIGNED_BYTE,      "L/ubyte", 1 },
//    { GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, "Z/uint", 4 },
//    { GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, "Z+S/uint", 4 },
//#endif
    { 0, 0, NULL, 0 }
};


static void PerfDraw()
{
    /* loop over formats */
    for (int fmt = 0; DstFormats[fmt].format; fmt++)
    {
        ReadFormat = DstFormats[fmt].format;
        ReadType = DstFormats[fmt].type;

        /* loop over sizes */
        for (int sz = 0; Sizes[sz]; sz++)
        {
            int imgSize;

            ReadWidth = ReadHeight = Sizes[sz];
            imgSize = ReadWidth * ReadHeight * DstFormats[fmt].pixel_size;
            ReadBuffer = malloc(imgSize);

            double rate = PerfMeasureRate(ReadPixels, eglx_PollEvents );
            double mbPerSec = rate * imgSize / (1024.0 * 1024.0);

            printf("glReadPixels(%d x %d, %s): %.1f images/sec, %.1f Mpixels/sec\n",
                   ReadWidth, ReadHeight,
                   DstFormats[fmt].name, rate, mbPerSec);

            free(ReadBuffer);
            eglx_SwapBuffers();
        }
    }

    glErrorCheck();
    exit(0);
}

static void PerfDraw2( int Sizes_)
{
    {
        int fmt = 0;
        ReadFormat = DstFormats[fmt].format;
        ReadType = DstFormats[fmt].type;

        {
            int imgSize;

            ReadWidth = ReadHeight = Sizes_;
            imgSize = ReadWidth * ReadHeight * DstFormats[fmt].pixel_size;
            ReadBuffer = malloc(imgSize);

            double rate = PerfMeasureRate(ReadPixels, eglx_PollEvents );
            double mbPerSec = rate * imgSize / (1024.0 * 1024.0);

            printf("glReadPixels(%d x %d, %s): %.1f images/sec, %.1f Mpixels/sec\n",
                   ReadWidth, ReadHeight,
                   DstFormats[fmt].name, rate, mbPerSec);

            free(ReadBuffer);
            eglx_SwapBuffers();
        }
    }

    glErrorCheck();
    exit(0);
}

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    int __testcase = integerFromArgs( "--testcase", argc, argv, NULL );
    int __pbo = integerFromArgs( "--pbo", argc, argv, NULL );

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
        if( __testcase != -1 ){
            if( __pbo != -1 )
                use_PBO = __pbo;

            PerfDraw2( __testcase );
            exit(0);
        }

        // render
        // ------
        PerfDraw();

        // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        eglx_SwapBuffers();
        eglx_PollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------


    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
