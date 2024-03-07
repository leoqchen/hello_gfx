/**
 * Measure glTex[Sub]Image2D() and glGetTexImage() rate
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
static const int WinWidth = 100;
static const int WinHeight = 100;
static GLFWwindow* window;

static GLuint VAO;
static GLuint VBO;
static GLuint program;
static GLuint TexObj = 0;
static GLubyte *TexImage = NULL;
static GLsizei TexSize;
static GLenum TexIntFormat, TexSrcFormat, TexSrcType;

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
    glVertexAttribPointer(vPos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct vertex), VOFFSET(x));
    glVertexAttribPointer(vTexCoord_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct vertex), VOFFSET(s));
    glEnableVertexAttribArray(vPos_location);
    glEnableVertexAttribArray(vTexCoord_location);
#endif

    // texture
    // ------------------------------------------------------------------
    glGenTextures(1, &TexObj);
    glBindTexture(GL_TEXTURE_2D, TexObj);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#if IS_GlLegacy
    glEnable(GL_TEXTURE_2D);
#else
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, TexObj );
#endif
}

static void CreateUploadTexImage2D(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        if (TexObj)
            glDeleteTextures(1, &TexObj);

        glGenTextures(1, &TexObj);
        glBindTexture(GL_TEXTURE_2D, TexObj);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, TexIntFormat,
                     TexSize, TexSize, 0,
                     TexSrcFormat, TexSrcType, TexImage);

        if (DrawPoint)
            glDrawArrays(GL_POINTS, 0, 1);
    }
    glFinish();
}

static void UploadTexImage2D(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        /* XXX is this equivalent to a glTexSubImage call since we're
         * always specifying the same image size?  That case isn't optimized
         * in Mesa but may be optimized in other drivers.  Note sure how
         * much difference that might make.
         */
        glTexImage2D(GL_TEXTURE_2D, 0, TexIntFormat,
                     TexSize, TexSize, 0,
                     TexSrcFormat, TexSrcType, TexImage);
        if (DrawPoint)
            glDrawArrays(GL_POINTS, 0, 1);
    }
    glFinish();
}


static void UploadTexSubImage2D(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        if (TexSubImage4) {
            GLsizei halfSize = (TexSize == 1) ? 1 : TexSize / 2;
            GLsizei halfPos = TexSize - halfSize;
            /* do glTexSubImage2D in four pieces */
            /* lower-left */
            glPixelStorei(GL_UNPACK_ROW_LENGTH, TexSize);
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                            0, 0, halfSize, halfSize,
                            TexSrcFormat, TexSrcType, TexImage);
            /* lower-right */
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, halfPos);
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                            halfPos, 0, halfSize, halfSize,
                            TexSrcFormat, TexSrcType, TexImage);
            /* upper-left */
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, halfPos);
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                            0, halfPos, halfSize, halfSize,
                            TexSrcFormat, TexSrcType, TexImage);
            /* upper-right */
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, halfPos);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, halfPos);
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                            halfPos, halfPos, halfSize, halfSize,
                            TexSrcFormat, TexSrcType, TexImage);
            /* reset the unpacking state */
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }
        else {
            /* replace whole texture image at once */
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                            0, 0, TexSize, TexSize,
                            TexSrcFormat, TexSrcType, TexImage);
        }
        if (DrawPoint)
            glDrawArrays(GL_POINTS, 0, 1);
    }
    glFinish();
}

#if !IS_GlEs
static void GetTexImage2D(unsigned count)
{
    unsigned i;
    GLubyte *buf = (GLubyte *) malloc(TexSize * TexSize * 4);
    for (i = 0; i < count; i++) {
        glGetTexImage(GL_TEXTURE_2D, 0,
                      TexSrcFormat, TexSrcType, buf);
    }
    glFinish();
    free(buf);
}
#endif

enum {
    MODE_CREATE_TEXIMAGE,
    MODE_TEXIMAGE,
    MODE_TEXSUBIMAGE,
    MODE_GETTEXIMAGE,
    MODE_COUNT
};

static const char *mode_name[MODE_COUNT] = {
    "Create_TexImage",
    "TexImage",
    "TexSubImage",
    "GetTexImage"
};

static const struct {
    GLenum format, type;
    GLenum internal_format;
    const char *name;
    GLuint texel_size;
    GLboolean full_test;
} SrcFormats[] = {
    { GL_RGBA, GL_UNSIGNED_BYTE,       GL_RGBA, "RGBA/ubyte", 4,   GL_TRUE },
//    { GL_RGB, GL_UNSIGNED_BYTE,        GL_RGB, "RGB/ubyte", 3,     GL_FALSE },
//    { GL_RGB, GL_UNSIGNED_SHORT_5_6_5, GL_RGB, "RGB/565", 2,       GL_FALSE },
//#if !IS_GlEs
//    { GL_BGRA, GL_UNSIGNED_BYTE,       GL_RGBA, "BGRA/ubyte", 4,   GL_FALSE },
//#endif
//#if !IS_Gl
//    { GL_LUMINANCE, GL_UNSIGNED_BYTE,  GL_LUMINANCE, "L/ubyte", 1, GL_FALSE },
//#endif
    { 0, 0, 0, NULL, 0, 0 }
};



static void PerfDraw()
{
    GLint maxSize;
    double rate;
    GLint fmt, mode;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    printf("GL_MAX_TEXTURE_SIZE = %d\n", maxSize);

    /* loop over source data formats */
    for (fmt = 0; SrcFormats[fmt].format; fmt++) {
        TexIntFormat = SrcFormats[fmt].internal_format;
        TexSrcFormat = SrcFormats[fmt].format;
        TexSrcType = SrcFormats[fmt].type;

        /* loop over glTexImage, glTexSubImage */
        for (mode = 0; mode < MODE_COUNT; mode++) {
            GLuint minsz, maxsz;

            if (SrcFormats[fmt].full_test) {
                minsz = 16;
                maxsz = 4096;
            }
            else {
                minsz = maxsz = 256;
                if (mode == MODE_CREATE_TEXIMAGE)
                    continue;
            }

            /* loop over a defined range of texture sizes, test only the
             * ones which are legal for this driver.
             */
            for (TexSize = minsz; TexSize <= maxsz; TexSize *= 4) {
                double mbPerSec;

                if (TexSize <= maxSize) {
                    GLint bytesPerImage;

                    bytesPerImage = TexSize * TexSize * SrcFormats[fmt].texel_size;
                    TexImage = (GLubyte*) malloc(bytesPerImage);
                    memset( TexImage, 0xff, bytesPerImage );

                    switch (mode) {
                        case MODE_TEXIMAGE:
                            rate = PerfMeasureRate(UploadTexImage2D);
                            break;

                        case MODE_CREATE_TEXIMAGE:
                            rate = PerfMeasureRate(CreateUploadTexImage2D);
                            break;

                        case MODE_TEXSUBIMAGE:
                            /* create initial, empty texture */
                            glTexImage2D(GL_TEXTURE_2D, 0, TexIntFormat,
                                         TexSize, TexSize, 0,
                                         TexSrcFormat, TexSrcType, NULL);
                            rate = PerfMeasureRate(UploadTexSubImage2D);
                            break;

                        case MODE_GETTEXIMAGE:
#if IS_GlEs
                            continue;
#else
                            glTexImage2D(GL_TEXTURE_2D, 0, TexIntFormat,
                                         TexSize, TexSize, 0,
                                         TexSrcFormat, TexSrcType, TexImage);
                            rate = PerfMeasureRate(GetTexImage2D);
                            break;
#endif

                        default:
                            exit(1);
                    }

                    mbPerSec = rate * bytesPerImage / (1024.0 * 1024.0);
                    free(TexImage);

                    glErrorCheck();
                }
                else {
                    rate = 0;
                    mbPerSec = 0;
                }

                printf("  %s(%s %d x %d)%s: "
                       "%.1f images/sec, %.1f MB/sec\n",
                       mode_name[mode], SrcFormats[fmt].name, TexSize, TexSize,
                       (DrawPoint) ? " + Draw" : "",
                       rate, mbPerSec);
                glfwSwapBuffers(window);
            }
        }
        printf("\n");
    }

    glErrorCheck();
}

static void PerfDraw2( GLint mode_, GLuint TexSize_ )
{
    GLint maxSize;
    double rate;
    GLint fmt, mode;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    printf("GL_MAX_TEXTURE_SIZE = %d\n", maxSize);

    {
        GLint fmt = 0;
        TexIntFormat = SrcFormats[fmt].internal_format;
        TexSrcFormat = SrcFormats[fmt].format;
        TexSrcType = SrcFormats[fmt].type;

        /* loop over glTexImage, glTexSubImage */
        mode = mode_;
        {
            GLuint minsz, maxsz;

            if (SrcFormats[fmt].full_test) {
                minsz = 16;
                maxsz = 4096;
            }
            else {
                minsz = maxsz = 256;
                if (mode == MODE_CREATE_TEXIMAGE)
                    return;
            }

            TexSize = TexSize_;
            {
                double mbPerSec;

                if (TexSize <= maxSize) {
                    GLint bytesPerImage;

                    bytesPerImage = TexSize * TexSize * SrcFormats[fmt].texel_size;
                    TexImage = (GLubyte*) malloc(bytesPerImage);
                    memset( TexImage, 0xff, bytesPerImage );

                    switch (mode) {
                        case MODE_TEXIMAGE:
                            rate = PerfMeasureRate(UploadTexImage2D);
                            break;

                        case MODE_CREATE_TEXIMAGE:
                            rate = PerfMeasureRate(CreateUploadTexImage2D);
                            break;

                        case MODE_TEXSUBIMAGE:
                            /* create initial, empty texture */
                            glTexImage2D(GL_TEXTURE_2D, 0, TexIntFormat,
                                         TexSize, TexSize, 0,
                                         TexSrcFormat, TexSrcType, NULL);
                            rate = PerfMeasureRate(UploadTexSubImage2D);
                            break;

                        case MODE_GETTEXIMAGE:
#if IS_GlEs
                            return;
#else
                            glTexImage2D(GL_TEXTURE_2D, 0, TexIntFormat,
                                         TexSize, TexSize, 0,
                                         TexSrcFormat, TexSrcType, TexImage);
                            rate = PerfMeasureRate(GetTexImage2D);
                            break;
#endif

                        default:
                            exit(1);
                    }

                    mbPerSec = rate * bytesPerImage / (1024.0 * 1024.0);
                    free(TexImage);

                    glErrorCheck();
                }
                else {
                    rate = 0;
                    mbPerSec = 0;
                }

                printf("  %s(%s %d x %d)%s: "
                       "%.1f images/sec, %.1f MB/sec\n",
                       mode_name[mode], SrcFormats[fmt].name, TexSize, TexSize,
                       (DrawPoint) ? " + Draw" : "",
                       rate, mbPerSec);
                glfwSwapBuffers(window);
            }
        }
        printf("\n");
    }

    glErrorCheck();
}

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    int __mode = integerFromArgs("--mode", argc, argv, NULL );
    int __testcase = integerFromArgs( "--testcase", argc, argv, NULL );
    int __draw = integerFromArgs("--draw", argc, argv, NULL );

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
        if( __mode != -1 && __testcase != -1 && __draw != -1 ){
            DrawPoint = __draw;

            printf("Draw = %d\n", DrawPoint);
            PerfDraw2( __mode, __testcase );
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
