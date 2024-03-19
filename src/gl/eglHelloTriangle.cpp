#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include "glUtils.h"
#include "myUtils.h"
#include "x11Utils.h"
#include "eglUtils.h"


// settings
static const int WinWidth = 800;
static const int WinHeight = 600;

const char *vertexShaderSource =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 400\n"
#endif
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n\0";

const char *fragmentShaderSource =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 400\n"
#endif
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";


int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    // glfw: initialize and configure
    // ------------------------------
//    GLFWwindow* window = glfw_CreateWindow( api, WinWidth, WinHeight );
    void* nativeDisplayPtr;
    void* nativeWindowPtr;
    xWindowCreate( &nativeDisplayPtr, &nativeWindowPtr, "", WinWidth, WinHeight );
    egl_CreateContext( nativeDisplayPtr, nativeWindowPtr ); //TODO: support GL
    printf("%s: GL_VENDER = %s\n", __func__, glGetString(GL_VENDOR));
    printf("%s: GL_RENDERER = %s\n", __func__, glGetString(GL_RENDERER));
    printf("%s: GL_VERSION = %s\n", __func__, glGetString(GL_VERSION));
    printf("%s: GL_SHADING_LANGUAGE_VERSION = %s\n", __func__, glGetString(GL_SHADING_LANGUAGE_VERSION));

    // some query
    int major_version = 0;
    int minor_version = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    printf("GL_MAJOR_VERSION = %d\n", major_version);
    printf("GL_MINOR_VERSION = %d\n", minor_version);

#if IS_GlLegacy
    printf("GL_EXTENSIONS = %s\n", glGetString(GL_EXTENSIONS)); // GL_EXTENSIONS is removed from core OpenGL 3.1 aand above
#endif

    int num_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    printf("GL_NUM_EXTENSIONS = %d\n", num_extensions);
    for( int i=0; i < num_extensions; i++ ){
        printf("%s ", glGetStringi(GL_EXTENSIONS, i));
    }
    printf("\n");

    // build and compile our shader program
    // ------------------------------------
    const GLuint program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    };

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLint apos_location = glGetAttribLocation(program, "aPos");
    printf("Uniform location: aPos=%d\n", apos_location);
    glVertexAttribPointer(apos_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray( apos_location );

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
//    while(!glfwWindowShouldClose(window))
    while( !xWindowShouldClose() )
    {
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(program);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // glBindVertexArray(0); // no need to unbind it every time

        // dump to disk
        // ------------
        static int i = 0;
        if( i == 0 ){
            i = 1;

            glFinish();
            GLubyte* pixels = (GLubyte*)malloc( WinWidth * WinHeight * 4 );
            glReadPixels(0, 0, WinWidth, WinHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

            // Write image Y-flipped because OpenGL
            stbi_flip_vertically_on_write( 1 );
            stbi_write_png("/tmp/1.png", WinWidth, WinHeight, 4, pixels, WinWidth*4);
            printf("dump to /tmp/1.png\n");
            stbi_write_jpg("/tmp/1.jpg", WinWidth, WinHeight, 4, pixels, 95);
            printf("dump to /tmp/1.jpg\n");

            free( pixels );
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
//        glfwSwapBuffers(window);
//        glfwPollEvents();
        egl_SwapBuffers();

        //static int frame = 0;
        //printf("frame = %d\n", frame++);
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
//    glfwDestroyWindow(window);
//    glfwTerminate();
    egl_Terminate();
    xWindowDestroy();
    return 0;
}
