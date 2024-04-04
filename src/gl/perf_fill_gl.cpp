/**
 * Measure fill rates.
 */
#include <stdio.h>
#include <stddef.h>
#include "linmath.h"
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"


// settings
static const int WinWidth = 1000;
static const int WinHeight = 1000;


static GLuint VAO;
static GLuint VBO;
static GLuint TexObj;
static GLuint ShaderProg_simple;
static GLuint ShaderProg_textured;
static GLuint ShaderProg1;
static GLuint ShaderProg2;

struct vertex
{
    GLfloat x, y, s, t, r, g, b, a;
};

#define VOFFSET(F) ((void *) offsetof(struct vertex, F))

static const struct vertex vertices[4] = {
    /*  x     y     s    t     r    g    b    a  */
    { -1.0, -1.0,  0.0, 0.0,  1.0, 0.0, 0.0, 0.5 },
    {  1.0, -1.0,  1.0, 0.0,  0.0, 1.0, 0.0, 0.5 },
    {  1.0,  1.0,  1.0, 1.0,  0.0, 0.0, 1.0, 0.5 },
    { -1.0,  1.0,  0.0, 1.0,  1.0, 1.0, 1.0, 0.5 }
};


// simple fill, blended fill
// ------------------------------------------------------------------
const char *vertexShaderSource_simple =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 330\n"
#endif
    "uniform mat4 MVP;\n"
    "layout (location = 0) in vec2 vPos;\n"
    "layout (location = 1) in vec4 vCol;\n"
    "out vec4 v_color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = MVP * vec4(vPos.x, vPos.y, 0.0, 1.0);\n"
    "   v_color = vCol;\n"
    "}\n\0";

const char *fragmentShaderSource_simple =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 330\n"
#endif
    "in vec4 v_color;\n"
    "layout (location = 0) out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "   outColor = v_color;\n"
    "}\n\0";


// textured fill
// ------------------------------------------------------------------
const char *vertexShaderSource_textured =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 330\n"
    #endif
    "uniform mat4 MVP;\n"
    "layout (location = 0) in vec2 vPos;\n"
    "layout (location = 1) in vec4 vCol;\n"
    "layout (location = 2) in vec2 vTexCoord;\n"
    "out vec2 v_texCoord;\n"
    "out vec4 v_color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = MVP * vec4(vPos.x, vPos.y, 0.0, 1.0);\n"
    "   v_texCoord = vTexCoord;\n"
    "   v_color = vCol;\n"
    "}\n\0";

const char *fragmentShaderSource_textured =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 330\n"
    #endif
    "uniform sampler2D Tex;\n"
    "in vec2 v_texCoord;\n"
    "in vec4 v_color;\n"
    "layout (location = 0) out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "   vec4 t = texture( Tex, v_texCoord );\n"
    "   outColor = t * v_color;\n"  //by default, GL_TEXTURE_ENV_MODE defaults to GL_MODULATE
    "}\n\0";


// shader1 fill, shader2 fill
// ------------------------------------------------------------------
const char *vertexShaderSource =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 330\n"
#endif
    "uniform mat4 MVP;\n"
    "layout (location = 0) in vec2 vPos;\n"
    "layout (location = 1) in vec4 vCol;\n"
    "layout (location = 2) in vec2 vTexCoord;\n"
    "out vec2 v_texCoord;\n"
    "out vec4 v_color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = MVP * vec4(vPos.x, vPos.y, 0.0, 1.0);\n"
    "   v_texCoord = vTexCoord;\n"
    "   v_color = vCol;\n"
    "}\n\0";

/* simple fragment shader */
const char *fragmentShaderSource1 =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 330\n"
#endif
    "uniform sampler2D Tex;\n"
    "in vec2 v_texCoord;\n"
    "in vec4 v_color;\n"
    "layout (location = 0) out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "   vec4 t = texture( Tex, v_texCoord );\n"
    "   outColor = vec4(1.0) - t * v_color;\n"
    "}\n\0";

/**
 * A more complex fragment shader (but equivalent to first shader).
 * A good optimizer should catch some of these no-op operations, but
 * probably not all of them.
 */
const char *fragmentShaderSource2 =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 330\n"
#endif
    "uniform sampler2D Tex;\n"
    "in vec2 v_texCoord;\n"
    "in vec4 v_color;\n"
    "layout (location = 0) out vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "   // as above \n"
    "   vec4 t = texture( Tex, v_texCoord );\n"
    "   t = vec4(1.0) - t * v_color;\n"

    "   vec4 u; \n"

    "   // no-op negate/swizzle \n"
    "   u = -t.wzyx; \n"
    "   t = -u.wzyx; \n"

    "   // no-op inverts \n"
    "   t = vec4(1.0) - t; \n"
    "   t = vec4(1.0) - t; \n"

    "   // no-op min/max \n"
    "   t = min(t, t); \n"
    "   t = max(t, t); \n"

    "   // no-op moves \n"
    "   u = t; \n"
    "   t = u; \n"
    "   u = t; \n"
    "   t = u; \n"

    "   // no-op add/mul \n"
    "   t = (t + t + t + t) * 0.25; \n"

    "   // no-op mul/sub \n"
    "   t = 3.0 * t - 2.0 * t; \n"

    "   // no-op negate/min/max \n"
    "   t = -min(-t, -t); \n"
    "   t = -max(-t, -t); \n"

    "   outColor = t; \n"
    "}\n\0";


static void PerfInit()
{
    // build and compile our shader program
    // ------------------------------------
    ShaderProg_simple = CreateProgramFromSource(vertexShaderSource_simple, fragmentShaderSource_simple);

    ShaderProg_textured = CreateProgramFromSource(vertexShaderSource_textured, fragmentShaderSource_textured);
    glUseProgram(ShaderProg_textured);
    glUniform1i(glGetUniformLocation(ShaderProg_textured, "Tex"), 0);  /* texture unit 0 */

    ShaderProg1 = CreateProgramFromSource(vertexShaderSource, fragmentShaderSource1);
    glUseProgram(ShaderProg1);
    glUniform1i(glGetUniformLocation(ShaderProg1, "Tex"), 0);  /* texture unit 0 */

    ShaderProg2 = CreateProgramFromSource(vertexShaderSource, fragmentShaderSource2);
    glUseProgram(ShaderProg2);
    glUniform1i(glGetUniformLocation(ShaderProg2, "Tex"), 0);  /* texture unit 0 */

    glUseProgram(0);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), VOFFSET(x)); //vPos
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof(struct vertex), VOFFSET(r)); //vCol
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), VOFFSET(s)); //vTexCoord

    glEnableVertexAttribArray(0); //vPos
    glEnableVertexAttribArray(1); //vCol

    // setup texture
    // ------------------------------------------------------------------
    TexObj = CreateTexture_FillWithCheckboard(128, 128);
}

static void Ortho()
{
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();

    mat4x4 m, p, mvp;
    mat4x4_ortho(p, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    mat4x4_identity(m);
    mat4x4_mul( mvp, p, m);

    glUseProgram( ShaderProg_simple );
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg_simple, "MVP"), 1, GL_FALSE, (const GLfloat*) &mvp);

    glUseProgram( ShaderProg_textured );
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg_textured, "MVP"), 1, GL_FALSE, (const GLfloat*) &mvp);

    glUseProgram( ShaderProg1 );
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg1, "MVP"), 1, GL_FALSE, (const GLfloat*) &mvp);

    glUseProgram( ShaderProg2 );
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg2, "MVP"), 1, GL_FALSE, (const GLfloat*) &mvp);

    glUseProgram( 0 );
}

static void DrawQuad(unsigned count)
{
    unsigned i;
    glClear(GL_COLOR_BUFFER_BIT);

    for (i = 0; i < count; i++) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        /* Avoid sending command buffers with huge numbers of fullscreen
         * quads.  Graphics schedulers don't always cope well with
         * this...
         */
        if (i % 128 == 0) {
            eglx_SwapBuffers();
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    glFinish();

    if (1)
        eglx_SwapBuffers();

}

static void PerfDraw()
{
    double rate;
    double pixelsPerDraw = WinWidth * WinHeight;

    Ortho();

    /* simple fill */
    glUseProgram( ShaderProg_simple );
    rate = PerfMeasureRate(DrawQuad, eglx_PollEvents ) * pixelsPerDraw;
    printf("   Simple fill: %s pixels/second\n",
                PerfHumanFloat(rate));

    /* blended fill */
    glEnable(GL_BLEND);
    glUseProgram( ShaderProg_simple );
    rate = PerfMeasureRate(DrawQuad, eglx_PollEvents ) * pixelsPerDraw;
    glDisable(GL_BLEND);
    printf("   Blended fill: %s pixels/second\n",
                PerfHumanFloat(rate));

    /* textured fill */
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, TexObj );
    glEnableVertexAttribArray(2); //vTexCoord
    glUseProgram( ShaderProg_textured );
    rate = PerfMeasureRate(DrawQuad, eglx_PollEvents ) * pixelsPerDraw;
    printf("   Textured fill: %s pixels/second\n",
                PerfHumanFloat(rate));

    /* shader1 fill */
    glUseProgram(ShaderProg1);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, TexObj );
    glEnableVertexAttribArray(2); //vTexCoord
    rate = PerfMeasureRate(DrawQuad, eglx_PollEvents ) * pixelsPerDraw;
    glUseProgram(0);
    printf("   Shader1 fill: %s pixels/second\n",
                PerfHumanFloat(rate));

    /* shader2 fill */
    glUseProgram(ShaderProg2);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, TexObj );
    glEnableVertexAttribArray(2); //vTexCoord
    rate = PerfMeasureRate(DrawQuad, eglx_PollEvents ) * pixelsPerDraw;
    glUseProgram(0);
    printf("   Shader2 fill: %s pixels/second\n",
                PerfHumanFloat(rate));

    glErrorCheck();
    exit(0);
}

int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

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
