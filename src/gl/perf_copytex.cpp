#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdio.h>
#include <string.h>

#include "myutils.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
int WinWidth = 100, WinHeight = 100;

static GLuint VBO, FBO, RBO, Tex;

const GLsizei MinSize = 16, MaxSize = 4096;
static GLsizei TexSize;

static const GLboolean DrawPoint = GL_TRUE;
static const GLboolean TexSubImage4 = GL_FALSE;

struct vertex
{
    GLfloat x, y, s, t;
};

static const struct vertex vertices[1] = {
        { 0.0, 0.0, 0.5, 0.5 },
};

#define VOFFSET(F) ((void *) offsetof(struct vertex, F))

static void PerfInit(void)
{
    const GLenum filter = GL_LINEAR;
    GLenum stat;

    /* setup VBO */
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexPointer(2, GL_FLOAT, sizeof(struct vertex), VOFFSET(x));
    glTexCoordPointer(2, GL_FLOAT, sizeof(struct vertex), VOFFSET(s));
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    /* setup texture */
    glGenTextures(1, &Tex);
    glBindTexture(GL_TEXTURE_2D, Tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glEnable(GL_TEXTURE_2D);

    /* setup rbo */
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, MaxSize, MaxSize);

    /* setup fbo */
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, RBO);

    stat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (stat != GL_FRAMEBUFFER_COMPLETE) {
        printf("fboswitch: Error: incomplete FBO!\n");
        exit(1);
    }

    /* clear the FBO */
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, MaxSize, MaxSize);
    glClear(GL_COLOR_BUFFER_BIT);
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

void PerfDraw(void)
{
    double rate, mbPerSec;
    GLint sub, maxTexSize;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);

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

            printf("  glCopyTex%sImage(%d x %d): %.1f copies/sec, %.1f Mpixels/sec\n",
                        (sub ? "Sub" : ""), TexSize, TexSize, rate, mbPerSec);
        }
    }

    exit(0);
}


int main( int argc, const char* argv[] )
{
    if( argc != 2 ){
        Usage:
        printf("Usage: %s [glXX|glesXX]\n", argv[0]);
        return 1;
    }

    api_t api = parse_api( argv[1] );
    if( api.api == -1 ){
        printf("invalid argument: %s\n", argv[1]);
        goto Usage;
    }

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    if( api.api == 1 ){
        glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
    }else {
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.minor);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(WinWidth, WinHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    printf("GL_VERSION = %s\n", glGetString(GL_VERSION));
    int major_version = 0;
    int minor_version = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    printf("GL_MAJOR_VERSION = %d\n", major_version);
    printf("GL_MINOR_VERSION = %d\n", minor_version);


    // init
    // -----------
    PerfInit();


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        PerfDraw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}