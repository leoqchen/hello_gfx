/*
 * 分别用 Legacy OpenGL、Modern OpenGL 这两种不同写法实现顶点上传、颜色上传
 *
 * glVertexPointer 等价于 glVertexAttribPointer( index = 0 )
 * glColorPointer 无法用 glVertexAttribPointer()等价实现
 */

#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// settings
const unsigned int WinWidth = 800;
const unsigned int WinHeight = 600;


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error: %s\n", description);
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

#if IS_GlEs
const char *vertexShaderSource =
    "#version 320 es\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 Color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   Color = aColor;\n"
    "}\n\0";

const char *fragmentShaderSource =
    "#version 320 es\n"
    "precision mediump float;\n"
    "in vec3 Color;\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4( Color.r, Color.g, Color.b, 1.0f );\n"
    "}\n\0";
#else
const char *vertexShaderSource =
    "#version 400\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 Color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   Color = aColor;\n"
    "}\n\0";

const char *fragmentShaderSource =
    "#version 400\n"
    "in vec3 Color;\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4( Color.r, Color.g, Color.b, 1.0f );\n"
    "}\n\0";
#endif

int main( int argc, const char* argv[] )
{
#if IS_GlEs
    int major = 3;
    int minor = 2;
#elif IS_GlLegacy
    int major = 3;
    int minor = 0;
#else
    int major = 3;
    int minor = 3;
#endif
    if( argc >= 2 && isdigit(argv[1][0]) )
        major = argv[1][0] - '0';
    if( argc >= 3 && isdigit(argv[2][0]) )
        minor = argv[2][0] - '0';
    printf("command line: major = %d, minor = %d\n", major, minor);

    // glfw: initialize and configure
    // ------------------------------
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);

#if IS_GlEs
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
#else
    if( major >= 3 && minor >= 2 ) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    }
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(WinWidth, WinHeight, "LearnOpenGL", NULL, NULL);
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
    int version = gladLoadGLES2(glfwGetProcAddress);
#else
    int version = gladLoadGL(glfwGetProcAddress);
#endif
    glfwSwapInterval(1);

    printf("glad: %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    printf("GL_VERSON = %s\n", glGetString(GL_VERSION));
    glfwSetWindowTitle( window, (const char*)glGetString(GL_VERSION) );

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n",  infoLog);
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // render loop
    // -----------
    glClearColor(0.4, 0.4, 0.4, 0.0);
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        static const GLfloat verts[3][2] = {
                { -1, -1 },
                {  1, -1 },
                {  0,  1 }
        };
        static const GLfloat colors[3][3] = {
                { 1, 0, 0 },
                { 0, 1, 0 },
                { 0, 0, 1 }
        };
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if IS_GlLegacy
        {
            // legacy OpenGL
            glVertexPointer(2, GL_FLOAT, 0, verts);
            glColorPointer(3, GL_FLOAT, 0, colors);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);

            glDrawArrays(GL_TRIANGLES, 0, 3);

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
        }
#else
        {
            // modern OpenGL
            glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, verts );
            glEnableVertexAttribArray( 0 );
            glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, colors );
            glEnableVertexAttribArray( 1 );

            glUseProgram(shaderProgram);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
#endif

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
