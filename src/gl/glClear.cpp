#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <ctype.h>
#include "glfwutils.h"
#include "myutils.h"


// settings
const unsigned int WinWidth = 800;
const unsigned int WinHeight = 600;


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

    // render loop
    // -----------
    int i = 0;
    int frame = 0;
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        frame++;
        if( frame % 6 == 0 ){
            if( ++i >= 1000 ){
                i = 0;
            }
        }
        float blue = (float)((i % 10) + 1) * 0.1f;
        float green = (float)(((i/10) % 10) + 1) * 0.1f;
        float red = (float)(((i/100) % 10) + 1) * 0.1f;
        glClearColor(red, green, blue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
