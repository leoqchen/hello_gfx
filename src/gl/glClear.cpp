#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdio.h>
#include <string.h>
#include "myutils.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int WinWidth = 800;
const unsigned int WinHeight = 600;

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

    // render loop
    // -----------
    int i = 0;
    int frame = 0;
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        float blue = (float)((i % 10) + 1) * 0.1f;
        float green = (float)(((i/10) % 10) + 1) * 0.1f;
        float red = (float)(((i/100) % 10) + 1) * 0.1f;
        glClearColor(red, green, blue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

        frame++;
        if( frame % 6 == 0 ){
            if( ++i >= 1000 ){
                i = 0;
            }
        }
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