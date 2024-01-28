#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "glfwutils.h"
#include "glutils.h"

static void error_callback(int error, const char* description)
{
    printf("GLFW Error: %s\n", description);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow* glfwInit_CreateWindow( api_t api, int width, int height )
{
    // glfw: initialize and configure
    // ------------------------------
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);

    if( api.api == API_GLES ){
        // OpenGLES
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    }else{
        // OpenGL
        if( api.major >= 3 && api.minor >= 2 ) {
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        }
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api.major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api.minor);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(width, height, "", NULL, NULL);
    if (!window){
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    glfwMakeContextCurrent(window);
#if IS_GlEs
    assert( api.api == API_GLES );
    int version = gladLoadGLES2(glfwGetProcAddress);
#else
    assert( api.api == API_GLLegacy || api.api == API_GL );
    int version = gladLoadGL(glfwGetProcAddress);
#endif
    glfwSwapInterval(1);

    // some queries
    // ---------------------------------------
    printf("%s: glad version: %d.%d\n", __func__, GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    const char* glversion = (const char*)glGetString(GL_VERSION);
    printf("%s: GL_VERSON = %s\n", __func__, glversion);
    glfwSetWindowTitle( window, glversion );

#if !IS_GlEs
    GLint profileBit;
    glGetIntegerv( GL_CONTEXT_PROFILE_MASK, &profileBit );
    printf("%s: GL_CONTEXT_PROFILE_MASK = %s\n", __func__, glContextProfileBitName(profileBit));
#endif

    GLint contextFlag;
    glGetIntegerv( GL_CONTEXT_FLAGS, &contextFlag );
    printf("%s: GL_CONTEXT_FLAGS = 0x%x\n", __func__, contextFlag);

    printf("%s: GL_SHADING_LANGUAGE_VERSION = %s\n", __func__, glGetString(GL_SHADING_LANGUAGE_VERSION));

    printf("\n");
    return window;
}
