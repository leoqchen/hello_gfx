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
const int WinWidth = 200;
const int WinHeight = 200;

static GLboolean DrawPoint = GL_TRUE;
static GLuint vertex_array;
static GLuint vertex_buffer;
static GLuint textureId = 0;
static GLint BaseLevel, MaxLevel;
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
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexPointer(2, GL_FLOAT, sizeof(struct vertex), VOFFSET(x));
    glTexCoordPointer(2, GL_FLOAT, sizeof(struct vertex), VOFFSET(s));
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#else
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
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

    // set up texture data and configure texture attributes
    // ------------------------------------------------------------------
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

#if IS_GlLegacy
    glEnable(GL_TEXTURE_2D);
#else
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureId );

    glUseProgram(program);
    // set the sampler texture unit to 0
    glUniform1i( samplerLoc, 0 );
#endif
}

static void GenMipmap(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        GLubyte texel[4];
        texel[0] = texel[1] = texel[2] = texel[3] = i & 0xff;
        /* dirty the base image */
        glTexSubImage2D(GL_TEXTURE_2D, BaseLevel,
                        0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, texel);
        glGenerateMipmap(GL_TEXTURE_2D);
        if (DrawPoint)
            glDrawArrays(GL_POINTS, 0, 1);
    }
    glFinish();
}

static void PerfDraw()
{
    const GLint NumLevels = 12;
    const GLint TexWidth = 2048, TexHeight = 2048;
    GLubyte *img;
    double rate;

    /* Make 2K x 2K texture */
    img = (GLubyte *) malloc(TexWidth * TexHeight * 4);
    memset(img, 128, TexWidth * TexHeight * 4);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_RGBA, TexWidth, TexHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, img);
    free(img);

    printf("Texture level[0] size: %d x %d, %d levels\n",
                TexWidth, TexHeight, NumLevels);

    /* loop over base levels 0, 2, 4 */
    for (BaseLevel = 0; BaseLevel <= 4; BaseLevel += 2) {

        /* loop over max level */
        for (MaxLevel = NumLevels; MaxLevel > BaseLevel; MaxLevel--) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, BaseLevel);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MaxLevel);

            rate = PerfMeasureRate(GenMipmap);

            printf("   glGenerateMipmap(levels %d..%d): %.2f gens/sec\n",
                        BaseLevel + 1, MaxLevel, rate);
        }
    }

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
