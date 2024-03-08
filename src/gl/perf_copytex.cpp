/**
 * Measure glCopyTex[Sub]Image() rate.
 * Create a large, off-screen framebuffer object for rendering and
 * copying the texture data from it since we can't make really large
 * on-screen windows.
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
#include "glutils.h"
#include "glfwutils.h"
#include "myutils.h"


// settings
static int WinWidth = 200;
static int WinHeight = 200;

static GLuint VAO, VBO, FBO, RBO, Tex, FBTex;
static GLuint program;
static GLint samplerLoc;

static const GLsizei MinSize = 16;
static const GLsizei MaxSize = 4096;
static GLsizei TexSize;

static GLboolean DrawPoint = GL_TRUE;
static const GLboolean TexSubImage4 = GL_FALSE;

struct vertex
{
    GLfloat x, y, s, t;
};

static const struct vertex vertices[1] = {
        { 0.0, 0.0, 0.5, 0.5 },
};

#define VOFFSET(F) ((void *) offsetof(struct vertex, F))


const char *vertexShaderSource =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 400\n"
#endif
    "layout (location = 0) in vec2 vPos;\n"
    "layout (location = 1) in vec2 vTexCoord;\n"
    "out vec2 v_texCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4( vPos.x, vPos.y, 0.0f, 1.0f );\n"
    "   v_texCoord = vTexCoord;\n"
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
    "in vec2 v_texCoord;\n"
    "layout (location = 0) out vec4 outColor;\n"
    "uniform sampler2D s_texture;\n"
    "void main()\n"
    "{\n"
    "   outColor = texture( s_texture, v_texCoord );\n"
    "}\n\0";

static void PerfInit()
{
    const GLenum filter = GL_LINEAR;
    GLenum stat;

    // build and compile our shader program
    // ------------------------------------
#if !IS_GlLegacy
    program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );
    samplerLoc = glGetUniformLocation( program, "s_texture" );
#endif

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
#if IS_GlLegacy
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexPointer(2, GL_FLOAT, sizeof(struct vertex), VOFFSET(x));
    glTexCoordPointer(2, GL_FLOAT, sizeof(struct vertex), VOFFSET(s));
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#else
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLint vPos_location = glGetAttribLocation(program, "vPos");
    const GLint vTexCoord_location = glGetAttribLocation(program, "vTexCoord");
    printf("Attrib location: vPos=%d\n", vPos_location);
    printf("Attrib location: vTexCoord=%d\n", vTexCoord_location);
    glVertexAttribPointer(vPos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct vertex), (void*)VOFFSET(x));
    glVertexAttribPointer(vTexCoord_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct vertex), (void *)VOFFSET(s) );
    glEnableVertexAttribArray(vPos_location);
    glEnableVertexAttribArray(vTexCoord_location);
#endif

    // setup fbo
    // ------------------------------------
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    /* setup rbo */
#if IS_GlLegacy
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, MaxSize, MaxSize);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, RBO);
#else
    glGenTextures( 1, &FBTex );
    glBindTexture( GL_TEXTURE_2D, FBTex );
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 MaxSize, MaxSize, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    glFramebufferTexture2D ( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_2D, FBTex, 0 );
#endif


    stat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (stat != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: incomplete FBO!: 0x%X, %s\n", stat, framebufferStatusName(stat));
        exit(1);
    }

    /* clear the FBO */
    static const GLenum attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers( 1, attachments);
    glViewport(0, 0, MaxSize, MaxSize);
    glClear(GL_COLOR_BUFFER_BIT);

    // set up texture data and configure texture attributes
    // ------------------------------------------------------------------
    glGenTextures(1, &Tex);
    glBindTexture(GL_TEXTURE_2D, Tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

#if IS_GlLegacy
    glEnable(GL_TEXTURE_2D);
#else
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, Tex );

    glUseProgram(program);
    // set the sampler texture unit to 0
    glUniform1i( samplerLoc, 0 );
#endif

}


static void CopyTexImage(unsigned count)
{
    unsigned i;
    for (i = 1; i < count; i++) {
        /* draw something */
        if (DrawPoint)
            glDrawArrays(GL_POINTS, 0, 1);

        /* copy whole texture */
        glCopyTexImage2D(GL_TEXTURE_2D, 0,
                         GL_RGBA, 0, 0, TexSize, TexSize, 0);
    }
    glFinish();
}


static void
CopyTexSubImage(unsigned count)
{
    unsigned i;
    for (i = 1; i < count; i++) {
        /* draw something */
        if (DrawPoint)
            glDrawArrays(GL_POINTS, 0, 1);

        /* copy sub texture */
        if (TexSubImage4) {
            /* four sub-copies */
            GLsizei half = TexSize / 2;
            /* lower-left */
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
                                0, 0, 0, 0, half, half);
            /* lower-right */
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
                                half, 0, half, 0, half, half);
            /* upper-left */
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
                                0, half, 0, half, half, half);
            /* upper-right */
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
                                half, half, half, half, half, half);
        }
        else {
            /* one big copy */
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
                                0, 0, 0, 0, TexSize, TexSize);
        }
    }
    glFinish();
}

void PerfDraw()
{
    double rate, mbPerSec;
    GLint sub, maxTexSize;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    printf("GL_MAX_TEXTURE_SIZE = %d\n", maxTexSize);

    /* loop over whole/sub tex copy */
    for (sub = 0; sub < 2; sub++) {

        /* loop over texture sizes */
        for (TexSize = MinSize; TexSize <= MaxSize; TexSize *= 4) {

            if (TexSize <= maxTexSize) {
                GLint bytesPerImage = 4 * TexSize * TexSize;

                if (sub == 0)
                    rate = PerfMeasureRate(CopyTexImage);
                else {
                    /* setup empty dest texture */
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                                 TexSize, TexSize, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                    rate = PerfMeasureRate(CopyTexSubImage);
                }

                mbPerSec = rate * bytesPerImage / (1024.0 * 1024.0);
            }
            else {
                rate = 0.0;
                mbPerSec = 0.0;
            }

            printf("  glCopyTex%sImage(%d x %d)%s: %.1f copies/sec, %.1f Mpixels/sec\n",
                   (sub ? "Sub" : ""), TexSize, TexSize,
                   (DrawPoint) ? " + Draw" : "",
                   rate, mbPerSec);
        }
    }

    glErrorCheck();
}

void PerfDraw2( int sub, int TexSize_ )
{
    double rate, mbPerSec;
    GLint maxTexSize;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    printf("GL_MAX_TEXTURE_SIZE = %d\n", maxTexSize);

    {
        TexSize = TexSize_;

        {

            if (TexSize <= maxTexSize) {
                GLint bytesPerImage = 4 * TexSize * TexSize;

                if (sub == 0)
                    rate = PerfMeasureRate(CopyTexImage);
                else {
                    /* setup empty dest texture */
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                                 TexSize, TexSize, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                    rate = PerfMeasureRate(CopyTexSubImage);
                }

                mbPerSec = rate * bytesPerImage / (1024.0 * 1024.0);
            }
            else {
                rate = 0.0;
                mbPerSec = 0.0;
            }

            printf("  glCopyTex%sImage(%d x %d)%s: %.1f copies/sec, %.1f Mpixels/sec\n",
                   (sub ? "Sub" : ""), TexSize, TexSize,
                   (DrawPoint) ? " + Draw" : "",
                   rate, mbPerSec);
        }
    }

    glErrorCheck();
}


int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    int __testcase = integerFromArgs( "--testcase", argc, argv, NULL );
    int __mode = integerFromArgs( "--mode", argc, argv, NULL );
    int __draw = integerFromArgs("--draw", argc, argv, NULL );

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
        if( __mode != -1 && __testcase != -1 && __draw != -1 ){
            DrawPoint = __draw;

            printf("Draw = %d\n", DrawPoint);
            PerfDraw2( __mode, __testcase );//TODO
            printf("\n");
            exit(0);
        }

        // render
        // ------
        DrawPoint = GL_FALSE;
        printf("Draw = %d\n", DrawPoint);
        PerfDraw();
        printf("\n");

        DrawPoint = GL_TRUE;
        printf("Draw = %d\n", DrawPoint);
        PerfDraw();
        printf("\n");
        exit(0);

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
