#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <ctype.h>
#include "glfwUtils.h"
#include "glUtils.h"
#include "myUtils.h"


// settings
static const int WinWidth = 800;
static const int WinHeight = 600;


int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    // glfw: initialize and configure
    // ------------------------------
    GLFWwindow* window = glfw_CreateWindow(api, WinWidth, WinHeight);

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
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
