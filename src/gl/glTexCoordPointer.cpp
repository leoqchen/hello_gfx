/*
 * 分别用 Legacy OpenGL、Modern OpenGL 这两种不同写法实现纹理坐标上传
 *
 */

#if IS_GlEs
#define GLAD_GLES2_IMPLEMENTATION
  #include <glad/gles2.h>
#else
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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
    "layout (location = 0) in vec4 a_position;\n"
    "layout (location = 1) in vec2 a_texCoord;\n"
    "out vec2 v_texCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   v_texCoord = a_texCoord;\n"
    "}\n\0";

const char *fragmentShaderSource =
    "#version 320 es\n"
    "precision mediump float;\n"
    "in vec2 v_texCoord;\n"
    "layout (location = 0) out vec4 outColor;\n"
    "uniform sampler2D s_texture;\n"
    "void main()\n"
    "{\n"
    "   outColor = texture( s_texture, v_texCoord );\n"
    "}\n\0";
#else
const char *vertexShaderSource =
    "#version 420\n"
    "layout (location = 0) in vec4 a_position;\n"
    "layout (location = 1) in vec2 a_texCoord;\n"
    "out vec2 v_texCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   v_texCoord = a_texCoord;\n"
    "}\n\0";

const char *fragmentShaderSource =
    "#version 420\n"
    "in vec2 v_texCoord;\n"
    "layout (location = 0) out vec4 outColor;\n"
    "uniform sampler2D s_texture;\n"
    "void main()\n"
    "{\n"
    "   outColor = texture( s_texture, v_texCoord );\n"
    "}\n\0";
#endif

int main( int argc, const char* argv[] )
{
#if IS_GlEs
    int major = 3;
    int minor = 2;
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
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
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

    // Create a 2x2 texture image
    GLuint textureId;
    static const GLubyte pixels[4 * 3] = {
        255,   0,   0, // Red
        0, 255,   0, // Green
        0,   0, 255, // Blue
        255, 255,   0  // Yellow
    };
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glGenTextures( 1, &textureId );
    glBindTexture( GL_TEXTURE_2D, textureId );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    GLint samplerLoc = glGetUniformLocation( shaderProgram, "s_texture" );

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        static const GLfloat vVertices[] = {
            -0.5f,  0.5f, 0.0f,  // Position 0
            0.0f,  0.0f,        // TexCoord 0
            -0.5f, -0.5f, 0.0f,  // Position 1
            0.0f,  1.0f,        // TexCoord 1
            0.5f, -0.5f, 0.0f,  // Position 2
            1.0f,  1.0f,        // TexCoord 2
            0.5f,  0.5f, 0.0f,  // Position 3
            1.0f,  0.0f         // TexCoord 3
        };
        static const GLushort indices[] = {
            0, 1, 2, 0, 2, 3
        };
        glClear(GL_COLOR_BUFFER_BIT );

#if IS_GlLegacy
        {
            // legacy OpenGL
            glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), vVertices );
            glTexCoordPointer( 2, GL_FLOAT, 5 * sizeof(GLfloat), &vVertices[3] );
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glBindTexture( GL_TEXTURE_2D, textureId );
            glEnable(GL_TEXTURE_2D);

            // set the sampler texture unit to 0
            glUniform1i( samplerLoc, 0 );

            glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
#else
        {
            // modern OpenGL
            glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vVertices );
            glEnableVertexAttribArray( 0 );
            glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );
            glEnableVertexAttribArray( 1 );

            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, textureId );

            // set the sampler texture unit to 0
            glUniform1i( samplerLoc, 0 );

            glUseProgram(shaderProgram);

            glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
        }
#endif

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteTextures( 1, &textureId );
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
