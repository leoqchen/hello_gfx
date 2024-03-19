/**
 * Measure fill rates.
 */
#include <stdio.h>
#include <stddef.h>
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

const char *vertexShaderSource =
    "#version 120\n"
    "void main()\n"
    "{\n"
    "   gl_Position = ftransform(); \n"
    "   gl_TexCoord[0] = gl_MultiTexCoord0; \n"
    "   gl_FrontColor = gl_Color; \n"
    "}\n\0";

/* simple fragment shader */
const char *fragmentShaderSource1 =
    "#version 120\n"
    "uniform sampler2D Tex;\n"
    "void main()\n"
    "{\n"
    "   vec4 t = texture2D(Tex, gl_TexCoord[0].xy); \n"
    "   gl_FragColor = vec4(1.0) - t * gl_Color; \n"
    "}\n\0";

/**
 * A more complex fragment shader (but equivalent to first shader).
 * A good optimizer should catch some of these no-op operations, but
 * probably not all of them.
 */
const char *fragmentShaderSource2 =
    "#version 120\n"
    "uniform sampler2D Tex;\n"
    "void main()\n"
    "{\n"
    "   // as above \n"
    "   vec4 t = texture2D(Tex, gl_TexCoord[0].xy); \n"
    "   t = vec4(1.0) - t * gl_Color; \n"

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

    "   gl_FragColor = t; \n"
    "}\n\0";


static void PerfInit()
{
    // build and compile our shader program
    // ------------------------------------
    ShaderProg1 = CreateProgramFromSource(vertexShaderSource, fragmentShaderSource1);
    glUseProgram(ShaderProg1);
    glUniform1i(glGetUniformLocation(ShaderProg1, "Tex"), 0);  /* texture unit 0 */

    ShaderProg2 = CreateProgramFromSource(vertexShaderSource, fragmentShaderSource2);
    glUseProgram(ShaderProg2);
    glUniform1i(glGetUniformLocation(ShaderProg2, "Tex"), 0);  /* texture unit 0 */

    glUseProgram(0);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexPointer(2, GL_FLOAT, sizeof(struct vertex), VOFFSET(x));
    glTexCoordPointer(2, GL_FLOAT, sizeof(struct vertex), VOFFSET(s));
    glColorPointer(4, GL_FLOAT, sizeof(struct vertex), VOFFSET(r));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // setup texture
    // ------------------------------------------------------------------
    TexObj = CreateTexture_FillWithCheckboard(128, 128);
}

static void Ortho()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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

    /* simple fill (fixed function pipeline) */
    rate = PerfMeasureRate(DrawQuad, eglx_PollEvents ) * pixelsPerDraw;
    printf("   Simple fill: %s pixels/second\n",
                PerfHumanFloat(rate));

    /* blended fill (fixed function pipeline) */
    glEnable(GL_BLEND);
    rate = PerfMeasureRate(DrawQuad, eglx_PollEvents ) * pixelsPerDraw;
    glDisable(GL_BLEND);
    printf("   Blended fill: %s pixels/second\n",
                PerfHumanFloat(rate));

    /* textured fill (fixed function pipeline) */
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    rate = PerfMeasureRate(DrawQuad, eglx_PollEvents ) * pixelsPerDraw;
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    printf("   Textured fill: %s pixels/second\n",
                PerfHumanFloat(rate));

    /* shader1 fill */
    glUseProgram(ShaderProg1);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    rate = PerfMeasureRate(DrawQuad, eglx_PollEvents ) * pixelsPerDraw;
    glUseProgram(0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    printf("   Shader1 fill: %s pixels/second\n",
                PerfHumanFloat(rate));

    /* shader2 fill */
    glUseProgram(ShaderProg2);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    rate = PerfMeasureRate(DrawQuad, eglx_PollEvents ) * pixelsPerDraw;
    glUseProgram(0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
