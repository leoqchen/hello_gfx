#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdio.h>
#include <string.h>

#include "myutils.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const int WinWidth = 200;
const int WinHeight = 200;

static GLboolean DrawPoint = GL_TRUE;
static GLuint VBO;
static GLuint TexObj = 0;
static GLint BaseLevel, MaxLevel;

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
    //if( !glfwExtensionSupported("GL_ARB_framebuffer_object") ){
    //    printf("Sorry, this test requires GL_ARB_framebuffer_object\n");
    //    exit(1);
    //}

    /* setup VBO w/ vertex data */
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexPointer(2, GL_FLOAT, sizeof(struct vertex), VOFFSET(x)); //TODO:FIXME:GLES, only GL2.1
    glTexCoordPointer(2, GL_FLOAT, sizeof(struct vertex), VOFFSET(s));//TODO:FIXME:GLES, only GL2.1
    glEnableClientState(GL_VERTEX_ARRAY);//TODO:FIXME:GLES, only GL2.1
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);//TODO:FIXME:GLES, only GL2.1

    glGenTextures(1, &TexObj);
    glBindTexture(GL_TEXTURE_2D, TexObj);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glEnable(GL_TEXTURE_2D);
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

static void PerfDraw(void)
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