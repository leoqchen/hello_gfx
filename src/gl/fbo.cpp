/*
 * OpenGL、OpenGLES在实现color attachment绑定有差异
 * 对于OpenGL，用render buffer、texture2D 绑定到color attachment均可
 * 对于OpenGLES，只能用texture2D 绑定到color attachment
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
    "void main()\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "}\n\0";

const char *fragmentShaderSource =
    "#version 320 es\n"
    "precision mediump float;\n"
    "layout(location = 0) out vec4 fragData0;\n"
    "layout(location = 1) out vec4 fragData1;\n"
    "layout(location = 2) out vec4 fragData2;\n"
    "layout(location = 3) out vec4 fragData3;\n"
    "void main()\n"
    "{\n"
    "  // first buffer will contain red color\n"
    "  fragData0 = vec4 ( 1, 0, 0, 1 );\n"
    "\n"
    "  // second buffer will contain green color\n"
    "  fragData1 = vec4 ( 0, 1, 0, 1 );\n"
    "\n"
    "  // third buffer will contain blue color\n"
    "  fragData2 = vec4 ( 0, 0, 1, 1 );\n"
    "\n"
    "  // fourth buffer will contain gray color\n"
    "  fragData3 = vec4 ( 0.5, 0.5, 0.5, 1 );\n"
    "}\n\0";
#else
const char *vertexShaderSource =
    "#version 400\n"
    "layout (location = 0) in vec4 a_position;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "}\n\0";

const char *fragmentShaderSource =
    "#version 400\n"
    "layout(location = 0) out vec4 fragData0;\n"
    "layout(location = 1) out vec4 fragData1;\n"
    "layout(location = 2) out vec4 fragData2;\n"
    "layout(location = 3) out vec4 fragData3;\n"
    "void main()\n"
    "{\n"
    "  // first buffer will contain red color\n"
    "  fragData0 = vec4 ( 1, 0, 0, 1 );\n"
    "\n"
    "  // second buffer will contain green color\n"
    "  fragData1 = vec4 ( 0, 1, 0, 1 );\n"
    "\n"
    "  // third buffer will contain blue color\n"
    "  fragData2 = vec4 ( 0, 0, 1, 1 );\n"
    "\n"
    "  // fourth buffer will contain gray color\n"
    "  fragData3 = vec4 ( 0.5, 0.5, 0.5, 1 );\n"
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

    // Setup fbo
    GLint defaultFramebuffer = 0;
    glGetIntegerv ( GL_FRAMEBUFFER_BINDING, &defaultFramebuffer );

    GLuint fbo;
    glGenFramebuffers ( 1, &fbo );
    glBindFramebuffer ( GL_FRAMEBUFFER, fbo );

    // Setup four output buffers and attach to fbo
    static const GLenum attachments[4] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3
    };
    GLsizei textureWidth = 400;
    GLsizei textureHeight = 400;

#if IS_ColorAttachRenderbuffer
    GLuint rb[4];
    glGenRenderbuffers( 4, &rb[0] );
    for( int i=0; i < 4; i++ )
    {
        glBindRenderbuffer( GL_RENDERBUFFER, rb[i] );
        glFramebufferRenderbuffer( GL_FRAMEBUFFER, attachments[i], GL_RENDERBUFFER, rb[i] );
        glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, textureWidth, textureHeight );
    }
#endif
#if IS_ColorAttachTexture
    GLuint colorTexId[4];
    glGenTextures ( 4, &colorTexId[0] );
    for( int i = 0; i < 4; ++i)
    {
        glBindTexture ( GL_TEXTURE_2D, colorTexId[i] );
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA,textureWidth, textureHeight,
                       0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );

        // Set the filtering mode
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        glFramebufferTexture2D ( GL_DRAW_FRAMEBUFFER, attachments[i], GL_TEXTURE_2D, colorTexId[i], 0 );
    }
#endif

    GLenum stat = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( stat != GL_FRAMEBUFFER_COMPLETE ){
        printf("Error: incomplete FBO!: 0x%X\n", stat);
        exit(1);
    }

    // Restore the original framebuffer
    glBindFramebuffer ( GL_FRAMEBUFFER, defaultFramebuffer );

    // render loop
    // -----------
    glClearColor ( 1.0f, 1.0f, 1.0f, 0.0f );
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glGetIntegerv ( GL_FRAMEBUFFER_BINDING, &defaultFramebuffer );

        // FIRST: use MRTs to output four colors to four buffers
        glBindFramebuffer ( GL_FRAMEBUFFER, fbo );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glDrawBuffers ( 4, attachments );

        // draw
        static const GLfloat vVertices[] = {
            -1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f,  1.0f, 0.0f,
        };
        static const GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

        // Use the program object
        glUseProgram ( shaderProgram );

        // Load the vertex position
        glVertexAttribPointer( 0, 3, GL_FLOAT,GL_FALSE, 3 * sizeof (GLfloat), vVertices );
        glEnableVertexAttribArray( 0 );

        // Draw a quad
        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

        // Restore the default framebuffer
        glBindFramebuffer( GL_DRAW_FRAMEBUFFER, defaultFramebuffer );

        // SECOND: copy the four output buffers into four window quadrants
        // with framebuffer blits
        // set the fbo for reading
        glBindFramebuffer( GL_READ_FRAMEBUFFER, fbo );

        // Copy the output red buffer to lower left quadrant
        glReadBuffer ( GL_COLOR_ATTACHMENT0 );
        glBlitFramebuffer ( 0, 0, textureWidth, textureHeight,
                            0, 0, WinWidth/2, WinHeight/2,
                            GL_COLOR_BUFFER_BIT, GL_LINEAR );

        // Copy the output green buffer to lower right quadrant
        glReadBuffer ( GL_COLOR_ATTACHMENT1 );
        glBlitFramebuffer ( 0, 0, textureWidth, textureHeight,
                            WinWidth/2, 0, WinWidth, WinHeight/2,
                            GL_COLOR_BUFFER_BIT, GL_LINEAR );

        // Copy the output blue buffer to upper left quadrant
        glReadBuffer ( GL_COLOR_ATTACHMENT2 );
        glBlitFramebuffer ( 0, 0, textureWidth, textureHeight,
                            0, WinHeight/2, WinWidth/2, WinHeight,
                            GL_COLOR_BUFFER_BIT, GL_LINEAR );

        // Copy the output gray buffer to upper right quadrant
        glReadBuffer ( GL_COLOR_ATTACHMENT3 );
        glBlitFramebuffer ( 0, 0, textureWidth, textureHeight,
                            WinWidth/2, WinHeight/2, WinWidth, WinHeight,
                            GL_COLOR_BUFFER_BIT, GL_LINEAR );

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
#if IS_ColorAttachRenderbuffer
    glDeleteRenderbuffers( 4, &rb[0] );
#endif
#if IS_ColorAttachTexture
    glDeleteTextures( 4, &colorTexId[0] );
#endif
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
