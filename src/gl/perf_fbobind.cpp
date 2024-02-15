/**
 * Measure rate of binding/switching between FBO targets.
 * Create two framebuffer objects for rendering to two textures.
 * Ping pong between texturing from one and drawing into the other.
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
int WinWidth = 200, WinHeight = 200;

static GLuint VAO, VBO;
static GLuint FBO[2], Tex[2];
static const GLsizei TexSize = 512;
static const GLboolean DrawPoint = GL_TRUE;
static GLuint program;
static GLint samplerLoc;

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),vertices, GL_STATIC_DRAW);

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
    glGenFramebuffers(2, FBO);
    glGenTextures(2, Tex);
    for( int i = 0; i < 2; i++ ){
        /* setup texture */
        const GLenum filter = GL_LINEAR;
        glBindTexture(GL_TEXTURE_2D, Tex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     TexSize, TexSize, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

        /* setup fbo */
        glBindFramebuffer(GL_FRAMEBUFFER, FBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, Tex[i], 0);
        GLenum stat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if( stat != GL_FRAMEBUFFER_COMPLETE ){
            printf("Error: incomplete FBO!: 0x%X, %s\n", stat, framebufferStatusName(stat));
            exit(1);
        }

        /* clear the FBO */
        glClear(GL_COLOR_BUFFER_BIT);
    }

#if IS_GlLegacy
    glEnable(GL_TEXTURE_2D);
#else
    // do nothing
#endif
}

static void FBOBind(unsigned count)
{
    unsigned i;
    for (i = 1; i < count; i++) {
        const GLuint dst = i & 1;
        const GLuint src = 1 - dst;

        /* bind src texture */
#if IS_GlLegacy
        glBindTexture(GL_TEXTURE_2D, Tex[src]);
#else
        glActiveTexture( GL_TEXTURE0 + src );
        glBindTexture( GL_TEXTURE_2D, Tex[src] );

        glUseProgram(program);
        glUniform1i( samplerLoc, src );
#endif

        /* bind dst fbo */
        glBindFramebuffer(GL_FRAMEBUFFER, FBO[dst]);
        GLenum attachments  = GL_COLOR_ATTACHMENT0;
        glDrawBuffers( 1, &attachments);

        /* draw something */
        if (DrawPoint)
            glDrawArrays(GL_POINTS, 0, 1);
    }
    glFinish();
}

void PerfDraw()
{
    double rate = PerfMeasureRate(FBOBind);
    printf("  FBO Binding: %1.f binds/sec\n", rate);

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
