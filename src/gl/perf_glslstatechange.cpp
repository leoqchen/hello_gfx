/**
 * Test states change when using shaders & textures.
 */
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stddef.h>
#include "linmath.h"
#include "glad.h"
#include "glUtils.h"
#include "eglUtils.h"
#include "myUtils.h"
#include "SGI_rgb.h"

// settings
static const int WinWidth = 500;
static const int WinHeight = 500;


static mat4x4 M;
static mat4x4 P;

static GLuint VAO;
static GLuint VBO;
static GLuint program1;
static GLuint program2;
static GLfloat Xrot = 0.0, Yrot = 0.0, Zrot = 0.0;
static GLfloat EyeDist = 10;

static GLint prog1_VertCoord_aLoc;
static GLint prog1_TexCoord0_aLoc;
static GLint prog1_TexCoord1_aLoc;
static GLint prog1_tex1_uLoc;
static GLint prog1_tex2_uLoc;
static GLint prog1_UniV1_uLoc;
static GLint prog1_UniV2_uLoc;
static GLint prog1_MVP_uLoc;

static GLint prog2_VertCoord_aLoc;
static GLint prog2_TexCoord0_aLoc;
static GLint prog2_TexCoord1_aLoc;
static GLint prog2_tex1_uLoc;
static GLint prog2_tex2_uLoc;
static GLint prog2_UniV1_uLoc;
static GLint prog2_UniV2_uLoc;
static GLint prog2_MVP_uLoc;

static GLuint texObj[4];
static const char* TexFiles[4] = {
    PROJECT_SOURCE_DIR  "data/tile.rgb",
    PROJECT_SOURCE_DIR  "data/tree2.rgba",
    PROJECT_SOURCE_DIR  "data/tile.rgb",
    PROJECT_SOURCE_DIR  "data/tree2.rgba"
};

typedef struct Vertex{
    vec2 VertCoords;
    vec2 Tex0Coords;
    vec2 Tex1Coords;
} Vertex;

static const Vertex vertices[4] = {
    { { -3.0, -3.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, },
    { {  3.0, -3.0 }, { 2.0, 0.0 }, { 1.0, 0.0 }, },
    { {  3.0,  3.0 }, { 2.0, 2.0 }, { 1.0, 1.0 }, },
    { { -3.0,  3.0 }, { 0.0, 2.0 }, { 0.0, 1.0 }, },
};


#if IS_GlLegacy
const char *vertexShaderSource =
    "attribute vec2 VertCoord;\n"
    "attribute vec2 TexCoord0;\n"
    "attribute vec2 TexCoord1;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * vec4( VertCoord, 0.0, 1.0 );\n"
    "    gl_TexCoord[0] = vec4( TexCoord0, 0.0, 0.0 );\n"
    "    gl_TexCoord[1] = vec4( TexCoord1, 0.0, 0.0 );\n"
    "}\n";

const char *fragmentShaderSource1 =
    "uniform sampler2D tex1;\n"
    "uniform sampler2D tex2;\n"
    "uniform vec4 UniV1;\n"
    "uniform vec4 UniV2;\n"
    "void main()\n"
    "{\n"
    "    vec4 t1 = texture2D(tex1, gl_TexCoord[0].xy);\n"
    "    vec4 t2 = texture2D(tex2, gl_TexCoord[1].xy);\n"
    "    gl_FragColor = mix(t1, t2, t2.w) + UniV1 + UniV2;\n"
    "}\n";

const char *fragmentShaderSource2 =
    "uniform sampler2D tex1;\n"
    "uniform sampler2D tex2;\n"
    "uniform vec4 UniV1;\n"
    "uniform vec4 UniV2;\n"
    "void main()\n"
    "{\n"
    "    vec4 t1 = texture2D(tex1, gl_TexCoord[0].xy);\n"
    "    vec4 t2 = texture2D(tex2, gl_TexCoord[1].xy);\n"
    "    gl_FragColor = t1 + t2 + UniV1 + UniV2;\n"
    "}\n";
#else
const char *vertexShaderSource =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 330\n"
#endif
    "layout (location = 0) in vec2 VertCoord;\n"
    "layout (location = 1) in vec2 TexCoord0;\n"
    "layout (location = 2) in vec2 TexCoord1;\n"
    "uniform mat4 MVP;\n"
    "out vec2 v_TexCoord0;\n"
    "out vec2 v_TexCoord1;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4( VertCoord.x, VertCoord.y, 0.0, 1.0 );\n"
    "    v_TexCoord0 = TexCoord0;\n"
    "    v_TexCoord1 = TexCoord1;\n"
    "}\n";

const char *fragmentShaderSource1 =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 330\n"
#endif
    "in vec2 v_TexCoord0\n;"
    "in vec2 v_TexCoord1\n;"
    "uniform sampler2D tex1;\n"
    "uniform sampler2D tex2;\n"
    "uniform vec4 UniV1;\n"
    "uniform vec4 UniV2;\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    vec4 t1 = texture( tex1, v_TexCoord0 );\n"
    "    vec4 t2 = texture( tex2, v_TexCoord1 );\n"
    "    FragColor = mix(t1, t2, t2.w) + UniV1 + UniV2;\n"
    "}\n";

const char *fragmentShaderSource2 =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 330\n"
#endif
    "in vec2 v_TexCoord0\n;"
    "in vec2 v_TexCoord1\n;"
    "uniform sampler2D tex1;\n"
    "uniform sampler2D tex2;\n"
    "uniform vec4 UniV1;\n"
    "uniform vec4 UniV2;\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    vec4 t1 = texture( tex1, v_TexCoord0 );\n"
    "    vec4 t2 = texture( tex2, v_TexCoord1 );\n"
    "    FragColor = t1 + t2 + UniV1 + UniV2;\n"
    "}\n";
#endif

static void DrawPolygonArray( GLint VertCoord_attr, GLint TexCoord0_attr, GLint TexCoord1_attr)
{
#if IS_GlLegacy
    static int init = 0;
    if( init == 0 ) {
        init = 1;

        glVertexAttribPointer(VertCoord_attr, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].VertCoords);
        glEnableVertexAttribArray(VertCoord_attr);

        glVertexAttribPointer(TexCoord0_attr, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].Tex0Coords);
        glEnableVertexAttribArray(TexCoord0_attr);

        glVertexAttribPointer(TexCoord1_attr, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].Tex1Coords);
        glEnableVertexAttribArray(TexCoord1_attr);
    }
#else
    static int init = 0;
    if( init == 0 ){
        init = 1;

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(VertCoord_attr, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, VertCoords));
        glEnableVertexAttribArray(VertCoord_attr);

        glVertexAttribPointer(TexCoord0_attr, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tex0Coords));
        glEnableVertexAttribArray(TexCoord0_attr);

        glVertexAttribPointer(TexCoord1_attr, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tex1Coords));
        glEnableVertexAttribArray(TexCoord1_attr);
    }
#endif

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

static void Draw(unsigned count)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < count; i++) {
        Yrot = 0.05 * i;

#if IS_GlLegacy
        glPushMatrix(); /* modelview matrix */
        glTranslatef(0.0, 0.0, -EyeDist);
        glRotatef(Zrot, 0, 0, 1);
        glRotatef(Yrot, 0, 1, 0);
        glRotatef(Xrot, 1, 0, 0);
#else
        mat4x4 m;
        mat4x4_dup( m, M );
        mat4x4_translate_in_place( m, 0.0, 0.0, -EyeDist );
        mat4x4_rotate( m, m, 0, 0, 1, Zrot );
        mat4x4_rotate( m, m, 0, 1, 0, Yrot );
        mat4x4_rotate( m, m, 1, 0, 0, Xrot );

        mat4x4 mvp;
        mat4x4_mul( mvp, P, m );
#endif

        glUseProgram(program1);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, texObj[0]);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, texObj[1]);
        glUniform4f( prog1_UniV1_uLoc, Xrot, Yrot, Zrot, 1.000000);
        glUniform4f( prog1_UniV2_uLoc, Xrot, Yrot, Zrot, 1.000000);
#if !IS_GlLegacy
        glUniformMatrix4fv( prog1_MVP_uLoc, 1, GL_FALSE, (const GLfloat*)&mvp );
#endif
        DrawPolygonArray(prog1_VertCoord_aLoc, prog1_TexCoord0_aLoc, prog1_TexCoord1_aLoc);

        glUseProgram(program2);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, texObj[2]);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, texObj[3]);
        glUniform4f( prog2_UniV1_uLoc, Xrot, Yrot, Zrot, 1.000000);
        glUniform4f( prog2_UniV2_uLoc, Xrot, Yrot, Zrot, 1.000000);
#if !IS_GlLegacy
        glUniformMatrix4fv( prog2_MVP_uLoc, 1, GL_FALSE, (const GLfloat*)&mvp );
#endif
        DrawPolygonArray(prog2_VertCoord_aLoc, prog2_TexCoord0_aLoc, prog2_TexCoord1_aLoc);

#if IS_GlLegacy
        glPopMatrix();
#endif
    }

    eglx_SwapBuffers();
}

static void PerfDraw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    printf("GLSL texture/program change rate\n");
    double rate = PerfMeasureRate(Draw, eglx_PollEvents );
    printf("  Immediate mode: %s change/sec\n", PerfHumanFloat(rate));

    glErrorCheck();
    exit(0);
}

static void InitTextures()
{
    GLenum filter = GL_LINEAR;
    int i;

    /* allocate 4 texture objects */
    glGenTextures(4, texObj);

    for (i = 0; i < 4; i++) {
        GLint imgWidth, imgHeight;
        GLenum imgFormat;
        GLubyte *image = NULL;

        image = SGI_LoadRGBImage(TexFiles[i], &imgWidth, &imgHeight, &imgFormat);
        printf("%s: width = %d, height = %d, format = %s\n", TexFiles[i], imgWidth, imgHeight, glFormatName(imgFormat));
        if (!image) {
            printf("Couldn't read %s\n", TexFiles[i]);
            exit(0);
        }

        if( 0 ){
            // dump image
            stbi_flip_vertically_on_write(1);
            int components = (imgFormat == GL_RGB) ? 3 : 4;
            char filename[128];
            sprintf(filename, "/tmp/%d.png", i);
            stbi_write_png(filename, imgWidth, imgHeight, components, image, imgWidth * components);
            printf("dump to %s\n", filename);
        }

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texObj[i]);
#if IS_GlLegacy
        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, imgWidth, imgHeight, imgFormat, GL_UNSIGNED_BYTE, image);
#else
        glTexImage2D( GL_TEXTURE_2D, 0, imgFormat, imgWidth, imgHeight, 0, imgFormat, GL_UNSIGNED_BYTE, image );
        glGenerateMipmap( GL_TEXTURE_2D );
#endif
        free(image);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    }
}

static void InitPrograms()
{
    const float UniV1[4] = {0.8, 0.2, 0.2, 0};
    const float UniV2[4] = {0.6, 0.6, 0.6, 0};

    {
        program1 = CreateProgramFromSource(vertexShaderSource, fragmentShaderSource1);
        glUseProgram( program1 );

        prog1_tex1_uLoc = glGetUniformLocation(program1, "tex1");
        prog1_tex2_uLoc = glGetUniformLocation(program1, "tex2");
        prog1_UniV1_uLoc = glGetUniformLocation(program1, "UniV1");
        prog1_UniV2_uLoc = glGetUniformLocation(program1, "UniV2");
        prog1_MVP_uLoc = glGetUniformLocation(program1, "MVP");
        glUniform1i( prog1_tex1_uLoc, 0);
        glUniform1i( prog1_tex2_uLoc, 1);
        glUniform4fv( prog1_UniV1_uLoc, 1, UniV1);
        glUniform4fv( prog1_UniV2_uLoc, 1, UniV2);

        prog1_VertCoord_aLoc = glGetAttribLocation(program1, "VertCoord");
        prog1_TexCoord0_aLoc = glGetAttribLocation(program1, "TexCoord0");
        prog1_TexCoord1_aLoc = glGetAttribLocation(program1, "TexCoord1");

        printf("prog1_VertCoord_aLoc = %d\n", prog1_VertCoord_aLoc);
        printf("prog1_TexCoord0_aLoc = %d\n", prog1_TexCoord0_aLoc);
        printf("prog1_TexCoord1_aLoc = %d\n", prog1_TexCoord1_aLoc);
        printf("prog1_tex1_uLoc = %d\n", prog1_tex1_uLoc);
        printf("prog1_tex2_uLoc = %d\n", prog1_tex2_uLoc);
        printf("prog1_UniV1_uLoc = %d\n", prog1_UniV1_uLoc);
        printf("prog1_UniV2_uLoc = %d\n", prog1_UniV2_uLoc);
        printf("prog1_MVP_uLoc = %d\n", prog1_MVP_uLoc);
        printf("\n");
    }
    {
        program2 = CreateProgramFromSource(vertexShaderSource, fragmentShaderSource2);
        glUseProgram( program2 );

        prog2_tex1_uLoc = glGetUniformLocation(program2, "tex1");
        prog2_tex2_uLoc = glGetUniformLocation(program2, "tex2");
        prog2_UniV1_uLoc = glGetUniformLocation(program2, "UniV1");
        prog2_UniV2_uLoc = glGetUniformLocation(program2, "UniV2");
        prog2_MVP_uLoc = glGetUniformLocation(program2, "MVP");
        glUniform1i( prog2_tex1_uLoc, 0);
        glUniform1i( prog2_tex2_uLoc, 1);
        glUniform4fv( prog2_UniV1_uLoc, 1, UniV1);
        glUniform4fv( prog2_UniV2_uLoc, 1, UniV2);

        prog2_VertCoord_aLoc = glGetAttribLocation(program2, "VertCoord");
        prog2_TexCoord0_aLoc = glGetAttribLocation(program2, "TexCoord0");
        prog2_TexCoord1_aLoc = glGetAttribLocation(program2, "TexCoord1");

        printf("prog2_VertCoord_aLoc = %d\n", prog2_VertCoord_aLoc);
        printf("prog2_TexCoord0_aLoc = %d\n", prog2_TexCoord0_aLoc);
        printf("prog2_TexCoord1_aLoc = %d\n", prog2_TexCoord1_aLoc);
        printf("prog2_tex1_uLoc = %d\n", prog2_tex1_uLoc);
        printf("prog2_tex2_uLoc = %d\n", prog2_tex2_uLoc);
        printf("prog2_UniV1_uLoc = %d\n", prog2_UniV1_uLoc);
        printf("prog2_UniV2_uLoc = %d\n", prog2_UniV2_uLoc);
        printf("prog2_MVP_uLoc = %d\n", prog2_MVP_uLoc);
        printf("\n");
    }
}

static void PerfInit()
{
    InitTextures();
    InitPrograms();

    glEnable(GL_DEPTH_TEST);
    glClearColor(.6, .6, .9, 0);
#if IS_GlLegacy
    glColor3f(1.0, 1.0, 1.0);
#endif

}

static void Reshape()
{
#if IS_GlLegacy
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 25.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -15.0);
#else
    mat4x4_identity( P );
    mat4x4_frustum( P, -1.0, 1.0, -1.0, 1.0, 5.0, 25.0 );

    mat4x4_identity( M );
    mat4x4_translate( M, 0.0, 0.0, -15.0 );
#endif
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
    Reshape();
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
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------


    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
