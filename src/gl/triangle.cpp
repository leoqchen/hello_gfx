#if GLES
  #define GLAD_GLES2_IMPLEMENTATION
  #include <glad/gles2.h>
#else
  #define GLAD_GL_IMPLEMENTATION
  #include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include "linmath.h"

typedef struct Vertex
{
    vec2 pos;
    vec3 col;
} Vertex;

static const Vertex vertices[3] =
{
    { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
    { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
    { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }
};

#if GLES
static const char* vertex_shader_text =
    "#version 320 es\n"
    "precision mediump float;\n"
    "uniform mat4 MVP;\n"
    "layout (location = 0) in vec3 vCol;\n"
    "layout (location = 1) in vec2 vPos;\n"
    "layout (location = 0) out vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
    "    color = vCol;\n"
    "}\n";

static const char* fragment_shader_text =
    "#version 320 es\n"
    "precision mediump float;\n"
    "layout (location = 0) in vec3 color;\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(color, 1.0);\n"
    "}\n";
#else
static const char* vertex_shader_text =
    "#version 420\n"
    "uniform mat4 MVP;\n"
    "layout (location = 0) in vec3 vCol;\n"
    "layout (location = 1) in vec2 vPos;\n"
    "layout (location = 0) out vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
    "    color = vCol;\n"
    "}\n";

static const char* fragment_shader_text =
    "#version 420\n"
    "layout (location = 0) in vec3 color;\n"
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(color, 1.0);\n"
    "}\n";
#endif

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

int main( int argc, const char* argv[] )
{
#if GLES
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

#if GLES
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
    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window){
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    glfwMakeContextCurrent(window);
#if GLES
    int version = gladLoadGLES2(glfwGetProcAddress);
#else
    int version = gladLoadGL(glfwGetProcAddress);
#endif
    glfwSwapInterval(1);

    printf("glad: %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    printf("GL_VERSON = %s\n", glGetString(GL_VERSION));
    glfwSetWindowTitle( window, (const char*)glGetString(GL_VERSION) );

//    float vertex[4];
//    glVertexPointer( 2, GL_FLOAT, 4, vertex);//XXX
//    //XXX: 试验绑定FBO
//    GLuint RBO, FBO;
//    glGenFramebuffers(1, &FBO);
//    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
//    /* setup rbo */
//    glGenRenderbuffers(1, &RBO);
//    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, 4096, 4096);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, RBO);
//    GLenum stat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//    if (stat != GL_FRAMEBUFFER_COMPLETE) {
//        printf("fboswitch: Error: incomplete FBO!: 0x%X\n", stat);
//    }else{
//        printf("framebuffer complete\n");
//    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    // fragment shader
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
    // check for shader compile errors
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    // link shaders
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    // check for linking errors
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // set up uniform data
    // ------------------------------------------------------------------
    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glEnableVertexAttribArray(vpos_location);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, col));

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;

        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

